#!/usr/bin/python

"""
    scheduler.py -- An asynchronous scheduling mechanism
    for use with the python_pubsub "asyncore" event loo
    (from mod_pubsub).

    Responsibilities: Maintain a list of items to be done at
    specific times in the future; run items to be done at present
    or in the past; tell event loop how long until the next
    scheduled task.

    $Id: scheduler.py,v 1.7 2004/04/19 05:39:15 bsittler Exp $

    Tested with Python 1.5.2 and Python 2.1.3,
    on Red Hat and Debian Linux.

    Known Issues: see the FIXME's in this file.

    Contact Information:
        http://mod-pubsub.sf.net/
        mod-pubsub-developer@lists.sourceforge.net
"""

## Copyright 2000-2004 KnowNow, Inc.  All rights reserved.

## @KNOWNOW_LICENSE_START@
## 
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
## 
## 1. Redistributions of source code must retain the above copyright
## notice, this list of conditions and the following disclaimer.
## 
## 2. Redistributions in binary form must reproduce the above copyright
## notice, this list of conditions and the following disclaimer in the
## documentation and/or other materials provided with the distribution.
## 
## 3. Neither the name of the KnowNow, Inc., nor the names of its
## contributors may be used to endorse or promote products derived from
## this software without specific prior written permission.
## 
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
## "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
## LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
## A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
## OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
## LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
## 
## @KNOWNOW_LICENSE_END@
## 

## $Id: scheduler.py,v 1.7 2004/04/19 05:39:15 bsittler Exp $


import time

import asyncore
"""
    Note that we are using the event-driven python_pubsub asyncore,
    not the polling "standard" asyncore that ships with Python.
"""

# FIXME: We need to be able to cancel timers we've scheduled previously.

class Scheduler:
    """
    Responsibilities:
        1. Maintain a list of items to be done at
           specific times in the future.
        2. Run items to be done at present or in the past.
        3. Tell event loop how long until the next scheduled task.
    """

    class Task:
        def __init__(self, func, when, name):
            self.func = func
            self.when = when
            self.name = name
        def __call__(self):
            self.func()
        def __repr__(self):
            return ("Task(%s, %s, %s)" %
                    (repr(self.func), repr(self.when), repr(self.name)))

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
                try:
                    i()
                except:
                    # FIXME: We could really use an error handler...
                    # but even this is better than crashing the event loop.
                    # Example:
                    # logger.log_err("Error in scheduled task " +
                    #                i.name + "\n" + cgitb.html())
                    self.handle_error(i)
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
    def handle_error (self, i):
        (file,fun,line), t, v, tbinfo = asyncore.compact_traceback()
        # Sometimes a user repr method will crash.
        try:
            i_repr = repr(i)
        except:
            i_repr = '<__repr__(i) failed for object at %0x>' % id(i)
        print (
            'uncaptured python exception in scheduled item %s (%s:%s %s)' % (
                i_repr,
                t,
                v,
                tbinfo
                )
            )


# Construct a singleton scheduler corresponding to
# the singleton asyncore event loop.

scheduler = Scheduler()

# In Python 1.5 and earlier, you'd use the apply() built-in function:
# apply(f, args, kw) calls the function f() with the argument tuple
# args and the keyword arguments in the dictionary kw. apply() is the
# same in 2.0+, but thanks to a patch from Greg Ewing, f(*args, **kw)
# as a shorter and clearer way to achieve the same effect.
#    -- section 9.1 of http://www.amk.ca/python/2.0/index.html

# So, we use the apply() function to stay compatible with Python 1.5.

def schedule_processing(*args, **kw):
    return apply(scheduler.schedule_processing, args, kw)

def timeout(*args, **kw):
    return apply(scheduler.timeout, args, kw)

def run(*args, **kw):
    return apply(scheduler.run, args, kw)


def main(argv):
    return
    # FIXME: Add a demo sample usage.  For now,
    # see mod_pubsub/kn_apps/sitewatch/sitewatch_sensor.py for sample usage.

if __name__ == "__main__": main(sys.argv)

# End of scheduler.py
