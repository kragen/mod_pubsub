#!/usr/bin/python

"""
    pubsub.py -- standalone PubSub Python Server
    compatible with mod_pubsub & cgi-bin/pubsub.cgi

    This is a PubSub Server that runs standalone from the command line.
    It is our goal to have the functionality of pubsub.py match that
    of cgi-bin/pubsub.cgi so we have two reference implementations.

    Contact Information:
       http://mod-pubsub.sf.net/
       mod-pubsub-developer@lists.sourceforge.net
"""

# Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.
#
# @KNOWNOW_LICENSE_START@
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.
#
# 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
# be used to endorse or promote any product without prior written
# permission from KnowNow, Inc.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# @KNOWNOW_LICENSE_END@
#
# $Id: pubsub.py,v 1.22 2003/05/06 23:32:12 ifindkarma Exp $


"""
    This server uses a protocol compatible with the other PubSub
    Servers, and serves as a fine reference tutorial for learning
    the PubSub Protocol.

    We use the following standard Python libraries:
    asyncore.py, cgi.py, cgitb.py, inspect.py, pydoc.py

    Our system contains a Server, some Connections to clients, some
    Topics, some Routes, and some other Events.

    The Server has a root Topic.
    The Server reacts to new TCP connections by creating Connections.
    A Connection handles a request from a client; it might serve up
      a server status page, create an Event and post it to a Topic,
      create a Route and post it to a Topic, or become a tunnel.
    Connections log information about their activities to the Server.

    A Topic is a collection of Events --- possibly Topics or Routes.  It
      also has another (possibly null) Topic that contains its Routes,
      and another Topic that is contains its subtopics (including the
      kn_routes topic).
    You can ask a Topic to navigate to some path below it.
    Route and Topic are kinds of Event.

    There are two ways to post an Event to a Topic --- "notify", which
    does checks appropriate to do_method=notify, and "post", which
    doesn't.  Routes and do_method=notify use "notify"; everything else
    uses "post".

    Posting an Event to a Topic causes the Event to get posted to all
    the Routes on the route topic of the Topic --- at first immediately,
    but it is planned to make this happen gradually.

    Every Topic knows its name.

    The subtopics of a Topic are the events in the Topic's kn_subtopics
    topic.

    When creating a Route, a Connection doesn't talk directly to the
    kn_routes subtopic; instead it calls create_route() on the topic
    it is routing out of.  This handles IRP and calls post on the
    kn_routes subtopic.

"""


PermissionDenied = "403 Permission denied"
StaleTopic = "404 Expired"
Error = "pubsub.py error"

import sys, os, asyncore, string, socket, time, cgi, urlparse, urllib, os.path, re, cgitb, getopt, traceback, errno
from cPickle import dump, load
from serverutils import *

def quopri_encode(s, chars):
    """Quoted-printable encode: return a version of 's' with the chars
    in 'chars' quoted-printable encoded.  Normally 'chars' should
    contain at least '='."""
    rv = []
    s = str(s)
    for c in s:
        if c in chars: rv.append('=' + "%02X" % ord(c))
        else: rv.append(c)
    return string.join(rv, '')

class UuidGen:
    def __init__(self, prefix):
        self.prefix = prefix
        self.n = 0
    def __call__(self):
        self.n = self.n + 1
        return "%s_%s" % (self.prefix, self.n)

uuid = UuidGen(os.getpid())

class Logger:
    def __init__(self, errlog, verbose=0):
        self.errlog = errlog
        self.verbose = verbose
    def log_err(self, err):
        err = str(err)
        errlines = string.split(err, '\n')
        log_err = "%s\n%s" % (time.time(),
                               string.join(map(lambda x: '    %s\n' % x, errlines), ''))
        # FIXME: Handle log file size overflow.
        # Perhaps log rotation or compaction?
        self.errlog.write(log_err)
        self.errlog.flush()
        if self.verbose:
            sys.stderr.write(log_err)
            sys.stderr.flush()

logger = None # Server inits this

class Scheduler:
    """Responsibilities: Maintain a list of items to be done at
    specific times in the future; run items to be done at present or
    in the past; tell event loop how long until the next scheduled
    task."""
    class Task:
        def __init__(self, func, when, name):
            self.func = func
            self.when = when
            self.name = name
        def __call__(self):
            self.func()
        def __repr__(self):
            return "Task(%s, %s, %s)" % (repr(self.func), repr(self.when), repr(self.name))
    def __init__(self):
        self.schedule = []
    def schedule_processing(self, func, when, name="unknown"):
        self.schedule.append(self.Task(func, when, name))
    def run(self):
        oschedule = self.schedule
        self.schedule = []
        now = time.time()
        for i in oschedule:
            if i.when <= now:
                try: i()
                except:
                    logger.log_err("Error in scheduled task " + i.name + "\n" +
                                   cgitb.html())
            else:
                self.schedule.append(i)
    def timeout(self):
        if self.schedule == []:
            return None
        when = self.schedule[0].when
        for i in self.schedule:
            if when > i.when: when = i.when
        diff = when - time.time()
        if diff < 0: return 0
        else: return diff

class Event:
    """Responsibilities: hold a dictionary of names and values.
    Provide read access to keys and values.  Expire when it's time.
    Compare itself with other Events."""
    
    def __init__(self, contents):
        self.contents = {}
        for key in contents.keys():
            self.contents[key] = contents[key]
        if not self.contents.has_key('kn_id'):
            self.contents['kn_id'] = uuid()
        if not self.contents.has_key('kn_time_t'):
            self.contents['kn_time_t'] = time.time()
        self.kn_expires = None
        if self.contents.has_key('kn_expires'):
            self.kn_expires = absolute_expiry(self.contents['kn_expires'])
        else:
            self.kn_expires = None
        if not self.contents.has_key('kn_payload'):
            self.contents['kn_payload'] = ''
    def is_expired(self):
        return self.kn_expires is not None and self.kn_expires < time.time()
    def clone(self, changes):
        """Create a copy of this event with specified changes."""
        nev = Event(self)
        for key in changes.keys():
            nev.contents[key] = changes[key]
        return nev
    def __getitem__(self, name):
        """Return the named header."""
        return self.contents[name]
    def has_key(self, name): return self.contents.has_key(name)
    def keys(self): return self.contents.keys()
    def __repr__(self): return "Event(%s)" % self.contents
    def __cmp__(self, other):
        if isinstance(other, Event):
            for key in self.keys() + other.keys():
                # FIXME: this should implement the correct dup-squashing algorithm
                # instead.
                if key not in ['kn_route_location', 'kn_time_t', 'kn_routed_from', 'kn_route_id', 'kn_route_checkpoint']:
                    if not self.has_key(key): return -1
                    if not other.has_key(key): return 1
                    if self[key] != other[key]: return cmp(self[key], other[key])
            return 0
        else: raise Error, "Tried to compare event to %s" % other

