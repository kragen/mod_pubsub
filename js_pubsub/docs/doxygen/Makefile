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

# $Id: Makefile,v 1.4 2004/04/19 05:39:10 bsittler Exp $

.PHONY: all update-cache update-cache-rules clean docs

all: docs

clean:
	rm -rf js html latex cache-rules doxygen-output

update-cache:
	@[ -d js ] || mkdir js
	@find js -name '*.js' | \
		sed 's|\(.*\)\.js|[ -f ../../"\1".js ] \|\| ( rm -f doxygen-output; echo "docs cache: removing cached file \1.js."; rm -f "&" )|' | \
		bash
	@find js -type d | \
		sed 's|.*|[ -d ../../"&" ] \|\| ( rm -f doxygen-output; rmdir "&" )|' | \
		bash
	@find ../../js -type d | \
		sed 's|\.\./\.\./\(.*\)|[ -d "\1" ] \|\| ( rm -f doxygen-output; mkdir "\1" )|' | \
		bash
	@find ../../js -name connectors -a -prune -o -name '*.js' -a -print | \
		sed 's|^\.\./\.\./\(.*\)\.js$$|\1.js|' | \
		xargs $(MAKE) -s
	@$(MAKE) -s js/connectors/kn_raw.js

# NOTE: Additional .js-based dependencies for this target are assembled
# automagically by the action associated with the "docs" target.

doxygen-output: Doxyfile mainpage.js
	: NOTE: You must have doxygen installed or make will fail.
	doxygen
	@touch doxygen-output

-include cache-rules

update-cache-rules: update-cache
	@rm -f cache-rules
	@echo -n "doxygen-output: " > cache-rules
	@find js -name '*.js' | \
		sed 's|.*|& \\|' >> cache-rules
	@echo "" >> cache-rules

docs: update-cache-rules
	@$(MAKE) doxygen-output

js/%.js: ../../js/%.js js2doxy js2doxy.pl
	@rm -f doxygen-output
	@echo "docs cache: updating cached file $@."
	@bash js2doxy < "$<" > "$@" || ( rm -f "$@"; exit 1 )
