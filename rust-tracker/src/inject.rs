//! Charge la DLL de tracking dans Project64 SANS injecteur externe.
//!
//! Transposition de la nouvelle méthode de `Sources/UI/MemoryReader.cpp`
//! (`CopyTrackerDLL` / `OpenProject64Settings` / `StartMemoryReader`). L'ancien
//! `PJ64Injector.exe` (CreateRemoteThread + LoadLibrary) était signalé par les
//! antivirus ; il est remplacé par un chargement « propre » :
//!
//!   1. copier `PJ64OoTMMTracker.dll` dans `<dossier PJ64>/Plugin/` ;
//!   2. envoyer Ctrl+T à la fenêtre principale de PJ64 (SendInput) pour ouvrir
//!      les réglages, ce qui force PJ64 à énumérer et charger ses plugins ;
//!   3. attendre l'apparition de la fenêtre de réglages, puis (côté poller) que
//!      la DLL crée la mémoire partagée, puis fermer cette fenêtre ;
//!   4. à l'arrêt : demander à la DLL de se décharger (SharedData.command =
//!      Shutdown), attendre que le module disparaisse, puis supprimer la DLL du
//!      dossier Plugin.
//!
//! Comme la DLL est chargée par PJ64 lui-même (et non écrite en mémoire par un
//! tiers), il n'y a plus de séquence d'injection suspecte à détecter.

use std::path::PathBuf;
use std::thread;
use std::time::{Duration, Instant};

use crate::i18n::LogStrings;
use crate::scene::REPO_ROOT;

// --- Constantes Win32 --------------------------------------------------------
const TH32CS_SNAPPROCESS: u32 = 0x0000_0002;
const TH32CS_SNAPMODULE: u32 = 0x0000_0008;
const TH32CS_SNAPMODULE32: u32 = 0x0000_0010;
const INVALID_HANDLE_VALUE: isize = -1;
const MAX_PATH: usize = 260;
const MAX_MODULE_NAME32: usize = 255;

const PROCESS_QUERY_LIMITED_INFORMATION: u32 = 0x1000;

const GW_OWNER: u32 = 4;
const SW_RESTORE: i32 = 9;
const WM_CLOSE: u32 = 0x0010;

const INPUT_KEYBOARD: u32 = 1;
const KEYEVENTF_KEYUP: u32 = 0x0002;
const VK_CONTROL: u16 = 0x11;
const VK_T: u16 = 0x54;

/// Nom du fichier DLL de tracking, identique côté C++.
const DLL_NAME: &str = "PJ64OoTMMTracker.dll";

// --- Structures Win32 --------------------------------------------------------
#[repr(C)]
struct ProcessEntry32W {
    dw_size: u32,
    cnt_usage: u32,
    th32_process_id: u32,
    th32_default_heap_id: usize,
    th32_module_id: u32,
    cnt_threads: u32,
    th32_parent_process_id: u32,
    pc_pri_class_base: i32,
    dw_flags: u32,
    sz_exe_file: [u16; MAX_PATH],
}

/// Miroir 64 bits de `MODULEENTRY32W` (les champs pointeur font 8 octets).
#[repr(C)]
struct ModuleEntry32W {
    dw_size: u32,
    th32_module_id: u32,
    th32_process_id: u32,
    glbl_cnt_usage: u32,
    proc_cnt_usage: u32,
    mod_base_addr: *mut u8,
    mod_base_size: u32,
    h_module: isize,
    sz_module: [u16; MAX_MODULE_NAME32 + 1],
    sz_exe_path: [u16; MAX_PATH],
}

/// `KEYBDINPUT` (Win32). `dw_extra_info` est un `ULONG_PTR` (8 octets en x64).
#[repr(C)]
#[derive(Clone, Copy)]
struct KeybdInput {
    w_vk: u16,
    w_scan: u16,
    dw_flags: u32,
    time: u32,
    dw_extra_info: usize,
}

/// `INPUT` (Win32) réduit au cas clavier. L'union d'origine (MOUSEINPUT le plus
/// gros) fait 32 octets en x64 ; on complète donc après `ki` pour que
/// `size_of::<Input>() == sizeof(INPUT) == 40`, valeur que `SendInput` exige
/// comme `cbSize`.
#[repr(C)]
#[derive(Clone, Copy)]
struct Input {
    type_: u32,
    _pad: u32,
    ki: KeybdInput,
    _tail: [u8; 8],
}

// Garde-fou : SendInput rejette tout `cbSize` différent de sizeof(INPUT).
const _: () = assert!(std::mem::size_of::<Input>() == 40);