def is_bad_topic_name(name):
    """Duplicates the stupid gratuitous topic-name rejection done by
    pubsub.cgi so as to provide an easy way of causing posting and
    subscribing to fail."""
    for c in name:
        if (not ('a' <= c <= 'z') and
            not ('A' <= c <= 'Z') and
            not ('0' <= c <= '9') and
            not ('\200' <= c <= '\377') and
            not (c in '-_.')):
            return 1
    if c[0] == '.' or c[-1] == '.': return 1
    return 0

class Topic(Event):
    """Responsibilities: hold a collection of Events, accessible by
    name, but kept in sequence of last update.  Provide access to
    descendants."""

    def __init__(self, name):
        Event.__init__(self, {'kn_payload': len(name) and name[-1] or "nobody should ever see the root topic's kn_payload"})
        self.name = name
        self.kn_subtopics = None
        self.events = []
        self.eventdict = {}
    def get_descendant(self, name):
        """Find a subtopic by name.  get_descendant(root, ['a', 'b']) should return the topic /a/b;
        get_descendant(basetopic, ['c', 'd']) where basetopic is /a/b should return /a/b/c/d.  Creates
        the topic if necessary."""
        if name == []: return self
        else: return self.get_subtopic(name[0]).get_descendant(name[1:])
    def get_subtopic(self, name):
        if is_bad_topic_name(name):
            raise PermissionDenied, "Bad topic name '%s'" % name
        # kn_subtopics is handled specially, because obviously we can't look in kn_subtopics to find it
        if name == 'kn_subtopics':
            if self.kn_subtopics is None:
                self.kn_subtopics = create_topic(self.name + [name])
            return self.kn_subtopics
        else:
            subtopics = self.get_subtopic('kn_subtopics')
            events = subtopics.get_events()
            for ev in events:
                if ev['kn_payload'] == name: return ev
            # hmm, we didn't find it, guess we should create it.
            top = create_topic(self.name + [name])
            subtopics.post(top)
            return top
    def create_route(self, route):
        """Given a Route, add it to the routes subtopic."""
        self.verify_route_ok(route)
        if self.get_subtopic('kn_routes').post(route):
            route.populate(self)
    def verify_route_ok(self, route):
        if not route.is_static_route():
            raise PermissionDenied, "Only static routes are allowed from regular topics like %s" % self.getname()
    def post(self, event):
        """Given an Event, add it to the topic, and route it to the
        appropriate places before returning."""
        if self.update_event(event):
            if self.kn_subtopics is not None:  # a kludge to avoid infinite recursion
                kn_routes = self.get_subtopic('kn_routes')
                for route in kn_routes.get_events():
                    if not route.post(event):
                        poison = route.poisoned()
                        kn_routes.post(poison)
            return 1
        else: return 0
    def get_events(self):
        return filter(lambda x: x is not None, self.events)
    def notify(self, event):
        """Given an Event, post it --- if this is allowed."""
        self.post(event)
    def getname(self):
        return string.join(map(urllib.quote, self.name), '/')
    def update_event(self, event):
        kn_id = event['kn_id']
        if self.eventdict.has_key(kn_id):
            oei = self.eventdict[kn_id]
            if self.events[oei] == event: return 0 # dup squashing
            self.events[oei] = None
        self.eventdict[kn_id] = len(self.events)
        self.events.append(event)
        return 1

class NoPostingTopic(Topic):
    def notify(self, event):
        raise PermissionDenied, "Can't post to %s" % self.getname()

class JournalTopic(Topic):
    def verify_route_ok(self, route):
        if route.is_static_route():
            raise PermissionDenied, "Can't create a static route from a journal topic."
    def update_event(self, event):
        self.events.append(event.clone({'kn_route_checkpoint': uuid()}))
        return 1
    def notify(self, event):
        routes = self.get_subtopic('kn_routes').get_events()
        for r in routes:
            if not r.is_stale():
                self.post(event)
                return
        raise StaleTopic, "No non-stale routes from %s" % self.getname()



# Create the appropriate sort of topic to be 'name'.
def create_topic(name):
    if name == []: return Topic(name)  # a root topic
    elif name[-1] in ['kn_subtopics', 'kn_routes']: return NoPostingTopic(name)
    elif name[-1] == 'kn_journal': return JournalTopic(name)
    else: return Topic(name)

class Route(Event):

    """Virtual base class; relies on subclasses to implement post()
    and is_static_route().  Responsibilities: handle initial route
    population.  Inform code readers that the various kinds of route
    have more in common than just being kinds of Events."""

    def populate(self, topic):
        if self.has_key('do_max_age'):
            now = time.time()
            max_age = self['do_max_age']
            for event in topic.get_events():
                if max_age == 'infinity' or now - float(max_age) <= float(event['kn_time_t']):
                    self.post(event)
        elif self.has_key('do_max_n'):
            events = topic.get_events()
            max_n = int(self['do_max_n'])
            if max_n < len(events):
                events = events[-max_n:]
            for event in events:
                self.post(event)
        elif self.has_key('do_since_checkpoint'):
            found_id = 0
            for ev in topic.get_events():
                if found_id: self.post(ev)
                if ev['kn_route_checkpoint'] == self['do_since_checkpoint']:
                    found_id = 1
            if not found_id:
                for ev in topic.get_events(): self.post(ev)
    def post(self, event): raise Error, "post not implemented"
    def is_static_route(self): raise Error, "is_static_route not implemented"
    def close(self): raise Error, "close not implemented"
    def is_stale(self): return 0
    def poisoned(self): return self

