//! Outgoing byte buffer for the multiplayer server socket (port of `NetBuffer`,
//! Sources/Multi/Buffer.cpp).
//!
//! Accumulates the bytes to transmit to the OoTMM server and drains them over a
//! non-blocking socket: a partial send just advances the cursor and the rest is
//! retried on the next tick. `is_empty` mirrors the C++ `size == 0` (true only
//! once every appended byte has been flushed), which the NOP heartbeat relies on.

use std::io::{self, Write};
use std::net::TcpStream;

/// A pending-transmit buffer with a send cursor (mirror of `NetBuffer`).
#[derive(Default)]
pub struct NetBuffer {
    data: Vec<u8>,
    pos: usize,
}

impl NetBuffer {
    pub fn new() -> Self {
        NetBuffer { data: Vec::with_capacity(4096), pos: 0 }
    }

    /// Reset the buffer without releasing its storage (`netBufClear`).
    pub fn clear(&mut self) {
        self.data.clear();
        self.pos = 0;
    }

    /// Whether every appended byte has been flushed (`netBufIsEmpty`).
    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
    }

    /// Append bytes to the end of the buffer (`netBufAppend`).
    pub fn append(&mut self, bytes: &[u8]) {
        self.data.extend_from_slice(bytes);
    }

    /// Send the pending buffer content over the socket, or drop it if networking
    /// is disabled (`netBufTransfer`). A `WouldBlock` leaves the remainder queued
    /// for the next tick; a full flush clears the buffer.
    ///
    /// @return Ok(()) on success (full or partial), Err on a fatal socket error.
    pub fn transfer(&mut self, sock: &mut TcpStream, net_enabled: bool) -> io::Result<()> {
        loop {
            if self.pos >= self.data.len() {
                self.clear();
                return Ok(());
            }
            if !net_enabled {
                self.clear();
                return Ok(());
            }
            match sock.write(&self.data[self.pos..]) {
                Ok(0) => return Err(io::ErrorKind::WriteZero.into()),
                Ok(n) => self.pos += n,
                Err(e) if e.kind() == io::ErrorKind::WouldBlock => return Ok(()),
                Err(e) if e.kind() == io::ErrorKind::Interrupted => continue,
                Err(e) => return Err(e),
            }
        }
    }
}
