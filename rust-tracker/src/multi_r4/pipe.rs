//! Windows named-pipe client for the r4 IPC channel (port of
//! `client/internal/ipc/project64_windows.go`).
//!
//! The custom Project64 build exposes a message-mode named pipe
//! `\\.\pipe\pj64em-ipc.<n>`. We connect with overlapped I/O so a blocking read
//! on the reader thread can be unblocked from another thread via `CancelIoEx`
//! (used at shutdown). Each direction owns its own event, so a read on the reader
//! thread and a write on the session thread run concurrently on the one handle.
//!
//! Raw kernel32 FFI (no windows-sys crate), matching `shared_mem.rs`.

use std::ffi::c_void;

// --- Minimal kernel32 declarations -------------------------------------------
#[link(name = "kernel32")]
extern "system" {
    fn CreateFileW(
        lp_file_name: *const u16,
        dw_desired_access: u32,
        dw_share_mode: u32,
        lp_security_attributes: *const c_void,
        dw_creation_disposition: u32,
        dw_flags_and_attributes: u32,
        h_template_file: isize,
    ) -> isize;
    fn SetNamedPipeHandleState(
        h_named_pipe: isize,
        lp_mode: *const u32,
        lp_max_collection_count: *const u32,
        lp_collect_data_timeout: *const u32,
    ) -> i32;
    fn CreateEventW(
        lp_event_attributes: *const c_void,
        b_manual_reset: i32,
        b_initial_state: i32,
        lp_name: *const u16,
    ) -> isize;
    fn ReadFile(
        h_file: isize,
        lp_buffer: *mut u8,
        n_number_of_bytes_to_read: u32,
        lp_number_of_bytes_read: *mut u32,
        lp_overlapped: *mut Overlapped,
    ) -> i32;
    fn WriteFile(
        h_file: isize,
        lp_buffer: *const u8,
        n_number_of_bytes_to_write: u32,
        lp_number_of_bytes_written: *mut u32,
        lp_overlapped: *mut Overlapped,
    ) -> i32;
    fn GetOverlappedResult(
        h_file: isize,
        lp_overlapped: *mut Overlapped,
        lp_number_of_bytes_transferred: *mut u32,
        b_wait: i32,
    ) -> i32;
    fn WaitForSingleObject(h_handle: isize, dw_milliseconds: u32) -> u32;
    fn ResetEvent(h_event: isize) -> i32;
    fn CancelIoEx(h_file: isize, lp_overlapped: *const Overlapped) -> i32;
    fn CloseHandle(h_object: isize) -> i32;
    fn GetLastError() -> u32;
    fn FindFirstFileW(lp_file_name: *const u16, lp_find_file_data: *mut FindDataW) -> isize;
    fn FindNextFileW(h_find_file: isize, lp_find_file_data: *mut FindDataW) -> i32;
    fn FindClose(h_find_file: isize) -> i32;
}

/// Win32 `WIN32_FIND_DATAW`, for enumerating the pipe namespace.
#[repr(C)]
struct FindDataW {
    dw_file_attributes: u32,
    ft_creation_time: [u32; 2],
    ft_last_access_time: [u32; 2],
    ft_last_write_time: [u32; 2],
    n_file_size_high: u32,
    n_file_size_low: u32,
    dw_reserved0: u32,
    dw_reserved1: u32,
    c_file_name: [u16; 260],
    c_alternate_file_name: [u16; 14],
}

const GENERIC_READ: u32 = 0x8000_0000;
const GENERIC_WRITE: u32 = 0x4000_0000;
const OPEN_EXISTING: u32 = 3;
const FILE_FLAG_OVERLAPPED: u32 = 0x4000_0000;
const PIPE_READMODE_MESSAGE: u32 = 0x0000_0002;
const ERROR_IO_PENDING: u32 = 997;
const ERROR_MORE_DATA: u32 = 234;
const INFINITE: u32 = 0xFFFF_FFFF;
const INVALID_HANDLE_VALUE: isize = -1;

/// Win32 `OVERLAPPED` (x64 layout: 32 bytes).
#[repr(C)]
struct Overlapped {
    internal: usize,
    internal_high: usize,
    offset: u32,
    offset_high: u32,
    h_event: isize,
}

impl Overlapped {
    fn new(event: isize) -> Overlapped {
        Overlapped { internal: 0, internal_high: 0, offset: 0, offset_high: 0, h_event: event }
    }
}

/// A connected message-mode named pipe. Shared behind an `Arc`: the reader thread
/// calls `read`, the session thread calls `write`; `cancel` unblocks a pending
/// read from a third thread; `Drop` closes the handles exactly once.
pub struct PipeConn {
    handle: isize,
    read_event: isize,
    write_event: isize,
}

// The handles are plain OS integers; the Win32 calls are internally synchronized
// per operation and each direction uses its own event, so concurrent read / write
// from different threads is sound.
unsafe impl Send for PipeConn {}
unsafe impl Sync for PipeConn {}

