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

# $Id: Makefile,v 1.6 2004/04/19 05:39:08 bsittler Exp $

CP=cp
SUBDIRS=js_pubsub

all:	kn_apps/kn_lib/pubsub.js kn_events
	$(CP) kn_apps/kn_lib/pubsub_raw.js js_pubsub/js/connectors/kn_raw.js
	$(CP) kn_tools/js_compress.sh js_pubsub/js/tools/kn_compress.sh
	$(CP) kn_tools/kn_localize.sh js_pubsub/js/tools/kn_localize.sh
	$(MAKE) -C js_pubsub "$@"

clean:
	@for x in $(SUBDIRS); do $(MAKE) -C "$$x" "$@" || exit 1; done
	rm -rf kn_events
	$(RM) kn_apps/kn_lib/pubsub.js
	$(RM) js_pubsub/js/connectors/kn_raw.js
	$(RM) js_pubsub/js/tools/kn_compress.sh
	$(RM) js_pubsub/js/tools/kn_localize.sh

.PHONY:	all clean kn_events

# Compress pubsub.js so it is much less bytes over the wire.

kn_apps/kn_lib/pubsub.js:	kn_apps/kn_lib/pubsub_raw.js kn_tools/js_compress.sh
	bash kn_tools/js_compress.sh < kn_apps/kn_lib/pubsub_raw.js > kn_apps/kn_lib/pubsub.js

# Make sure mod_pubsub can write to the kn_events directory ---
# otherwise it will not run.

# Note that you only need the kn_events directory if you are using
# the Perl PubSub Server.  (The Python PubSub Server runs in memory.)

HTTPD_USER=$(shell whoami)

kn_events:
	[ x"$(HTTPD_USER)" != x ]  # Define HTTPD_USER as the user that runs your web server, e.g. make HTTPD_USER=httpd (or nobody, or www-data)
	if ! [ -d "$@" ]; \
	then mkdir $@; \
	    if ! chown $(HTTPD_USER) $@; then rmdir $@; false; fi; \
	fi

# End of Makefile
