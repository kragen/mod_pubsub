#!/usr/bin/python

"""
        scheduler.py -- An asynchronous scheduling mechanism for use
        with the python_pubsub "asyncore" event loop (from
        mod_pubsub).

        Responsibilities: Maintain a list of items to be done at
        specific times in the future; run items to be done at present
        or in the past; tell event loop how long until the next
        scheduled task.

        Version 1.0 -- February 8, 2003.  Initial implementation.
        Works fine on Debian GNU Linux 3.0 with Python 2.1.3.

        Known Issues:

            *. No demo sample provided in this file.
               For now, see sitewatch_sensor.py for a sample usage.

        Contact Information:
            http://mod-pubsub.sf.net/
            mod-pubsub-developer@lists.sourceforge.net
"""

## Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.

## @KNOWNOW_LICENSE_START@

## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:

## 1. Redistributions of source code must retain the above copyright
## notice, this list of conditions and the following disclaimer.

## 2. Redistributions in binary form must reproduce the above copyright
## notice, this list of conditions and the following disclaimer in
## the documentation and/or other materials provided with the
## distribution.

## 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
## be used to endorse or promote any product without prior written
## permission from KnowNow, Inc.

## THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
## WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
## MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
## IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
## DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
## DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
## GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
## IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
## OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
## ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## @KNOWNOW_LICENSE_END@

## $Id: scheduler.py,v 1.1 2003/02/18 08:26:03 ifindkarma Exp $


import time

import asyncore
"""
    Note that we are using the event-driven python_pubsub asyncore,
    not the polling "standard" asyncore. -- Ben and Adam, 2/8/2003
"""

# FIXME: we need to be able to cancel timers we've scheduled previously

class Scheduler:
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
                i()
            else:
                self.schedule.append(i)
    def timeout(self):
        if self.schedule == []:
            return None
        when = self.schedule[0].when
        for i in self.schedule:
            if when > i.when: when = i.when
        diff = when - time.time()
        if diff < 0:
            return 0
        else:
            return diff

# construct a singleton scheduler corresponding to the singleton
# asyncore event loop
scheduler = Scheduler()

def schedule_processing(*args, **kw):
    return scheduler.schedule_processing(*args, **kw)

def timeout(*args, **kw):
    return scheduler.timeout(*args, **kw)

def run(*args, **kw):
    return scheduler.run(*args, **kw)

def main(argv):
    return # FIXME: Add a demo sample usage.

if __name__ == "__main__": main(sys.argv)

# End of scheduler.py