class StaticRoute(Route):
    """Responsibilities: route events to somewhere else."""
    def __init__(self, kn_to=None, location=None, misc={}):
        Event.__init__(self, misc)
        self.kn_to = kn_to
        self.location = location
        if misc.has_key('kn_uri'):
            self.kn_uri = misc['kn_uri']
        elif location is not None:
            self.kn_uri = '%s/kn_routes/%s' % (location, self['kn_id'])
        else:
            self.kn_uri = None
        if misc.has_key('kn_content_filter'): self.content_filter = re.compile(misc['kn_content_filter'])
        else: self.content_filter = None
    def poisoned(self):
        return StaticRoute(None, self.location, self.clone({'kn_payload': '', 'kn_expires': '+300'}))
    def post(self, event):
        """Forward an event to the destination.  Returns a success
        value; if it returns false, this route will be removed."""
        # hmm, this early return is going to make do_max_n interesting.
        if event.is_expired(): return 1
        if self.is_expired(): return 1
        if self.kn_to is None: return 1  
        # and this one too.
        if self.content_filter is not None and not self.content_filter.search(event['kn_payload']): return 1
        changes = {'kn_route_id': self['kn_id']}
        if self.kn_uri is not None: changes['kn_route_location'] = self.kn_uri
        if self.location is not None: changes['kn_routed_from'] = self.location
        try: self.kn_to.notify(event.clone(changes))
        except: return 0
        return 1
    def is_static_route(self): return 1
    def close(self): pass

# FIXME: These guys need to get deleted sometime!
class Tunnel(Route):

    """Responsibilities: format a sequence of events and send them to
    a Connection.  Behave as a route.

    """

    def __init__(self, connection):
        Event.__init__(self, {'kn_payload': str(connection), 'stale': '0'})
        self.conn = connection
        self.conn.report_status("tunnel")
        self.header_sent = 0
        self.dead = 0
        
    def post(self, event):
        # this assumes that it's always safe to write a space to a
        # tunnel as a keepalive byte.  This is true for our existing
        # tunnel formats, but maybe won't be true for every future
        # tunnel format; at that time we'll need to fix this.
        class TunnelTickler:
            def __init__(self, route, conn, scheduler):
                self.route = route
                self.conn = conn
                self.scheduler = scheduler
                self()
            def __call__(self):
                if self.conn.closed:
                    self.route.close()
                else:
                    try:
                        self.conn.send(' ')
                        self.scheduler.schedule_processing(self, time.time() + .5, 'tickle tunnel')
                    except:
                        self.conn.log_err("tickling problem on %s:" % self.conn +
                                          cgitb.html())
                        self.conn.close()
        if self.dead:
            return 0
        if not self.header_sent:
            self.conn.send(self.headerfrom(event))
            TunnelTickler(self, self.conn, self.conn.server.scheduler)
            self.header_sent = 1
        self.conn.report_status("tunnel sending event %s" % event['kn_id'])
        self.conn.tickle_renderer = 1
        #gsb print "%s" % self.encode(event)
        self.conn.send(self.encode(event))
        return 1
        
    def is_static_route(self): return 0
    def is_stale(self): return self['stale'] != '0'
    def become_stale(self):
        # XXX this doesn't cause the update to be properly routed
        self.contents['stale'] = '1'
    def close(self):
        if self.dead: return
        self.conn.finish_sending()
        self.conn.server.scheduler.schedule_processing(lambda self=self: self.become_stale(), time.time() + 100,
                                                       'stalify old tunnel')
    # for pickling
    def __getstate__(self):
        return {'contents': {'kn_payload': self['kn_payload'], 'stale': 1},
                'dead': 1} # and omit 'header_sent' and 'conn'


class SimpleTunnel(Tunnel):

    """Responsibilities: format events in the
    kn_response_format=simple format."""

    def headerfrom(self, event):
        return http_header(event['status'], 'text/plain')

    def ev_encode(_, dict):
        """Given a dictionary, return a string representing the
        dictionary's contents in the kn_response_format=simple
        encoding for events."""
        rv = []
        for hdr in dict.keys():
            if hdr != 'kn_payload':
                name = quopri_encode(hdr, '=:\n')
                value = quopri_encode(dict[hdr], '=\n')
                rv.append("%s: %s\n" % (name, value))
        rv.append("\n")
        if dict.has_key('kn_payload'):
            rv.append(dict['kn_payload'])
        try: return string.join(rv, '')
        except: print repr(rv); raise

    def encode(self, dict):
        """Same as ev_encode, except including the count and
        separators, so it can actually be sent on a tunnel."""
        str = self.ev_encode(dict)
        return "%d\n%s\n" % (len(str), str)

class JavaScriptTunnel(Tunnel):
    def headerfrom(self, event):
        return (http_header(event['status'], 'text/html; charset=utf-8') +
                "<html><head><title>%s</title>" % event['status'] +
                html_prologue_string(self._jsTunnel_conn) + "</head>\n" +
                '<body bgcolor="#f0f0ff" %s>\n' % (self.watching and (' onload="if (parent.kn_tunnelLoadCallback) ' +
                                                   'parent.kn_tunnelLoadCallback(window)"') or '') +
                "<h1>%s</h1>" % event['status'] + event['html_payload'] + '<!--');
    def __init__(self, conn, watching):
        Tunnel.__init__(self, conn)
        self.watching = watching
        self._jsTunnel_conn = conn
    def encode(self, dict):
        return ('--><script type="text/javascript"><!--\n' +
                'if (parent.kn_sendCallback) parent.kn_sendCallback({elements:[\n' +
                string.join(map(lambda name, dict=dict, self=self: self.encode_pair(name, dict[name]),
                                dict.keys()), ',\n') +
                ']}, window);\n' +
                '// -->\n' +
                '</script><!--\n')
    def encode_pair(self, name, value):
        return '{%s, %s}' % (self.js_encode_pair('name', str(name)),
                             self.js_encode_pair('value', str(value)))
    def js_encode_pair(self, name, value):
        return '%s: "%s"' % ((self.is_utf8(value) and name + 'U' or name),
                             self.js_encode_string(value))
    def is_utf8(self, value):
        try:
            for c in value:
                if '\x80' <= c <= '\xff': return 1
            return 0
        except:
            raise
            
    def js_encode_string(self, value):
        rv = []
        for c in value:
            if c in ['\\', '"']: rv.append('\\' + c)
            elif c == '\f': rv.append('\\f')
            elif c == '\n': rv.append('\\n')
            elif c == '\r': rv.append('\\r')
            elif c == '\t': rv.append('\\t')
            elif c == '<' or not (' ' <= c <= '~'): rv.append('\\x%2.2x' % ord(c))
            else: rv.append(c)
        return string.join(rv, '')
    def close(self):
        self.conn.send('-->\n</body></html>\n');
        Tunnel.close(self)

