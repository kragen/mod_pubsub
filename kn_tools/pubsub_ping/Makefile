# pubsub_ping - send events to a PubSub Server

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

# $Id: Makefile,v 1.2 2004/04/19 05:39:15 bsittler Exp $

##
## NOTE: This Makefile requires GNU Make, which is sometimes called
##       'gmake', and which may be installed in /usr/local/bin.
##

# pubsub_ping version
VERSION=devel

## Directories for build
ROOT=.

# source directories
SRCROOT=$(ROOT)
SRCDIR=$(SRCROOT)/src
INCLUDEDIR=$(SRCROOT)/include
MY_GETOPTDIR=$(SRCROOT)/my_getopt

# destination directories
DSTROOT=$(ROOT)
BINDIR=$(DSTROOT)/bin
OBJDIR=$(DSTROOT)/obj

## Defaults (not platform-specific)

# use_gcc -- use GNU C Compiler
use_gcc=1

# use_old_cc -- use old cc
use_old_cc=0

# use_ssl -- use OpenSSL for libkn HTTPS support
use_ssl=0

# LIBC -- extra libraries with missing libc pieces
LIBC=

# LIBKNROOT -- libkn installation directory
# To have this, do "sudo make install" in mod_pubsub/c_pubsub/libkn/
LIBKNROOT=/usr/local/kn/libkn

## Known platforms (may override defaults)
PLAT=$(shell uname -a)

# Solaris
ifneq (,$(findstring SunOS,$(PLAT)))
    use_gcc=0
    use_old_cc=1
    use_ssl=1
    LIBC=-lnsl -lsocket
endif

## OPENSSLROOT -- OpenSSL installation directory
OPENSSLROOT=/usr/local/ssl

## C preprocessing flags
CPPFLAGS=-I$(LIBKNROOT)/include
CPPFLAGS+=-DVERSION=\"$(VERSION)\"

## additional include directories for libkn ssl support
ifneq (0,$(use_ssl))
    CPPFLAGS+= -I$(OPENSSLROOT)/include
endif

## C compiler (no explicit arguments required)

# GNU C compiler
ifneq (0,$(use_gcc))
    CC=gcc
    CFLAGS=-O3 -Wall -Werror
    COMPILE=$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<
endif

# old cc
ifneq (0,$(use_old_cc))
    CC=cc
    CFLAGS=-D__FUNCTION__=__FILE__\":\ unknown\ function\"
    COMPILE=$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<
endif

## linker
LD=$(CC)
LDFLAGS=-L$(LIBKNROOT)/lib
LINK=$(LD) $(LDFLAGS) -o $@

## additional library directories for libkn ssl support
ifneq (0,$(use_ssl))
    LDFLAGS+= -L$(OPENSSLROOT)/lib
endif

## attempt to remove several files, some of which may not exist
RM=rm
RMFLAGS=-f
REMOVE=$(RM) $(RMFLAGS)

## attempt to create a directory, which may already exist
MKDIR=mkdir
CREATEDIR=[ -d $@ ] || $(MKDIR) $@

## create a file or update its modification time
TOUCH = touch $@

## libraries for pubsub_ping
LIBS=-lm
LIBKN=-lkn
LIBS+= $(LIBKN)

## additional libraries for libkn ssl support
ifneq (0,$(use_ssl))
    LIBS+= -lssl -lcrypto
endif

## extra libraries with missing libc pieces
LIBS+= $(LIBC)

## object files for pubsub_ping
OBJS=$(OBJDIR)/main.o $(OBJDIR)/my_getopt.o $(OBJDIR)/pubsub_ping.o

all: $(BINDIR)/pubsub_ping

.PHONY: all clean $(BINDIR) $(OBJDIR)

$(BINDIR) $(OBJDIR):
	$(CREATEDIR)

$(BINDIR)/bin.dir:
	$(MAKE) $(MAKEFLAGS) $(BINDIR)
	$(TOUCH)

$(OBJDIR)/obj.dir:
	$(MAKE) $(MAKEFLAGS) $(OBJDIR)
	$(TOUCH)

$(BINDIR)/pubsub_ping: $(OBJS) $(BINDIR)/bin.dir
	$(LINK) $(OBJS) $(LIBS)

clean:
	$(REMOVE) $(BINDIR)/pubsub_ping $(OBJDIR)/*.o $(OBJDIR)/obj.dir $(BINDIR)/bin.dir

$(OBJDIR)/main.o: $(SRCDIR)/main.c $(OBJDIR)/obj.dir $(INCLUDEDIR)/pubsub_ping.h $(MY_GETOPTDIR)/getopt.h $(MY_GETOPTDIR)/my_getopt.h
	$(COMPILE)

$(OBJDIR)/pubsub_ping.o: $(SRCDIR)/pubsub_ping.c $(OBJDIR)/obj.dir $(INCLUDEDIR)/pubsub_ping.h $(INCLUDEDIR)/_pubsub_ping.h
	$(COMPILE)

$(OBJDIR)/my_getopt.o: $(MY_GETOPTDIR)/my_getopt.c $(OBJDIR)/obj.dir $(MY_GETOPTDIR)/my_getopt.h
	$(COMPILE)

# End of Makefile
