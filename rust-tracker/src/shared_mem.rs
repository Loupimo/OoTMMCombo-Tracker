//! Lecture de la mémoire partagée écrite par la DLL PJ64OoTMMTracker.
//!
//! C'est LE point qui justifie Rust pour ce projet : le contrat DLL <-> tracker
//! est un POD C pur (aucun pointeur). On remirroir la struct C en `#[repr(C)]`
//! et on lit le buffer par un cast direct du pointeur mappé — zéro marshalling,
//! exactement comme le fait le C++ dans Sources/UI/MemoryReader.cpp.
//!
//! Contrat d'origine (Headers/UI/MemoryReader.h) :
//! ```c
//! #define BUFFER_SIZE 1024
//! typedef struct Event    { uint32_t PC; uint32_t Mem; uint32_t Query[6]; } Event;
//! enum class TrackerCommand : uint8_t { None = 0, Shutdown = 1 };
//! typedef struct SharedData {
//!     uint32_t GameVersion[2];
//!     LONG     MaxSize;              // = i32
//!     volatile LONG CurrIndex;       // = i32
//!     Event    Buffer[BUFFER_SIZE];
//!     volatile int32_t HostROMVersion;
//!     TrackerCommand   Command;      // = u8 (tracker -> DLL : demande d'arrêt)
//! } SharedData;
//! ```
//! Comme tous les champs font 4 octets et qu'il n'y a aucun pointeur, le layout
//! est identique en 32 et 64 bits : un tracker Rust 64 bits peut mapper la
//! mémoire créée par la DLL 32 bits sans souci de bitness.

use std::ffi::c_void;
use std::ptr;

/// Doit rester en phase avec BUFFER_SIZE côté C (Headers/UI/MemoryReader.h).
pub const BUFFER_SIZE: usize = 1024;

/// Nom du mapping Win32, identique à OpenFileMappingA(..., "PJ64_SHARED_MEM").
const SHARED_MEM_NAME: &[u8] = b"PJ64_SHARED_MEM\0";

const FILE_MAP_ALL_ACCESS: u32 = 0x000F_001F;

// --- Déclarations kernel32 minimales (pas de crate windows-sys) --------------
#[link(name = "kernel32")]
extern "system" {
    fn OpenFileMappingA(
        dw_desired_access: u32,
        b_inherit_handle: i32,
        lp_name: *const u8,
    ) -> isize;

    fn MapViewOfFile(
        h_file_mapping_object: isize,
        dw_desired_access: u32,
        dw_file_offset_high: u32,
        dw_file_offset_low: u32,
        dw_number_of_bytes_to_map: usize,
    ) -> *mut c_void;

    fn UnmapViewOfFile(lp_base_address: *const c_void) -> i32;
    fn CloseHandle(h_object: isize) -> i32;
}

/// Miroir exact de `Event` (Headers/UI/MemoryReader.h:24).
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct Event {
    /// Valeur du program counter au moment du hook.
    pub pc: u32,
    /// Adresse mémoire lue (ou champ ré-utilisé par le tracker d'entrances).
    pub mem: u32,
    /// Données collectées : combo key, gi, scene, entrance / coords respawn.
    pub query: [u32; 6],
}

/// Valeur de `TrackerCommand::Shutdown` (Headers/UI/MemoryReader.h:35) : demande
/// à la DLL de s'arrêter et de restaurer ce qu'elle a patché avant de se décharger.
pub const TRACKER_COMMAND_SHUTDOWN: u8 = 1;

/// Miroir exact de `SharedData` (Headers/UI/MemoryReader.h:43).
///
/// Le champ `command` a été ajouté avec la nouvelle méthode d'injection : le
/// tracker l'écrit à `Shutdown` pour que la DLL, chargée comme plugin PJ64, se
/// décharge proprement (au lieu d'être arrachée par un injecteur externe). Le
/// `#[repr(C)]` reproduit le padding C : un `u8` suivi de 3 octets de bourrage,
/// donc `size_of::<SharedData>()` reste égal au `sizeof(SharedData)` du C++.
#[repr(C)]
pub struct SharedData {
    pub game_version: [u32; 2],
    pub max_size: i32,
    pub curr_index: i32,
    pub buffer: [Event; BUFFER_SIZE],
    pub host_rom_version: i32,
    pub command: u8,
}