class ServerSaver:
    def __init__(self, root, filename, interval, scheduler):
        self.root = root
        self.filename = filename
        self.interval = interval
        self.scheduler = scheduler
        self.reschedule()
    def __call__(self):
        write_event_pool(self.filename, self.root)
        self.reschedule()
    def reschedule(self):
        self.scheduler.schedule_processing(self, time.time() + self.interval)

class Server:
    """Responsibilities: accept incoming connections and create
    Connection objects for them.  Track overall server state."""
    def __init__(self, portnum, logfile, errlog, scheduler, docroot, knroot, verbose, poolfile, ignorePrologue):
        self.logfile = logfile
        global logger
        logger = Logger(errlog)
        self.scheduler = scheduler
        self.alive = 1
        self.docroot = docroot
        self.knroot = knroot
        self.verbose = verbose
        self.ignorePrologue = ignorePrologue
        self.root_topic = read_event_pool(poolfile)
        ServerSaver(self.root_topic, poolfile, 30, self.scheduler)

        self.portnum = portnum
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.bind(("", portnum))
        self.socket.listen(5)
        asyncore.socket_map[self] = 1

        self.connstatus = {}
        self.peers = {}
        self.openedconns = 0
        self.closedconns = 0
        self.starttime = time.time()

        self.log(self, 'server', 'start', '')
    def getname(self): return 'pubsub.Server'
    def readable(self): return 1
    def writable(self): return 0
    def fileno(self): return self.socket.fileno()
    def handle_error(self, *args):
        logger.log_err("server error: " + cgitb.html())
    def handle_read_event(self):
        (conn, addr) = self.socket.accept()
        nc = Connection(conn, self, addr)
        self.openedconns = self.openedconns + 1
    def connection_address_is(self, conn, addr):
        self.peers[id(conn)] = addr
    def log(self, where, family, event, msg):
        log_msg = "%s %s: <%s/%s> %s\n" % (time.time(), id(where), family, event, msg)
        self.logfile.write(log_msg)
        self.logfile.flush()
        if self.verbose > 1:
            sys.stderr.write(log_msg)
            sys.stderr.flush()
    def report_status(self, conn, status):
        self.connstatus[id(conn)] = status
        self.log(conn, 'connection', self.peers[id(conn)], status)
        #gsb print(conn, 'connection', self.peers[id(conn)], status)
    def connection_closed(self, conn):
        del self.connstatus[id(conn)]
        del self.peers[id(conn)]
        self.closedconns = self.closedconns + 1
    def server_status(self):
        hdr = """<html><head><title>Server status</title></head><body bgcolor="white">
        <h1>Server status</h1>
        <p> Up for %d seconds; %d connections opened (%d still active).</p>
        <ul>
        """ % (time.time() - self.starttime, self.openedconns, self.openedconns - self.closedconns)
        conns = map(lambda x, self=self: "<li> %x: %s: %s</li>\n" % (x, self.peers[x], self.connstatus[x]),
                    self.connstatus.keys())
        return hdr + string.join(conns, '') + "</ul></body></html>\n"
    def kill(self): self.alive = 0
    def documentroot(self): return self.docroot
    def getknroot(self): return self.knroot

def route_get_topic(conn, uri, query):
    """Given a connection, a request-URI, and a query object, return
    the topic a route request (tunnel or other) with those parameters
    should route from."""
    if query.has_key('kn_from'):
        uri = query['kn_from'][0]
    return conn.get_topic(uri)

def absolute_expiry(value, base = 0.0):
    # print 'absolute_expiry' + str((value, base))
    if value is None: return value
    if value[0] == '+': return absolute_expiry(value[1:], time.time() + base)
    if value=="infinity": return None
    if value=="now": value = time.time()
    return base + float(value)

def status_event(query, status, payload='', htmlpayload=None):
    if htmlpayload is None: htmlpayload = cgi.escape(payload)
    if query.has_key('kn_status_from'): kn_route_location = query['kn_status_from'][0]
    else: kn_route_location = ''
    return Event({'kn_route_location': kn_route_location,
                  'status': status,
                  'kn_payload': payload,
                  'html_payload': htmlpayload})

def tunnel(conn, uri, httpreq, query):
    class TunnelCloser:
        def __init__(self, route): self.route = route
        def __call__(self): self.route.close(); self.route.become_stale()
    if query.has_key('kn_response_format') and query['kn_response_format'][0] == 'simple':
        route = SimpleTunnel(conn)
    else: # FIXME: handle unknown response format
        route = JavaScriptTunnel(conn, watching=1)
    topic = route_get_topic(conn, uri, query)
    topic.create_route(route)
    conn.report_status('tunnel from %s' % topic.getname())
    route.post(status_event(query, '200 Watching topic', "watching %s" % topic.getname()))
    # And we DON'T finish sending.  Not until much later.  But there might be expiry:
    if query.has_key('kn_expires'):
        conn.server.scheduler.schedule_processing(TunnelCloser(route), absolute_expiry(query['kn_expires'][0]),
                                                  'tunnel closer')

def query_to_event(q):
    ev = {}
    for header in q.keys():
        if header not in ['do_method', 'kn_from', 'kn_to', 'kn_route_checkpoint', 'kn_debug', 'kn_status_from', 'kn_status_to',
                          'kn_response_format']:
            ev[header] = q[header][0]
    return Event(ev)

def create_status_route(conn, query):
    if query.has_key('kn_status_to'):
        rv = StaticRoute(conn.get_topic(query['kn_status_to'][0]))
        conn.send("HTTP/1.0 204 No Content\r\n\r\n")
        conn.finish_sending()
        return rv
    else:
        if query.has_key('kn_response_format') and query['kn_response_format'][0] == 'simple':
            return SimpleTunnel(conn)
        else:
            return JavaScriptTunnel(conn, watching=0)

