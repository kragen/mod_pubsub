#!/usr/bin/python

import sys, async_writer, socket, errno, time, string, asyncore, re

timer = time.time

if len(sys.argv) != 4:
    sys.stderr.write("%s: usage: %s servername topic nlisteners\n" % 
        (sys.argv[0], sys.argv[0]))
    sys.exit(1)

# assumes topic doesn't need URL-encoding
def ruthmagic(topic):
    return ("GET /magic?%s%%01onmsg%%01oninit"
        "%%01onload%%01onerror%%010 HTTP/1.0\r\n\r\n" % topic)

class Tunnel(async_writer.Writer):
    super = async_writer.Writer
    
    # number of running tunnels
    n = 0

    # number of running tunnels that have received headers
    rcvd_hdrs = 0

    def __init__(self, server, port, topic):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setblocking(0)
        try: sock.connect((server, port))
        except socket.error, errcode:
            if errcode[0] in (errno.EINPROGRESS, errno.EWOULDBLOCK): pass
            else: raise
        self.super.__init__(self, sock)
        self.write(ruthmagic(topic))
        self.linebuf = ''
        Tunnel.n = Tunnel.n + 1

    def readable(self): 
        return 1

    # this seems like it contains some very common handle_read_event code;
    # perhaps it should be factored out to somewhere else
    def handle_read_event(self):
        try: x = self.socket.recv(1024)
        except socket.error, errcode:
            if errcode[0] in (errno.ECONNRESET, errno.EPIPE):
                self.close()
                return
            else: raise
        self.log("got %d bytes at %s" % (len(x), timer()))
        inbuf = self.linebuf + x
        lines = string.split(inbuf, '\n')
        self.linebuf = lines.pop()
        for line in lines: self.handle_line(line)
        if x == '':
            self.close()
        # print x

    kn_id_re = re.compile('name="kn_id" value="([^"]*)"');
    def handle_line(self, line):
        if line == '\r':
            Tunnel.rcvd_hdrs = Tunnel.rcvd_hdrs + 1
        else:
            match = Tunnel.kn_id_re.search(line)
            if match is not None:
                print "%s %s %s" % (timer(), id(self), match.group(1))

    def close(self):
        self.handle_line(self.linebuf)
        self.linebuf = ''
        self.super.close(self)
        Tunnel.n = Tunnel.n - 1
        Tunnel.rcvd_hdrs = Tunnel.rcvd_hdrs - 1

    # override parent's log method with a nop
    def log(self, msg): pass

server, topic, nlisteners = sys.argv[1:]
nlisteners = int(nlisteners)

for i in range(nlisteners):
    Tunnel(server, 9092, topic)

ready = 0

while Tunnel.n > 0:
    sys.stderr.write("Going through the loop again with %d tunnels\n" % Tunnel.n)
    if not ready and Tunnel.rcvd_hdrs == Tunnel.n:
        ready = 1
        sys.stderr.write("Ready\n")
    asyncore.poll(None)
