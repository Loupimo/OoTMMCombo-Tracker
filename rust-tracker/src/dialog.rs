//! Dialogues de fichiers natifs Windows (ouvrir / enregistrer).
//!
//! Équivalent des `QFileDialog::getOpenFileName` / `getSaveFileName` du Qt
//! `LogTab`. Comme le reste du projet (shared_mem.rs, inject.rs), on appelle
//! directement l'API Win32 en `extern "system"` brut (`comdlg32`) plutôt que
//! d'ajouter une dépendance : la boîte de dialogue commune `GetOpenFileNameA` /
//! `GetSaveFileNameA` suffit et reste stable.

use std::path::PathBuf;

const MAX_PATH: usize = 260;

// Flags OPENFILENAME utilisés (winuser.h / commdlg.h).
const OFN_OVERWRITEPROMPT: u32 = 0x0000_0002;
const OFN_HIDEREADONLY: u32 = 0x0000_0004;
const OFN_NOCHANGEDIR: u32 = 0x0000_0008; // ne pas changer le CWD du process
const OFN_PATHMUSTEXIST: u32 = 0x0000_0800;
const OFN_FILEMUSTEXIST: u32 = 0x0000_1000;

/// Miroir de `OPENFILENAMEA` (commdlg.h). `#[repr(C)]` reproduit le padding de
/// l'ABI x64 ; le `const assert` verrouille la taille attendue (152 octets),
/// qui doit être écrite dans `l_struct_size`.
#[repr(C)]
struct OpenFileNameA {
    l_struct_size: u32,
    hwnd_owner: isize,
    h_instance: isize,
    lpstr_filter: *const u8,
    lpstr_custom_filter: *mut u8,
    n_max_cust_filter: u32,
    n_filter_index: u32,
    lpstr_file: *mut u8,
    n_max_file: u32,
    lpstr_file_title: *mut u8,
    n_max_file_title: u32,
    lpstr_initial_dir: *const u8,
    lpstr_title: *const u8,
    flags: u32,
    n_file_offset: u16,
    n_file_extension: u16,
    lpstr_def_ext: *const u8,
    l_cust_data: isize,
    lpfn_hook: usize,
    lpstr_template_name: *const u8,
    pv_reserved: *mut core::ffi::c_void,
    dw_reserved: u32,
    flags_ex: u32,
}

const _: () = assert!(std::mem::size_of::<OpenFileNameA>() == 152);

#[link(name = "comdlg32")]
extern "system" {
    fn GetOpenFileNameA(ofn: *mut OpenFileNameA) -> i32;
    fn GetSaveFileNameA(ofn: *mut OpenFileNameA) -> i32;
}

/// Construit la chaîne de filtre OPENFILENAME : des couples (libellé, motif)
/// séparés par des NUL, terminés par un NUL supplémentaire.
fn make_filter(pairs: &[(&str, &str)]) -> Vec<u8> {
    let mut v = Vec::new();
    for (label, pattern) in pairs {
        v.extend_from_slice(label.as_bytes());
        v.push(0);
        v.extend_from_slice(pattern.as_bytes());
        v.push(0);
    }
    v.push(0);
    v
}

/// Chaîne C terminée par NUL.
fn cstr(s: &str) -> Vec<u8> {
    let mut v = Vec::with_capacity(s.len() + 1);
    v.extend_from_slice(s.as_bytes());
    v.push(0);
    v
}

/// Lit le buffer de chemin (jusqu'au premier NUL) en `PathBuf`.
fn buf_to_path(buf: &[u8]) -> PathBuf {
    let end = buf.iter().position(|&c| c == 0).unwrap_or(buf.len());
    PathBuf::from(String::from_utf8_lossy(&buf[..end]).into_owned())
}

/// Prépare une structure OPENFILENAME commune aux deux dialogues.
///
/// SAFETY : les buffers `file`, `filter`, `title` (et `def_ext`) doivent rester
/// vivants pendant l'appel Win32 ; ils sont conservés par l'appelant.
fn build_ofn(
    file: &mut [u8; MAX_PATH],
    filter: *const u8,
    title: *const u8,
    def_ext: *const u8,
    flags: u32,
) -> OpenFileNameA {
    let mut ofn: OpenFileNameA = unsafe { std::mem::zeroed() };
    ofn.l_struct_size = std::mem::size_of::<OpenFileNameA>() as u32;
    ofn.lpstr_filter = filter;
    ofn.lpstr_file = file.as_mut_ptr();
    ofn.n_max_file = MAX_PATH as u32;
    ofn.lpstr_title = title;
    ofn.lpstr_def_ext = def_ext;
    ofn.flags = flags;
    ofn
}

/// Boîte de dialogue « Ouvrir un fichier ». `filter` = couples (libellé, motif),
/// ex. `&[("Tracking Files (*.trck)", "*.trck")]`.
///
/// @return Le chemin choisi, ou None si l'utilisateur a annulé.
pub fn open_file(title: &str, filter: &[(&str, &str)]) -> Option<PathBuf> {
    let mut file = [0u8; MAX_PATH];
    let filt = make_filter(filter);
    let title_c = cstr(title);
    let mut ofn = build_ofn(
        &mut file,
        filt.as_ptr(),
        title_c.as_ptr(),
        std::ptr::null(),
        OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
    );
    if unsafe { GetOpenFileNameA(&mut ofn) } != 0 {
        Some(buf_to_path(&file))
    } else {
        None
    }
}

/// Boîte de dialogue « Enregistrer sous ». `def_ext` (sans point, ex. "trck")
/// est ajoutée si l'utilisateur ne saisit pas d'extension.
///
/// @return Le chemin choisi, ou None si l'utilisateur a annulé.
pub fn save_file(title: &str, filter: &[(&str, &str)], def_ext: &str) -> Option<PathBuf> {
    let mut file = [0u8; MAX_PATH];
    let filt = make_filter(filter);
    let title_c = cstr(title);
    let ext_c = cstr(def_ext);
    let mut ofn = build_ofn(
        &mut file,
        filt.as_ptr(),
        title_c.as_ptr(),
        ext_c.as_ptr(),
        OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
    );
    if unsafe { GetSaveFileNameA(&mut ofn) } != 0 {
        Some(buf_to_path(&file))
    } else {
        None
    }
}
