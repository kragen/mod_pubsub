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

# $Id: Makefile,v 1.2 2004/04/19 05:39:13 bsittler Exp $

srcdir = .

VERSION = 1.5.8
DSTROOT := $(shell pwd)/pocketnoc
PKG_DSTROOT = pocketnoc
TAR = /usr/local/bin/tar cfz
CPDIR = cp -Rf
MKDR = mkdir -p 

ifneq "$(findstring CYGWIN,$(shell uname -s))" ""
	CYGWIN = YES
else
	CYGWIN = NO
endif

.PHONY: install clean tarball nocd

nocd:
ifeq "$(CYGWIN)" "YES"
	cd windows && make clean && make
else 
	cd solaris && VERSION="$(VERSION)" make all
endif

install: clean nocd
ifeq "$(CYGWIN)" "NO"
	$(MKDR) $(DSTROOT)/sensor
	$(MKDR) $(DSTROOT)/source
	$(MKDR) $(DSTROOT)/html_client/pocketnoc
	install -m 444 LICENSE $(DSTROOT)
	install -m 666 html/index.html $(DSTROOT)/html_client/pocketnoc/
	install -m 666 solaris/* $(DSTROOT)/source/
	install -m 555 solaris/nocd $(DSTROOT)/sensor/pocketnoc
	rm -f $(DSTROOT)/source/nocd
endif

tarball: install
ifeq "$(CYGWIN)" "NO" 
	#mv $(DSTROOT)/sensor/nocd $(DSTROOT)/sensor/pocketnoc
	$(TAR) pocketnoc-$(VERSION).tar.gz $(PKG_DSTROOT)
else
	wise32 /c /d VERSION="$(VERSION)" PocketNOC_Install.wse
	install -m 555 PocketNOC_Install.exe PocketNOC-$(VERSION).exe
endif

clean:
	rm -rf $(DSTROOT)
	rm -rf *tar*

# End of Makefile
