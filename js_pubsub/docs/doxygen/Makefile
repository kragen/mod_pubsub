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

# $Id: Makefile,v 1.1 2003/07/20 07:09:16 ifindkarma Exp $

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

# NOTE: Additional .js-based dependencies for this target are
# assembled automagically by the action associated with the "docs"
# target.
doxygen-output: Doxyfile mainpage.js
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