/// Contexte passé à `EnumWindows` pour collecter les fenêtres d'un PID.
struct WinSearch {
    pid: u32,
    windows: Vec<isize>,
}

// --- Déclarations FFI --------------------------------------------------------
#[link(name = "kernel32")]
extern "system" {
    fn CreateToolhelp32Snapshot(flags: u32, process_id: u32) -> isize;
    fn Process32FirstW(snapshot: isize, entry: *mut ProcessEntry32W) -> i32;
    fn Process32NextW(snapshot: isize, entry: *mut ProcessEntry32W) -> i32;
    fn Module32FirstW(snapshot: isize, entry: *mut ModuleEntry32W) -> i32;
    fn Module32NextW(snapshot: isize, entry: *mut ModuleEntry32W) -> i32;
    fn CloseHandle(handle: isize) -> i32;
    fn OpenProcess(desired_access: u32, inherit: i32, pid: u32) -> isize;
    fn QueryFullProcessImageNameA(
        process: isize,
        flags: u32,
        buffer: *mut u8,
        size: *mut u32,
    ) -> i32;
}

#[link(name = "user32")]
extern "system" {
    fn EnumWindows(callback: extern "system" fn(isize, isize) -> i32, lparam: isize) -> i32;
    fn GetWindowThreadProcessId(hwnd: isize, pid: *mut u32) -> u32;
    fn IsWindowVisible(hwnd: isize) -> i32;
    fn GetWindow(hwnd: isize, cmd: u32) -> isize;
    fn IsIconic(hwnd: isize) -> i32;
    fn ShowWindow(hwnd: isize, cmd: i32) -> i32;
    fn SetForegroundWindow(hwnd: isize) -> i32;
    fn SendInput(n: u32, inputs: *const Input, cb_size: i32) -> u32;
    fn PostMessageA(hwnd: isize, msg: u32, wparam: usize, lparam: isize) -> i32;
    fn GetClassNameA(hwnd: isize, class_name: *mut u8, max_count: i32) -> i32;
}

// --- Utilitaires -------------------------------------------------------------
fn wide_to_string(buf: &[u16]) -> String {
    let end = buf.iter().position(|&c| c == 0).unwrap_or(buf.len());
    String::from_utf16_lossy(&buf[..end])
}

/// Cherche le PID de Project64 par nom d'exécutable (insensible à la casse).
pub fn find_pj64_pid() -> Option<u32> {
    unsafe {
        let snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if snap == INVALID_HANDLE_VALUE || snap == 0 {
            return None;
        }
        let mut entry: ProcessEntry32W = std::mem::zeroed();
        entry.dw_size = std::mem::size_of::<ProcessEntry32W>() as u32;

        let mut found = None;
        if Process32FirstW(snap, &mut entry) != 0 {
            loop {
                if wide_to_string(&entry.sz_exe_file)
                    .to_lowercase()
                    .contains("project64")
                {
                    found = Some(entry.th32_process_id);
                    break;
                }
                if Process32NextW(snap, &mut entry) == 0 {
                    break;
                }
            }
        }
        CloseHandle(snap);
        found
    }
}

/// Chemin complet de l'exécutable du processus (mirror `GetProcessPath`).
fn process_path(pid: u32) -> Option<PathBuf> {
    unsafe {
        let handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, pid);
        if handle == 0 {
            return None;
        }
        let mut buf = [0u8; MAX_PATH];
        let mut size = MAX_PATH as u32;
        let ok = QueryFullProcessImageNameA(handle, 0, buf.as_mut_ptr(), &mut size);
        CloseHandle(handle);
        if ok == 0 {
            return None;
        }
        Some(PathBuf::from(String::from_utf8_lossy(&buf[..size as usize]).into_owned()))
    }
}

/// Localise la DLL de tracking à copier : d'abord à côté de l'exécutable du
/// tracker (scénario déployé), sinon dans les sorties de build C++.
fn source_dll() -> Option<PathBuf> {
    if let Ok(exe) = std::env::current_exe() {
        if let Some(dir) = exe.parent() {
            let p = dir.join(DLL_NAME);
            if p.exists() {
                return Some(p);
            }
        }
    }
    ["Release", "Debug"].into_iter().find_map(|cfg| {
        let p = PathBuf::from(format!("{REPO_ROOT}/x64/{cfg}/{DLL_NAME}"));
        p.exists().then_some(p)
    })
}

/// Chemin où la DLL sera copiée : `<dossier PJ64>/Plugin/PJ64OoTMMTracker.dll`.
pub fn plugin_dll_path(pid: u32) -> Option<PathBuf> {
    let pj64 = process_path(pid)?;
    let dir = pj64.parent()?;
    Some(dir.join("Plugin").join(DLL_NAME))
}

