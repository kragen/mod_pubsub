# general asyncore tools.

import asyncore

# a mixin for asyncore connections, allowing data to be queued, similar to
# asyncore.dispatcher_with_send, but much smaller.
class Writer:
    def __init__(self, socket):
        self.__data = ''
        self.socket = socket
        socket.setblocking(0)
        asyncore.socket_map[self] = 1

    # asyncore calls this to find out if we want to write data.
    def writable(self):
        return self.__data != ''

    # asyncore calls this when it senses that our fileno is writable.
    def handle_write_event(self):
        try: 
            n_bytes_sent = self.socket.send(self.__data)
        except socket.error, errcode:
            if (errcode[0] in (errno.ECONNRESET, errno.EPIPE)):
                self.close()
                return
            else: 
                raise
        self.__data = self.__data[n_bytes_sent:]
        self.log('sent %d bytes' % n_bytes_sent)

    # select.select() calls this; asyncore either calls it itself or lets
    # select do it.
    def fileno(self):
        return self.socket.fileno()

    # enqueue some data for writing.
    def write(self, str):
        self.__data = self.__data + str
    
    # in a functional or quaject world, this would be a callout or
    # functional parameter.  But we're in an OO world, so it's just a method
    # subclasses can override to handle debugging messages.
    def log(self, str):
        print "%s: %s" % (self, str)

    # deal with the connection being closed.  Because of the way
    # asyncore works, this doesn't guarantee we won't be called again.
    # so various routines need to be equipped to deal with that.
    # this unfortunately means we can't close the actual socket.
    def close(self):
        self.log('closed conn')
        if asyncore.socket_map.has_key(self):
            del asyncore.socket_map[self]

    # if we don't do this then asyncore makes errors useless
    def handle_error(*args): raise