/// Poignée sur la vue mappée + curseur de lecture du ring buffer.
pub struct SharedMemory {
    map_handle: isize,
    view: *mut SharedData,
    /// Dernier index consommé, pour ne relire que les nouveaux Event.
    read_index: i32,
}

impl SharedMemory {
    /// Ouvre le mapping "PJ64_SHARED_MEM" et mappe la vue.
    ///
    /// @return Some(SharedMemory) si PJ64 tourne avec la DLL injectée, sinon None.
    pub fn open() -> Option<Self> {
        unsafe {
            let map_handle =
                OpenFileMappingA(FILE_MAP_ALL_ACCESS, 0, SHARED_MEM_NAME.as_ptr());
            if map_handle == 0 {
                return None;
            }

            let view = MapViewOfFile(
                map_handle,
                FILE_MAP_ALL_ACCESS,
                0,
                0,
                std::mem::size_of::<SharedData>(),
            ) as *mut SharedData;

            if view.is_null() {
                CloseHandle(map_handle);
                return None;
            }

            // On démarre le curseur sur l'index courant : on ne rejoue pas
            // l'historique déjà présent, on suit seulement le flux à venir.
            let read_index = ptr::read_volatile(ptr::addr_of!((*view).curr_index));

            Some(SharedMemory {
                map_handle,
                view,
                read_index,
            })
        }
    }

    /// The two game-version words of the currently tracked ROM
    /// (used to detect the stable vs dev build).
    pub fn game_version(&self) -> [u32; 2] {
        unsafe { (*self.view).game_version }
    }

    /// Ask the DLL to shut down and undo its patches (mirror of
    /// `MemoryReader::StartMemoryReader` step 10: `DLLData->Command = Shutdown`).
    /// A volatile write so the DLL's poll loop observes it promptly.
    pub fn request_shutdown(&self) {
        unsafe {
            ptr::write_volatile(ptr::addr_of_mut!((*self.view).command), TRACKER_COMMAND_SHUTDOWN);
        }
    }

    /// Consomme tous les Event apparus depuis le dernier appel.
    ///
    /// Réplique la logique du ring buffer de MemoryReader.cpp : on lit de
    /// `read_index` jusqu'à `CurrIndex`, avec gestion du wrap sur `MaxSize`.
    ///
    /// @return Le vecteur des nouveaux Event, dans l'ordre d'arrivée.
    pub fn poll(&mut self) -> Vec<Event> {
        let mut out = Vec::new();
        unsafe {
            let max = (*self.view).max_size;
            if max <= 0 {
                return out;
            }
            let curr = ptr::read_volatile(ptr::addr_of!((*self.view).curr_index));

            let mut i = self.read_index;
            // Avance jusqu'à rattraper curr, en bouclant sur le ring buffer.
            while i != curr {
                if i >= 0 && (i as usize) < BUFFER_SIZE {
                    out.push(ptr::read(ptr::addr_of!((*self.view).buffer[i as usize])));
                }
                i += 1;
                if i >= max {
                    i = 0;
                }
            }
            self.read_index = curr;
        }
        out
    }
}

impl Drop for SharedMemory {
    fn drop(&mut self) {
        unsafe {
            if !self.view.is_null() {
                UnmapViewOfFile(self.view as *const c_void);
            }
            if self.map_handle != 0 {
                CloseHandle(self.map_handle);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Le mapping DLL <-> tracker repose sur un layout binaire identique. Ce
    /// test verrouille la taille attendue (calculée depuis le C++), notamment le
    /// padding qui suit le `command: u8` : GameVersion(8) + MaxSize(4) +
    /// CurrIndex(4) + Buffer(1024*32) + HostROMVersion(4) + command(1) -> arrondi
    /// à un multiple de 4 = 32792.
    #[test]
    fn shared_data_layout_matches_cpp() {
        assert_eq!(std::mem::size_of::<Event>(), 32);
        assert_eq!(std::mem::size_of::<SharedData>(), 32792);
    }
}