def dump_exc():
    (type, value, tb) = sys.exc_info()
    return string.join(traceback.format_exception(type, value, tb), '')

def add_static_route_internal(conn, uri, query):
    topic = route_get_topic(conn, uri, query)
    kn_to = conn.get_topic(query['kn_to'][0])
    conn.report_status('route from %s to %s' % (topic.getname(), kn_to.getname()))
    query['kn_payload'] = [query['kn_to'][0]]
    route = StaticRoute(kn_to, "%s/%s/%s" % (conn.getservername(), conn.urlroot(), topic.getname()), query_to_event(query))
    topic.create_route(route)

def notify_internal(conn, uri, query):
    if query.has_key('kn_to'):
        uri = query['kn_to'][0]
    topic = conn.get_topic(uri)
    event = query_to_event(query)
    conn.report_status('notify of %s on %s' % (event['kn_id'], topic.getname()))
    topic.notify(event)

def run_batchable_internal(conn, uri, query, routine, output):
    try:
        routine(conn, uri, query)
        output.post(status_event(query, '200 OK'))
    except:
        output.post(status_event(query, '500 Internal Server Error', dump_exc(), cgitb.html()))
        conn.log_err('(%s)\n' % routine + cgitb.html())
        
def run_batchable(conn, uri, httpreq, query, routine):
    output = create_status_route(conn, query)
    run_batchable_internal(conn, uri, query, routine, output)
    output.close()

def add_static_route(conn, uri, httpreq, query):
    run_batchable(conn, uri, httpreq, query, add_static_route_internal)
def notify(conn, uri, httpreq, query):
    run_batchable(conn, uri, httpreq, query, notify_internal)
    
def do_help(conn):
    conn.report_status('sending help')
    conn.send(http_header('200 OK', 'text/html') + "No help for you.\n")
    conn.finish_sending()

def do_blank(conn):
    conn.report_status('sending blank')
    conn.send(http_header('200 OK', 'text/html; charset=utf-8') +
              "<html><head><title>This Space Intentionally Left Blank</title>\n" +
              html_prologue_string(conn) +
              '<script type="text/javascript">\n' +
              '<!--\n' +
              'if (parent.kn_redrawCallback) { parent.kn_redrawCallback(window); }\n' +
              """else { document.write('<body bgcolor="white"></body'); }\n""" +
              "// -->\n" +
              "</script>\n</head>\n")
    conn.finish_sending()

def do_batch(conn, query):
    output = create_status_route(conn, query)
    if not query.has_key('kn_batch'): query['kn_batch'] = []
    output.post(status_event(query, '200 batching',
                             'batching %d requests' % len(query['kn_batch'])))
    for item in query['kn_batch']:
        subquery = cgi.parse_qs(item, keep_blank_values=1)
        if subquery.has_key('do_method'):
            do_method = subquery['do_method'][0]
            if do_method == 'notify':
                run_batchable_internal(conn, '/', subquery, notify_internal, output)
            elif do_method == 'route':
                run_batchable_internal(conn, '/', subquery, add_static_route_internal, output)
    output.close()

Error400 = 'Request Error'

def parse_http_request_header(str):
    header = parse_http_header(str)
    reqlinechunks = string.split(header['firstline'], ' ')
    if len(reqlinechunks) != 3:
        raise Error400, "Wrong number of spaces in request line %d" % len(reqlinechunks)
    del header['firstline']
    header['method'], header['uri'], header['version'] = reqlinechunks
    return header

def parse_http_response_header(str):
    header = parse_http_header(str)
    responselinechunks = string.split(header['firstline'], ' ', 2)
    del header['firstline']
    header['version'], header['code'], header['message'] = responselinechunks
    return header

def parse_http_header(str):
    lines = string.split(str, '\r\n')
    headers = []
    for i in range(1, len(lines)):
        if lines[i][0] in [' ', '\t']:
            if len(headers) == 0:
                raise Error, ("First HTTP header line begins with whitespace", lines[0])
            headers[-1] = headers[-1] + ' ' + string.lstrip(lines[i])
        else:
            headers.append(lines[i])
    hdrdict = {}
    for i in range(len(headers)):
        colon = string.find(headers[i], ':')
        if colon == -1:
            raise Error, ("HTTP header without colon", headers[i])
        name = string.lower(headers[i][:colon])
        value = string.lstrip(headers[i][colon+1:])
        if not hdrdict.has_key(name):
            hdrdict[name] = value
        else:
            hdrdict[name] = hdrdict[name] + ',' + value
    return {'firstline': lines[0], 'headers': hdrdict}

def array_begins(haystack, needle):
    return haystack[0:len(needle)] == needle

def handle_http_request(conn, uri, httpreq, query_string):
    urichunks = urlpath(uri)
    urlroot = "/%s" % conn.urlroot()
    if (uri == urlroot) or (string.find(uri, urlroot + "/") == 0):
        handle_urlroot_request(conn, uri[len(urlroot):], httpreq, query_string)
    elif httpreq['method'] == 'GET':
        httpget(conn, urichunks)
    else:
        conn.send(http_header('501 Not Implemented', 'text/html') +
                  "The %s method is not supported for %s.\n" % (cgi.escape(httpreq['method']), cgi.escape(uri)))
        conn.finish_sending()

def html_prologue_string(conn):
    return ('<script type="text/javascript" src="/%s?do_method=whoami"></script>'
            % cgi.escape(conn.urlroot()))

def js_prologue_string(conn):
    str = ''
    if not conn.shouldIgnorePrologue():
        # print "Doing prologue"
        try:
            str = conn.pathread(urlpath(conn.getknroot()) + ['kn_apps', 'kn_lib', 'prologue.js'])[1]
        except: pass
    return ('kn_userid = "anonymous"; kn_displayname = "Anonymous User";\r\n'
            + str + '\r\n' +
            ("if (! window.kn_server) window.kn_server = '/%s';" % conn.urlroot()))

