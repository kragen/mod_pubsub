# Copyright 2000-2002 KnowNow, Inc.  All Rights Reserved.
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
# $Id: Makefile,v 1.2 2003/01/29 05:28:03 kragen Exp $

all:	kn_apps/kn_lib/pubsub.js kn_events

.PHONY:	all

# Compress pubsub.js so it's much less bytes over the wire.

kn_apps/kn_lib/pubsub.js:	kn_apps/kn_lib/pubsub_raw.js kn_tools/js_compress.sh
	bash kn_tools/js_compress.sh < kn_apps/kn_lib/pubsub_raw.js > kn_apps/kn_lib/pubsub.js

# Make sure mod_pubsub can write to the kn_events directory --- otherwise it
# won't run.

kn_events:
	[ x"$(HTTPD_USER)" != x ]  # define HTTPD_USER as the user that runs your web server, e.g. make HTTPD_USER=httpd (or nobody, or www-data)
	mkdir $@
	if ! chown $(HTTPD_USER) $@; then rmdir $@; false; fi

# End of Makefile
