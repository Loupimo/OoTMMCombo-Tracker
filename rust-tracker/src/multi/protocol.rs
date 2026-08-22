//! Emulator API transport (port of Sources/Multi/Protocol.cpp + API.cpp).
//!
//! `ApiConn` speaks to the running emulator over a blocking TCP socket to read /
//! write the OoTMM "net context" in game memory. Two wire protocols are handled:
//!   * PROTOCOL_PJ64 — a custom Project64 build that connects to the tracker's
//!     listening socket, then serves raw read/write opcodes (values little-endian).
//!   * PROTOCOL_ARES — the Ares emulator's GDB remote stub (hex, big-endian).
//! Every transaction that hits a socket error latches `error` and drops the
//! socket, exactly like the C++ `game->apiError` path.

use std::io::{Read, Write};
use std::net::TcpStream;

/// The shared network context lives at a fixed RDRAM address; the tracker polls
/// its magic to know the game is a net-enabled OoTMM build (`API.h`).
pub const NET_GLOBAL_ADDR: u32 = 0x8000_01a0;
pub const NET_MAGIC: u32 = 0x905A_B56A;

/// Which wire protocol an emulator connection speaks.
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum Protocol {
    Pj64,
    Ares,
}

/// A blocking connection to the emulator API, plus the resolved net-context base
/// address and the sticky error flag (mirror of the API-side `Game` fields).
pub struct ApiConn {
    sock: Option<TcpStream>,
    protocol: Protocol,
    pub error: bool,
    pub net_addr: u32,
}

impl ApiConn {
    /// Wrap a freshly accepted / connected emulator socket and send the protocol
    /// handshake if needed (`protocolInit`: Ares expects a leading '+').
    pub fn new(sock: TcpStream, protocol: Protocol) -> Self {
        let mut c = ApiConn { sock: Some(sock), protocol, error: false, net_addr: 0 };
        if protocol == Protocol::Ares {
            c.ares_start();
        }
        c
    }

    /// Whether this connection speaks the Ares GDB protocol.
    pub fn is_ares(&self) -> bool {
        self.protocol == Protocol::Ares
    }

    /// Latch the error flag and drop the socket (mirror of the C++ error path).
    fn fail(&mut self) {
        self.error = true;
        self.sock = None;
    }

    /// Send every byte or fail (`sockSend`).
    fn send(&mut self, bytes: &[u8]) -> bool {
        let ok = match self.sock.as_mut() {
            Some(s) => s.write_all(bytes).is_ok(),
            None => false,
        };
        if !ok {
            self.fail();
        }
        ok
    }

    /// Receive exactly `buf.len()` bytes or fail (`sockRecv`).
    fn recv_exact(&mut self, buf: &mut [u8]) -> bool {
        let ok = match self.sock.as_mut() {
            Some(s) => s.read_exact(buf).is_ok(),
            None => false,
        };
        if !ok {
            self.fail();
        }
        ok
    }

    // ── PJ64 ─────────────────────────────────────────────────────────────────

    /// PJ64 read: `[op, addr(4 LE)]` then `size` value bytes.
    fn pj64_read(&mut self, op: u8, size: usize, addr: u32) -> [u8; 4] {
        let mut packet = [0u8; 5];
        packet[0] = op;
        packet[1..5].copy_from_slice(&addr.to_le_bytes());
        let mut val = [0u8; 4];
        if self.send(&packet) {
            self.recv_exact(&mut val[..size]);
        }
        val
    }

    /// PJ64 write: `[op, addr(4 LE), value(size)]`.
    fn pj64_write(&mut self, op: u8, size: usize, addr: u32, value: &[u8]) {
        let mut packet = Vec::with_capacity(5 + size);
        packet.push(op);
        packet.extend_from_slice(&addr.to_le_bytes());
        packet.extend_from_slice(&value[..size]);
        self.send(&packet);
    }

    // ── Ares GDB stub ────────────────────────────────────────────────────────

    fn ares_start(&mut self) {
        self.send(b"+");
    }

    /// GDB memory read: `$m{addr:08x},{count:x}#{cksum}` -> hex bytes.
    fn ares_read(&mut self, addr: u32, count: usize, out: &mut [u8]) -> bool {
        let body = format!("m{addr:08x},{count:x}");
        let cksum: u8 = body.bytes().fold(0u8, |a, b| a.wrapping_add(b));
        if !self.send(b"$") || !self.send(body.as_bytes()) || !self.send(format!("#{cksum:02x}").as_bytes()) {
            return false;
        }
        let mut ack = [0u8; 1];
        if !self.recv_exact(&mut ack) || ack[0] != b'+' {
            return false;
        }
        // '$' + count*2 hex chars + '#' + 2 checksum chars.
        let mut resp = vec![0u8; 1 + count * 2 + 3];
        if !self.recv_exact(&mut resp) || resp[0] != b'$' {
            return false;
        }
        for i in 0..count {
            out[i] = (unhex(resp[1 + i * 2]) << 4) | unhex(resp[1 + i * 2 + 1]);
        }
        if resp[1 + count * 2] != b'#' {
            return false;
        }
        self.send(b"+")
    }