def handle_urlroot_request(conn, uri, httpreq, query_string):
    conn.report_status('handling urlroot request: %s' % uri)
    knroot = urlpath(conn.getknroot())
    query = cgi.parse_qs(query_string, keep_blank_values=1)
    if query.has_key('do_method'):
        do_method = query['do_method'][0]
    elif httpreq['method'] == 'POST':
        do_method = 'notify'
    elif query == {}:
        do_method = None
    else:
        do_method = 'route'

    if do_method == 'route':
        if query.has_key('kn_to') and string.find(query['kn_to'][0], 'javascript:') != 0:
            add_static_route(conn, uri, httpreq, query)
        else:
            tunnel(conn, uri, httpreq, query)
    elif do_method == 'notify':
        notify(conn, uri, httpreq, query)
    elif do_method == 'help' or do_method == 'test':
        do_help(conn)
    elif do_method == 'blank':
        do_blank(conn)
    elif do_method == 'batch':
        do_batch(conn, query)
    elif do_method == 'lib':
        header = http_header('200 OK', 'text/javascript')
        data = (js_prologue_string(conn) +
                conn.pathread(knroot + ['kn_apps', 'kn_lib', 'pubsub.js'])[1])
        conn.send(header + data)
        conn.finish_sending()
    elif do_method == 'libform':
        conn.send(http_header('200 OK', 'text/javascript') +
                  js_prologue_string(conn) +
                  conn.pathread(knroot + ['kn_apps', 'kn_lib', 'pubsub.js'])[1] +
                  "\n" +
                  conn.pathread(knroot + ['kn_apps', 'kn_lib', 'form.js'])[1])
        conn.finish_sending()
    elif do_method == 'lib2form':
        conn.send(http_header('200 OK', 'text/javascript') +
                  js_prologue_string(conn) +
                  conn.pathread(knroot + ['kn_apps', 'kn_lib', 'pubsub.js'])[1] +
                  "\n" +
                  "window.kn__form2way = true;\n" +
                  conn.pathread(knroot + ['kn_apps', 'kn_lib', 'form.js'])[1])
        conn.finish_sending()
    elif do_method == 'whoami':
        conn.send(http_header('200 OK', 'text/javascript') +
                  js_prologue_string(conn))
        conn.finish_sending()
    elif do_method == 'noop':
        conn.send(http_header('200 OK', 'text/plain') +
                  'No operation performed.')
        conn.finish_sending()
    elif do_method is None:
        if uri == '/':
            conn.send(http_header('200 OK', 'text/html') +
                      "<html><head><title>Welcome to pubsub.py</title></head>\n" +
                      '<body bgcolor="white"><h1>Welcome</h1>\n' +
                      'Try ' +
                      '<a href="server-status">Server status.</a>\n</body></html>')
            conn.finish_sending()
        elif uri == '/server-status':
            conn.send(http_header('200 OK', 'text/html') + conn.server.server_status())
            conn.finish_sending()
        elif uri == '/killserver':
            conn.server.kill()
        else:
            conn.send(http_header('404 Not Found', 'text/plain') + "Not found.")
            conn.finish_sending()
    else:
        conn.send(http_header('400 Method Not Supported', 'text/plain') +
                  "Don't understand %s.\n" % do_method)
        conn.finish_sending()

class Connection(asyncore.dispatcher_with_send):
    """Responsibilities: parse incoming HTTP data.  Dispatch requests.
    Report status to server."""
    def __init__(self, sock, server, addr):
        asyncore.dispatcher_with_send.__init__(self, sock)
        self.closed = 0
        self.inbuf = ''
        self.sending_response = 0
        self.done_sending = 0
        self.tickle_renderer = 0
        self.expecting_body_bytes = None
        self.server = server
        self.server.connection_address_is(self, addr)
        self.addr = addr
        self.sockname = sock.getsockname()
        self.report_status('reading HTTP header')
    def getname(self): return 'pubsub.Connection'
    def report_status(self, status):
        """Report present connection status."""
        if not self.closed:
            self.server.report_status(self, status)
    def log_err(self, msg):
        logger.log_err("%x: %s: %s" % (id(self), self.addr, msg))
    def finish_sending(self):
        self.report_status('finished sending')
        self.done_sending = 1
        self.initiate_send()
    def handle_read_event(self):
        try:
            data = self.socket.recv(4096)
            if data == '':
                # print "%s: eof on read" % self
                self.close()
            else:
                self.inbuf = self.inbuf + data
                self.try_to_respond()
        except socket.error, val:
            if val[0] != errno.ECONNRESET:
                print "%s: read error: %s" % (self, val)
            self.close()
    def try_to_respond(self):
        if self.expecting_body_bytes is None:
            self.try_to_parse_header()
        elif self.expecting_body_bytes > 0:
            self.try_to_receive_body()
        elif not self.sending_response:
            self.send_response()
    def try_to_parse_header(self):
        eoh = string.find(self.inbuf, '\r\n\r\n')
        if eoh != -1:
            self.httpreq = parse_http_request_header(self.inbuf[:eoh])
            self.inbuf = self.inbuf[eoh+4:]
            if self.httpreq['method'] in ['GET', 'HEAD']:
                self.expecting_body_bytes = 0
                self.try_to_respond()
            elif self.httpreq['method'] == 'POST':
                hdrs = self.httpreq['headers']
                if hdrs.has_key('content-length'):
                    self.report_status('getting POST body')
                    self.expecting_body_bytes = int(hdrs['content-length'])
                    self.try_to_respond()
            else:
                self.expecting_body_bytes = 0
                self.try_to_respond()
    def try_to_receive_body(self):
        if len(self.inbuf) >= self.expecting_body_bytes:
            self.httpreq['body'] = self.inbuf[0:self.expecting_body_bytes]
            self.inbuf = self.inbuf[self.expecting_body_bytes:]
            self.expecting_body_bytes = 0
            self.report_status('got body')
            self.try_to_respond()
    def send_response(self):
        try:
            self.sending_response = 1
            uri = self.httpreq['uri']
            self.report_status('sending response for %s' % uri)
            if self.httpreq['method'] == 'POST':
                qs = self.httpreq['body']
            else:
                qm = string.find(uri, '?')
                if qm == -1:
                    qs = ''
                else:
                    qs = uri[qm+1:]
                    uri = uri[:qm]
            handle_http_request(self, uri, self.httpreq, qs)
        except:
            self.report_status('sending internal server error response')
            self.send(http_header("500 Internal Server Error", "text/html") +
                      '<html><head><title>500 Internal Server Error</title>' +
                      html_prologue_string(self) +
                      '</head><body bgcolor="#f0f0ff">\n' +
                      cgitb.html())
            self.log_err('send_response\n' + cgitb.html())
            self.finish_sending()
    def initiate_send(self):
        if self.closed: return
        num_sent = asyncore.dispatcher.send(self, self.out_buffer)
        self.out_buffer = self.out_buffer[num_sent:]
        if self.out_buffer == '' and self.done_sending:
            self.close()
    def writable(self):
        return (not self.connected) or len(self.out_buffer) or self.tickle_renderer
    def handle_write(self):
        if self.tickle_renderer and not self.done_sending:
            self.tickle_renderer = 0
            self.send(' ' * 4096)
        else:
            self.initiate_send()
    def send(self, data):
        return asyncore.dispatcher_with_send.send(self, data)
    def close(self):
        if not self.closed:
            self.done_sending = 1
            self.closed = 1
            self.server.connection_closed(self)
            self.out_buffer = ''
            return asyncore.dispatcher_with_send.close(self)
    def log(self, msg):
        pass
    def get_topic(self, uri):
        # Demeter would call me Hades for this:
        # XXX this is really wrong wrt off host routes!  We must fix our URI handling!
        # in places where we're passed just a path, we should not remove self.urlroot;
        # in places where we're passed an entire URI, we should.
        (_, _, path, _, _, _) = urlparse.urlparse(uri)
        urlroot = "/%s/" % self.urlroot()
        # print str(("get_topic 1", path, urlroot))
        if (path == urlroot) or (string.find(path, urlroot)) == 0: path = path[len(urlroot):]
        # print str(("get_topic 2", path, urlroot))
        return self.server.root_topic.get_descendant(filter(lambda x: x != '', string.split(path, '/')))
    def getservername(self):
        if self.httpreq['headers'].has_key('host'):
            return 'http://%s' % self.httpreq['headers']['host']
        else:
            return 'http://%s:%s' % self.sockname
    def repr(self):
        return "<Connection %d>" % id(self)
    def documentroot(self):
        return self.server.documentroot()
    def urlroot(self):
        return "kn"
    def getknroot(self):
        return self.server.getknroot()
    def pathread(self, realpath):
        return pathread(self.documentroot(), realpath)
    def shouldIgnorePrologue(self):
        return self.server.ignorePrologue