/// Indique si un module donné est chargé dans le processus (mirror
/// `IsModuleLoaded`). On énumère aussi les modules 32 bits depuis ce processus
/// 64 bits (`TH32CS_SNAPMODULE32`).
pub fn is_module_loaded(pid: u32, module_name: &str) -> bool {
    unsafe {
        let snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if snap == INVALID_HANDLE_VALUE || snap == 0 {
            return false;
        }
        let mut entry: ModuleEntry32W = std::mem::zeroed();
        entry.dw_size = std::mem::size_of::<ModuleEntry32W>() as u32;

        let mut found = false;
        if Module32FirstW(snap, &mut entry) != 0 {
            loop {
                if wide_to_string(&entry.sz_module).eq_ignore_ascii_case(module_name) {
                    found = true;
                    break;
                }
                if Module32NextW(snap, &mut entry) == 0 {
                    break;
                }
            }
        }
        CloseHandle(snap);
        found
    }
}

/// Callback `EnumWindows` : accumule les fenêtres appartenant au PID cherché.
extern "system" fn enum_windows_proc(hwnd: isize, lparam: isize) -> i32 {
    unsafe {
        let search = &mut *(lparam as *mut WinSearch);
        let mut pid = 0u32;
        GetWindowThreadProcessId(hwnd, &mut pid);
        if pid == search.pid {
            search.windows.push(hwnd);
        }
    }
    1 // TRUE : continuer l'énumération
}

/// Toutes les fenêtres top-level du processus PJ64 (mirror `GetProject64Windows`).
pub fn pj64_windows(pid: u32) -> Vec<isize> {
    let mut search = WinSearch { pid, windows: Vec::new() };
    unsafe {
        EnumWindows(enum_windows_proc, &mut search as *mut WinSearch as isize);
    }
    search.windows
}

/// Fenêtre principale de PJ64 : visible et sans propriétaire (mirror
/// `GetProject64Window`).
fn main_window(pid: u32) -> Option<isize> {
    unsafe {
        pj64_windows(pid)
            .into_iter()
            .find(|&hwnd| IsWindowVisible(hwnd) != 0 && GetWindow(hwnd, GW_OWNER) == 0)
    }
}

/// Envoie Ctrl+T à la fenêtre principale de PJ64 pour ouvrir les réglages (ce
/// qui force le chargement des plugins). Mirror `OpenProject64Settings`.
fn open_settings(hwnd: isize) -> bool {
    unsafe {
        if IsIconic(hwnd) != 0 {
            ShowWindow(hwnd, SW_RESTORE);
        }
        SetForegroundWindow(hwnd);
        // Laisser Windows effectuer le changement de fenêtre active.
        thread::sleep(Duration::from_millis(100));

        let key = |vk: u16, up: bool| Input {
            type_: INPUT_KEYBOARD,
            _pad: 0,
            ki: KeybdInput {
                w_vk: vk,
                w_scan: 0,
                dw_flags: if up { KEYEVENTF_KEYUP } else { 0 },
                time: 0,
                dw_extra_info: 0,
            },
            _tail: [0; 8],
        };
        // CTRL down, T down, T up, CTRL up.
        let inputs = [
            key(VK_CONTROL, false),
            key(VK_T, false),
            key(VK_T, true),
            key(VK_CONTROL, true),
        ];
        SendInput(inputs.len() as u32, inputs.as_ptr(), std::mem::size_of::<Input>() as i32) == 4
    }
}

/// La console de debug de la DLL (`AllocConsole`) apparaît elle aussi comme une
/// nouvelle fenêtre top-level de PJ64. La fermer plus tard avec WM_CLOSE
/// terminerait tout l'émulateur (fermer une console tue le process attaché) : on
/// l'ignore donc lors de la détection de la fenêtre de réglages.
fn is_console_window(hwnd: isize) -> bool {
    unsafe {
        let mut buf = [0u8; 64];
        let n = GetClassNameA(hwnd, buf.as_mut_ptr(), buf.len() as i32);
        if n <= 0 {
            return false;
        }
        String::from_utf8_lossy(&buf[..n as usize]) == "ConsoleWindowClass"
    }
}