/// List the currently open `pj64em-ipc.*` pipes (`listPipes`).
///
/// The named-pipe filesystem is enumerated with `FindFirstFileW` on the pattern
/// `\\.\pipe\*` — Go's `os.ReadDir` and .NET's `Directory.GetFiles` do the same.
/// Rust's `std::fs::read_dir` can't be used here: it rewrites the path with a
/// verbatim `\\?\` prefix that the pipe device rejects (fails with "path not
/// found"), so the pipe would never be discovered.
pub fn list_pipes() -> Vec<String> {
    let mut out = Vec::new();
    unsafe {
        let pattern = wide(r"\\.\pipe\*");
        let mut data: FindDataW = std::mem::zeroed();
        let handle = FindFirstFileW(pattern.as_ptr(), &mut data);
        if handle == INVALID_HANDLE_VALUE {
            return out;
        }
        loop {
            let len = data.c_file_name.iter().position(|&c| c == 0).unwrap_or(data.c_file_name.len());
            let name = String::from_utf16_lossy(&data.c_file_name[..len]);
            if name.starts_with("pj64em-ipc.") {
                out.push(format!(r"\\.\pipe\{name}"));
            }
            if FindNextFileW(handle, &mut data) == 0 {
                break;
            }
        }
        FindClose(handle);
    }
    out
}

/// UTF-16, null-terminated (for the `*W` APIs).
fn wide(s: &str) -> Vec<u16> {
    s.encode_utf16().chain(std::iter::once(0)).collect()
}

impl PipeConn {
    /// Open the pipe at `path` in message read mode (`newPJ64Conn`).
    pub fn open(path: &str) -> Result<PipeConn, String> {
        unsafe {
            let name = wide(path);
            let handle = CreateFileW(
                name.as_ptr(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                std::ptr::null(),
                OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED,
                0,
            );
            if handle == INVALID_HANDLE_VALUE || handle == 0 {
                return Err(format!("CreateFile({path}) failed: {}", GetLastError()));
            }

            let mode = PIPE_READMODE_MESSAGE;
            if SetNamedPipeHandleState(handle, &mode, std::ptr::null(), std::ptr::null()) == 0 {
                let e = GetLastError();
                CloseHandle(handle);
                return Err(format!("SetNamedPipeHandleState failed: {e}"));
            }

            let read_event = CreateEventW(std::ptr::null(), 1, 0, std::ptr::null());
            let write_event = CreateEventW(std::ptr::null(), 1, 0, std::ptr::null());
            if read_event == 0 || write_event == 0 {
                if read_event != 0 {
                    CloseHandle(read_event);
                }
                if write_event != 0 {
                    CloseHandle(write_event);
                }
                CloseHandle(handle);
                return Err("CreateEvent failed".to_string());
            }

            Ok(PipeConn { handle, read_event, write_event })
        }
    }

    /// Read one whole message from the pipe (`PJ64Conn.Read`). Blocks until a
    /// message arrives, the pipe breaks, or a `cancel` aborts the read.
    pub fn read(&self) -> Result<Vec<u8>, String> {
        let mut buf = [0u8; 4096];
        let mut msg = Vec::new();
        loop {
            unsafe {
                let mut ov = Overlapped::new(self.read_event);
                ResetEvent(self.read_event);
                let mut n: u32 = 0;
                let ok = ReadFile(self.handle, buf.as_mut_ptr(), buf.len() as u32, &mut n, &mut ov);
                let (n, err) = self.wait_overlapped(self.read_event, ok, &mut ov);
                msg.extend_from_slice(&buf[..n as usize]);
                match err {
                    0 => return Ok(msg),
                    e if e == ERROR_MORE_DATA => continue, // partial message, keep reading
                    e => return Err(format!("pipe read failed: {e}")),
                }
            }
        }
    }

    /// Write one whole message to the pipe (`PJ64Conn.Write`).
    pub fn write(&self, data: &[u8]) -> Result<(), String> {
        unsafe {
            let mut ov = Overlapped::new(self.write_event);
            ResetEvent(self.write_event);
            let mut n: u32 = 0;
            let ok = WriteFile(self.handle, data.as_ptr(), data.len() as u32, &mut n, &mut ov);
            let (_, err) = self.wait_overlapped(self.write_event, ok, &mut ov);
            if err == 0 {
                Ok(())
            } else {
                Err(format!("pipe write failed: {err}"))
            }
        }
    }

    /// Cancel any pending I/O to unblock a read on another thread (shutdown).
    pub fn cancel(&self) {
        unsafe {
            CancelIoEx(self.handle, std::ptr::null());
        }
    }

    /// Wait for an overlapped operation started by `ReadFile` / `WriteFile` and
    /// return `(bytes, last_error)` (`waitOverlapped`; 0 = success).
    unsafe fn wait_overlapped(&self, event: isize, started_ok: i32, ov: *mut Overlapped) -> (u32, u32) {
        if started_ok == 0 {
            let e = GetLastError();
            if e != ERROR_IO_PENDING {
                return (0, e);
            }
            // Pending: block until the op completes (or is cancelled, which
            // signals the event and then surfaces as an error from GetOverlappedResult).
            if WaitForSingleObject(event, INFINITE) != 0 {
                CancelIoEx(self.handle, ov);
                return (0, GetLastError());
            }
        }
        let mut n: u32 = 0;
        let ok = GetOverlappedResult(self.handle, ov, &mut n, 0);
        if ok == 0 {
            (n, GetLastError())
        } else {
            (n, 0)
        }
    }
}

impl Drop for PipeConn {
    fn drop(&mut self) {
        unsafe {
            CancelIoEx(self.handle, std::ptr::null());
            CloseHandle(self.handle);
            CloseHandle(self.read_event);
            CloseHandle(self.write_event);
        }
    }
}