def http_header(statusline, contenttype):
    return "HTTP/1.0 %s\r\nContent-Type: %s\r\n\r\n" % (statusline, contenttype)

# How to use HttpClient?
# Well, we need it for two things: outbound routes and content-transform routes.
# For both, we want to follow redirects.

# For both, we care about success or failure; for outbound routes, we
# want to delete the route on failure, and for content-transform
# routes, actually, I think we want that as well.  For
# content-transform routes, we need the response body to be passed to
# the route guy to modify the event with; for outbound routes, we need
# no such thing.

# So something like this for content-transform:
# HttpClient(http_post(url, ['kn_payload', payload]),
#            FinishRouting(event, topic, self))
# where FinishRouting looks like

class FinishRouting:
    def __init__(self, event, topic, route):
        self.event = event
        self.topic = topic
        self.route = route
    def __call__(self, response):
        if response.is_success():
            self.route.deliver(self.event.clone({'kn_payload': response.body()}))
        else:
            self.topic.poison_route(self.route)

# And for external routes, something like
# HttpClient(http_post(url, event),
#            ReportRouteStatus(event, topic, self))
# where ReportRouteStatus looks like

class ReportRouteStatus:
    def __init__(self, topic, route):
        self.topic = topic
        self.route = route
    def __call__(self, response):
        if not response.is_success():
            self.topic.poison_route(self.route)

# Perhaps these could be changed to

# class ReportRouteStatus:
#     def __init__(self, topic, route):
#         self.topic = topic
#         self.route = route
#     def handle_success(self, response): pass
#     def handle_failure(self, response):
#         self.topic.poison_route(self.route)
#     def __call__(self, response):
#         if response.is_success(): self.handle_success(response)
#         else: self.handle_failure(response)
# 
# class FinishRouting(ReportRouteStatus):
#     def __init__(self, event, topic, route):
#         ReportRouteStatus.__init__(self, topic, route)
#         self.event = event
#     def handle_success(self, response):
#         self.route.deliver(self.event.clone({'kn_payload': response.body()}))

# On second thought, that's the same number of lines, but 77 more
# characters, and it involves inheritance, so it's probably better
# to do it the other way.

# So it looks like I want a class for HTTP messages, anyway; I wrote the
# above response-code-checking code originally as if response['code'][0]
# in '123':

class HttpClient(asyncore.dispatcher_with_send):
    def __init__(self, addr, request):
        socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        socket.setblocking(0)
        socket.connect(addr)
        asyncore.dispatcher_with_send.__init__(self, socket)
        self.send(request)
    def readable(self): return 1
    def handle_read_event(self):
        try:
            data = self.socket.recv(4096)
            if data == '': self.handle_eof()
            else: self.handle_data(data)
        except socket.error, val:
            print "Socket error %s" % val

    # FIXME: This should be replaced with a reasonable configuration system!

def urlpath(filename):
    filenamechunks = string.split(filename, '/')
    if len(filename) > 0 and filename[-1] == '/': filenamechunks.append('index.html')
    realpath = []
    for x in filenamechunks:
        if x == '' or x == '.': pass
        elif x == '..':
            if len(realpath) > 0: realpath.pop()
        else:
            realpath.append(x)
    return realpath

def pathopen(documentroot, realpath):
    # XXX raceable
    for i in range(len(realpath)):
        dir = string.join([documentroot] + realpath[0:i], '/')
        # avoid /com1/foo etc.
        if not os.path.exists(dir):
            raise Error404, "Couldn't find %s.\n" % string.join(realpath, '/')
    file = string.join([documentroot] + realpath, '/')
    return (mimetype(file), open(file, 'rb'))

def pathread(documentroot, realpath):
    type, file = pathopen(documentroot, realpath)
    data = file.read()
    file.close()
    return type, data