    /// GDB memory write: `$M{addr:08x},{size:x}:{hex bytes}#{cksum}` -> "OK".
    fn ares_write(&mut self, addr: u32, value: &[u8]) -> bool {
        let mut body = format!("M{addr:08x},{size:x}:", size = value.len());
        for b in value {
            body.push_str(&format!("{b:02x}"));
        }
        let cksum: u8 = body.bytes().fold(0u8, |a, b| a.wrapping_add(b));
        if !self.send(b"$") || !self.send(body.as_bytes()) || !self.send(format!("#{cksum:02x}").as_bytes()) {
            return false;
        }
        let mut ack = [0u8; 1];
        if !self.recv_exact(&mut ack) || ack[0] != b'+' {
            return false;
        }
        let mut resp = [0u8; 1 + 2 + 3];
        if !self.recv_exact(&mut resp) || resp[0] != b'$' || &resp[1..4] != b"OK#" {
            return false;
        }
        self.send(b"+")
    }

    fn ares_read_int(&mut self, addr: u32, size: usize) -> u32 {
        let mut buf = [0u8; 4];
        if !self.ares_read(addr, size, &mut buf) {
            self.fail();
            return 0;
        }
        match size {
            4 => u32::from_be_bytes(buf),
            2 => ((buf[0] as u32) << 8) | buf[1] as u32,
            1 => buf[0] as u32,
            _ => 0,
        }
    }

    fn ares_write_int(&mut self, addr: u32, value: u32, size: usize) {
        // Big-endian bytes, high byte first (`aresWriteInt`).
        let all = value.to_be_bytes();
        let bytes = &all[4 - size..];
        if !self.ares_write(addr, bytes) {
            self.fail();
        }
    }

    // ── Protocol-agnostic accessors (`protocolReadX` / `protocolWriteX`) ──────

    pub fn read8(&mut self, addr: u32) -> u8 {
        match self.protocol {
            Protocol::Pj64 => self.pj64_read(2, 1, addr)[0],
            Protocol::Ares => self.ares_read_int(addr, 1) as u8,
        }
    }

    pub fn read32(&mut self, addr: u32) -> u32 {
        match self.protocol {
            Protocol::Pj64 => u32::from_le_bytes(self.pj64_read(4, 4, addr)),
            Protocol::Ares => self.ares_read_int(addr, 4),
        }
    }

    /// Read a raw byte buffer (PJ64: byte-by-byte; Ares: one bulk read).
    pub fn read_buffer(&mut self, addr: u32, out: &mut [u8]) {
        match self.protocol {
            Protocol::Pj64 => {
                for (i, b) in out.iter_mut().enumerate() {
                    *b = self.read8(addr + i as u32);
                }
            }
            Protocol::Ares => {
                if !self.ares_read(addr, out.len(), out) {
                    self.fail();
                }
            }
        }
    }

    pub fn write8(&mut self, addr: u32, value: u8) {
        match self.protocol {
            Protocol::Pj64 => self.pj64_write(6, 1, addr, &[value]),
            Protocol::Ares => self.ares_write_int(addr, value as u32, 1),
        }
    }

    pub fn write16(&mut self, addr: u32, value: u16) {
        match self.protocol {
            Protocol::Pj64 => self.pj64_write(7, 2, addr, &value.to_le_bytes()),
            Protocol::Ares => self.ares_write_int(addr, value as u32, 2),
        }
    }

    pub fn write32(&mut self, addr: u32, value: u32) {
        match self.protocol {
            Protocol::Pj64 => self.pj64_write(8, 4, addr, &value.to_le_bytes()),
            Protocol::Ares => self.ares_write_int(addr, value, 4),
        }
    }

    pub fn write_buffer(&mut self, addr: u32, buf: &[u8]) {
        match self.protocol {
            Protocol::Pj64 => {
                for (i, b) in buf.iter().enumerate() {
                    self.write8(addr + i as u32, *b);
                }
            }
            Protocol::Ares => {
                if !self.ares_write(addr, buf) {
                    self.fail();
                }
            }
        }
    }

    // ── Net-context mutex (`apiContextLock` / `apiContextUnlock`) ─────────────

    /// Try to acquire the OoTMM net-context lock in game memory. On success
    /// `net_addr` points at the live net context and returns true.
    pub fn context_lock(&mut self) -> bool {
        if self.read32(NET_GLOBAL_ADDR + 0x00) != NET_MAGIC {
            return false;
        }
        let mut attempts = 0;
        loop {
            // Take the mutex.
            self.write32(NET_GLOBAL_ADDR + 0x0c, 1);
            if self.read32(NET_GLOBAL_ADDR + 0x08) == 0 {
                // Re-validate the magic and the lock under the mutex.
                if self.read32(NET_GLOBAL_ADDR + 0x00) != NET_MAGIC {
                    self.write32(NET_GLOBAL_ADDR + 0x0c, 0);
                    return false;
                }
                if self.read32(NET_GLOBAL_ADDR + 0x0c) != 1 {
                    self.write32(NET_GLOBAL_ADDR + 0x0c, 0);
                    return false;
                }
                self.net_addr = self.read32(NET_GLOBAL_ADDR + 0x04);
                return true;
            }
            // Contended: back off and retry.
            self.write32(NET_GLOBAL_ADDR + 0x0c, 0);
            attempts += 1;
            if attempts > 20 || self.error {
                return false;
            }
        }
    }

    /// Release the net-context lock.
    pub fn context_unlock(&mut self) {
        self.write32(NET_GLOBAL_ADDR + 0x0c, 0);
        self.net_addr = 0;
    }
}

/// Decode a single hex digit (`unhex`).
fn unhex(ch: u8) -> u8 {
    match ch {
        b'0'..=b'9' => ch - b'0',
        b'A'..=b'F' => ch - b'A' + 10,
        b'a'..=b'f' => ch - b'a' + 10,
        _ => 0,
    }
}
