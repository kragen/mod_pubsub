# Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.

# @KNOWNOW_LICENSE_START@

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:

# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.

# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.

# 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
# be used to endorse or promote any product without prior written
# permission from KnowNow, Inc.

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

# @KNOWNOW_LICENSE_END@

# @(#) $Id: Makefile,v 1.2 2003/04/25 02:37:44 bsittler Exp $

PLAT = $(shell uname -a)
evloop=evloop_select.o evloop_common.o

# For compiling on Solaris:
# Solaris doesn't have socket stuff in libc:

ifneq (,$(findstring SunOS, $(PLAT)))
LDLIBS=-lsocket -lnsl -lm
CFLAGS=-O -DFD_SETSIZE=20480
evloop=evloop_soldevpoll.o evloop_common.o

# For compiling on Linux:
else
CFLAGS=-g -Wall -Werror
LDLIBS+=-lm
endif

# For profiling with gprof on Solaris:
# CC=gcc 
# CFLAGS=-pg -DFD_SETSIZE=10240 
# LDFLAGS=-lsocket -lnsl -ldl -pg

# For debugging:
# LDLIBS += -lefence 

CPPFLAGS=-Imy_getopt -I.
VERSION=devbuild
TARDIR=perftool-$(VERSION)

# which evloop implementation to use

all: nc perftool analyze lineartest test-parser evltest

nc: nc.o dstring.o nonblock.o pipefit.o openconn.o \
	 doubletime.o $(evloop)

perftool: perftool.o masterparms.o dstring.o \
	my_getopt/my_getopt.o master.o openconn.o connqueue.o \
	nonblock.o pipefit.o dstr_interp.o parser.o evrpt.o doubletime.o \
	post_event.o $(evloop) urlencode.o

analyze: analyze.o linear.o analyze_conn.o dstring.o

lineartest: lineartest.o linear.o

test-parser: test-parser.o parser.o dstring.o

evltest: evltest.o $(evloop) dstring.o

clean: 
	rm -f my_getopt/*.o *.o nc perftool analyze lineartest test-parser evltest

depend:
	makedepend *.c my_getopt/*.c

# very minimal regression tests
test: lineartest test-parser evltest
	./lineartest
	./test-parser
	./evltest

tar: perftool analyze test
	rm -rf $(TARDIR) $(TARDIR).tar $(TARDIR).tar.gz
	mkdir $(TARDIR)
	cp perftool analyze perftool.html analyze.html $(TARDIR)/.
	tar cvf $(TARDIR).tar $(TARDIR)
	rm -rf $(TARDIR)
	gzip -9 $(TARDIR).tar

.PHONY: depend clean all test tar