def endswith(string, *endings):
    def ends(string, ending):
        return len(string) >= len(ending) and string[-len(ending):] == ending
    for ending in endings:
        if ends(string, ending): return 1
    return 0

def mimetype(path):
    if endswith(path, '.html'): return 'text/html; charset=utf-8'
    # FIXME: cgi support does not work
    elif endswith(path, '.cgi'): return 'text/html; charset=utf-8'
    elif endswith(path, '.js'): return 'text/javascript'
    elif endswith(path, '.gif'): return 'image/gif'
    elif endswith(path, '.jpg', '.jpeg'): return 'image/jpeg'
    else: return 'text/plain'

Error404 = '404 no file'

def httpget(conn, pathchunks):
    try:
        (type, data) = conn.pathread(pathchunks)
        conn.send(http_header('200 OK', type) + data)
        conn.finish_sending()
    except (Error404, IOError), msg:
        conn.send(http_header('404 Not Found', 'text/html') + cgi.escape(str(msg)))
        conn.finish_sending()

def read_event_pool(filename):
    if filename is None: return create_topic([])
    try: file = open(filename, "rb")
    except IOError:
        try: file = open(filename + ".old", "rb")
        except IOError: return create_topic([])  # new event pool
    rv = load(file)
    file.close()
    return rv

def write_event_pool(filename, root):
    if filename is None: return
    tmpfile = filename + ".tmp"
    file = open(tmpfile, "wb")
    dump(root, file)
    file.close()
    oldfile = filename + ".old"
    try: os.remove(oldfile)
    except os.error: pass  # it's OK if the old file isn't there
    try: os.rename(filename, oldfile)
    except os.error: pass # it's also OK if the event pool file wasn't there
    os.rename(tmpfile, filename) # it's not OK if this fails, though

def main(argv):
    verbose = 0
    filename = None
    autoPortNum = 0
    ignorePrologue = 0
    optlist, argv[1:] = getopt.getopt(argv[1:], "vhaif:", ["verbose", "help", "auto", "ignore", "file="])
    for opt, val in optlist:
        if opt in ('-v', '--verbose'):
            verbose = verbose + 1
        elif opt in ('-h', '--help'):
            print (
                "%s: Usage: %s [options...] ( portnum | --auto ) docroot [topicroot]\n"
                % (argv[0], argv[0]) + "\n"
                "  -h, --help                 print this message and exit\n"
                "  -v, --verbose              increase verbosity of logging to stderr and stdout\n"
                "  -f, --file                 filename to use as persistent event pool\n"
                "                             (multiple occurrences have a cumulative effect)\n"
                "                             (default is *not* to have a persistent event pool)\n"
                "  -a, --auto                 automatically extract the portnum \n"
                "                             from the {docroot}/kn_apps/kn_lib/prologue.js file;\n"
                "                             in this case the portnum must *not* be supplied\n"
                "  -i, --ignore               ignore the prologue.js file; \n"
                "                             used to run pubsub.py standalone, without being \n"
                "                             affected by the cross domain setup in prologue.js\n"
            )
            return
        elif opt in ('-f', '--file'):
            filename = val
        elif opt in ('-a', '--auto'):
            autoPortNum = 1
        elif opt in ('-i', '--ignore'):
            ignorePrologue = 1

    logfile = open("pubsub.log", "ab")
    errlog = open("pubsub.err.log", "ab")
    if not autoPortNum:
        minRequiredArgs = 3
        maxRequiredArgs = 4
    else:
        minRequiredArgs = 2
        maxRequiredArgs = 3

    if len(argv) < minRequiredArgs or len(argv) > maxRequiredArgs:
        exitWithError(
            "%s: Usage: %s [--help] [options...] ( portnum | --auto ) docroot [topicroot]\n" % (argv[0], argv[0])
        )
    else:

        # get the document root from the command line
        docRoot = argv[minRequiredArgs - 1]
        if not os.access(docRoot, os.F_OK):
            exitWithError(
                "Specified document root: %s is not an accessible directory, exiting...\n" % docRoot
            )

        # get the kn root uri from the command line
        if len(argv) == maxRequiredArgs: topicRoot = argv[maxRequiredArgs-1]
        else: topicRoot = ""

        if not autoPortNum:
            # get port from command line
            portNumStr = argv[1]

        else:
            # attempt to override the port using the one in the prologue.js file
            try:
                prologuePath = os.path.join(docRoot, 'kn_apps', 'kn_lib', 'prologue.js')
                serverAddress = readPubSubServerAddress (prologuePath)
                if serverAddress != None:
                    serverLocation = urlparse.urlparse(serverAddress)[1]
                    # split on the : (if present) otherwise must be on the std port
                    splitServerLocation = serverLocation .split(":")
                    if len(splitServerLocation) == 2:
                        portNumStr = splitServerLocation[1]
                    else:
                        portNumStr = "80"
            except IOError, (errno, strerror):
                exitWithError(
                    "Error accessing %s, (%s): %s\n"
                    "It is required when auto is used." % (prologuePath, errno, strerror)
                )

        try:
            portNum = int(portNumStr)
            if portNum < 1 or portNum > 65535 :
                exitWithError(
                    "Specified port number: %s is not in the valid range 1-65535, exiting...\n" % portNumStr
                )
        except ValueError:
            exitWithError(
                "Specified port number: %s is not a valid number, exiting...\n" % portNumStr
            )

        sch = Scheduler()
        server = Server(portNum, logfile, errlog, sch, docRoot, topicRoot, verbose, filename, ignorePrologue)

        if verbose:
            if ignorePrologue:
                ignorePrologueStr = "true"
            else:
                ignorePrologueStr = "false"

            print(
                "\nPubSub Server initialized\n"
                "    Port: %s\n"
                "    Document root: %s\n"
                "    Topic root: %s\n"
                "    Event pool file: %s\n"
                "    Prologue ignored: %s\n" % (str(portNum), docRoot, topicRoot, filename, ignorePrologueStr))
        else:
            print "\nPubSub Server initialized\n"

        while server.alive:
            asyncore.poll(sch.timeout())
            sch.run()

if __name__ == "__main__": main(sys.argv)

# End of pubsub.py
