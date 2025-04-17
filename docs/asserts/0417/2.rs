
impl FileIO for Socket {
    fn read(&self, buf: &mut [u8]) -> AxResult<usize> {
        if self.domain == Domain::AF_UNIX {
            return self.buffer.as_ref().unwrap().read(buf);
        }
        match &self.inner {
            SocketInner::Tcp(s) => s.recv(buf),
            SocketInner::Udp(s) => s.recv(buf),
        }
    }

    fn write(&self, buf: &[u8]) -> AxResult<usize> {
        if self.domain == Domain::AF_UNIX {
            return self.buffer.as_ref().unwrap().write(buf);
        }
        match &self.inner {
            SocketInner::Tcp(s) => s.send(buf),
            SocketInner::Udp(s) => s.send(buf),
        }
    }

    fn flush(&self) -> AxResult {
        Err(AxError::Unsupported)
    }

    fn readable(&self) -> bool {
        if self.domain == Domain::AF_UNIX {
            return self.buffer.as_ref().unwrap().readable();
        }
        poll_interfaces();
        match &self.inner {
            SocketInner::Tcp(s) => s.poll().map_or(false, |p| p.readable),
            SocketInner::Udp(s) => s.poll().map_or(false, |p| p.readable),
        }
    }

    fn writable(&self) -> bool {
        // if self.domain == Domain::AF_UNIX {
        //     return self.buffer.as_ref().unwrap().writable();
        // }
        poll_interfaces();
        match &self.inner {
            SocketInner::Tcp(s) => s.poll().map_or(false, |p| p.writable),
            SocketInner::Udp(s) => s.poll().map_or(false, |p| p.writable),
        }
    }

    fn executable(&self) -> bool {
        false
    }

    fn get_type(&self) -> FileIOType {
        FileIOType::Socket
    }

    fn get_status(&self) -> OpenFlags {
        let mut flags = OpenFlags::default();

        if self.close_exec.load(core::sync::atomic::Ordering::Acquire) {
            flags |= OpenFlags::CLOEXEC;
        }

        if self.is_nonblocking() {
            flags |= OpenFlags::NON_BLOCK;
        }

        flags
    }

    fn set_status(&self, flags: OpenFlags) -> bool {
        self.set_nonblocking(flags.contains(OpenFlags::NON_BLOCK));

        true
    }

    fn ready_to_read(&self) -> bool {
        self.readable()
    }

    fn ready_to_write(&self) -> bool {
        self.writable()
    }

    fn set_close_on_exec(&self, _is_set: bool) -> bool {
        self.close_exec
            .store(_is_set, core::sync::atomic::Ordering::Release);
        true
    }
}
