# Copyright 2000-2004 KnowNow, Inc.  All rights reserved.

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
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 
# 3. Neither the name of the KnowNow, Inc., nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# @KNOWNOW_LICENSE_END@
# 

# @(#) $Id: Makefile,v 1.4 2004/04/19 05:39:14 bsittler Exp $

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
	post_event.o $(evloop) urlencode.o nan.o

analyze: analyze.o linear.o analyze_conn.o dstring.o nan.o

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