/// Attend qu'une NOUVELLE fenêtre PJ64 (absente de `before`) apparaisse, jusqu'à
/// `timeout`. Mirror `WaitForSettingsWindow`.
fn wait_for_new_window(pid: u32, before: &[isize], timeout: Duration) -> Option<isize> {
    let start = Instant::now();
    while start.elapsed() < timeout {
        for hwnd in pj64_windows(pid) {
            if !before.contains(&hwnd) && unsafe { IsWindowVisible(hwnd) } != 0 {
                // Ne jamais retourner la console de debug : la refermer tuerait PJ64.
                if is_console_window(hwnd) {
                    continue;
                }
                return Some(hwnd);
            }
        }
        thread::sleep(Duration::from_millis(10));
    }
    None
}

/// Ferme une fenêtre (WM_CLOSE), utilisé pour refermer les réglages une fois la
/// DLL chargée.
pub fn close_window(hwnd: isize) {
    unsafe {
        PostMessageA(hwnd, WM_CLOSE, 0, 0);
    }
}

/// Résultat d'un chargement de plugin réussi.
pub struct Loaded {
    /// Chemin de la DLL copiée dans `Plugin/` (à supprimer au déchargement).
    pub dll_path: PathBuf,
    /// Fenêtre de réglages ouverte par Ctrl+T (à fermer une fois connecté).
    pub settings_window: Option<isize>,
}

/// Copie la DLL dans `Plugin/`, ouvre les réglages avec Ctrl+T et attend la
/// fenêtre de réglages. Étapes 3–6 de `StartMemoryReader`. La mémoire partagée
/// est ensuite attendue par le poller (qui referme la fenêtre une fois connecté).
/// `log` reçoit les messages de progression (repris tels quels du C++).
///
/// @return Les infos de nettoyage, ou un message d'erreur.
pub fn load_plugin(pid: u32, s: &LogStrings, log: &dyn Fn(&str)) -> Result<Loaded, String> {
    let dll_path = plugin_dll_path(pid).ok_or_else(|| s.err_no_exe_path.clone())?;
    let plugin_dir =
        dll_path.parent().ok_or_else(|| s.err_no_plugin_dir.clone())?.to_path_buf();
    let source = source_dll().ok_or_else(|| s.err_dll_missing.replace("{name}", DLL_NAME))?;

    if let Some(pj) = process_path(pid) {
        log(&s.pj64_path.replace("{path}", &pj.display().to_string()));
    }
    log(&s.plugin_dir.replace("{path}", &plugin_dir.display().to_string()));
    log(&s.tracker_dll.replace("{path}", &source.display().to_string()));

    // Le dossier Plugin doit exister pour que PJ64 y trouve la DLL ; on le crée
    // au besoin (PJ64 le rescanne à l'ouverture des réglages).
    let _ = std::fs::create_dir_all(&plugin_dir);

    // Copier la DLL. Si la copie échoue MAIS que le module est déjà chargé
    // (fichier verrouillé par une session précédente), on continue quand même.
    match std::fs::copy(&source, &dll_path) {
        Ok(_) => log(&s.dll_copied.replace("{path}", &dll_path.display().to_string())),
        Err(e) => {
            if !is_module_loaded(pid, DLL_NAME) {
                return Err(s.err_dll_copy.replace("{err}", &e.to_string()));
            }
        }
    }

    // Mémoriser les fenêtres existantes AVANT d'ouvrir les réglages.
    let before = pj64_windows(pid);

    let hwnd = main_window(pid).ok_or_else(|| s.err_no_main_window.clone())?;
    log(&s.main_window_found.replace("{hwnd}", &format!("{hwnd:X}")));
    // Ctrl+T is delivered to the focused PJ64 window; a fullscreen PJ64 swallows it.
    log(&s.windowed_note);
    if !open_settings(hwnd) {
        return Err(s.err_sendinput.clone());
    }
    log(&s.ctrl_t_sent);

    // Repérer la fenêtre de réglages fraîchement apparue (au plus 3 s, comme le C++).
    let settings_window = wait_for_new_window(pid, &before, Duration::from_millis(3000));

    Ok(Loaded { dll_path, settings_window })
}

/// Supprime la DLL copiée dans `Plugin/` (mirror `RemoveTrackerDLL`). Best-effort.
pub fn remove_tracker_dll(dll_path: &PathBuf) {
    let _ = std::fs::remove_file(dll_path);
}

/// Attend que le module DLL disparaisse du processus (après une demande de
/// Shutdown), jusqu'à `timeout`. Mirror de la boucle d'attente d'unload.
///
/// @return `true` si le module s'est bien déchargé dans le délai.
pub fn wait_for_unload(pid: u32, timeout: Duration) -> bool {
    let start = Instant::now();
    while start.elapsed() < timeout {
        if !is_module_loaded(pid, DLL_NAME) {
            return true;
        }
        thread::sleep(Duration::from_millis(50));
    }
    false
}
