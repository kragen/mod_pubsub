#! /bin/bash --

# Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.
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
# $Id: kn_localize.sh,v 1.2 2003/03/19 05:34:39 ifindkarma Exp $

# Build a default English localization table for a localizable
# JavaScript file; it understands two styles of localizable string:
# simple strings:
## _kn_$("string") for the "kn" context,
## foo$("string") for the "foo" context,
## $("string") for the empty context, and
## $$("context", "string") for the more general form.
# formatted strings:
## _kn_$_("in the %{1} window I saw %{0}",
##        _kn_$("oranges"),
##        window.name) for the "kn" context,
## foo$_("in the %{1} window I saw %{0}",
##       foo$("oranges"),
##       window.name) for the "foo" context,
## $_("in the %{1} window I saw %{0}",
##        $("oranges"),
##        window.name) for the empty context, and
## $$_("context", "in the %{1} window I saw %{0}",
##     $$("context", "oranges"),
##     window.name) for the more general form.
# There can be at most one such string per line, and no string
# continuation or operators may be used. The '$'-operator and the string
# to be localized must appear on the same line. Double quotes may be used
# instead of single quotes.

cat <<.
// This localization template was automatically generated using
// kn_localize.sh.  The numbers inside /* comments */ are line numbers
// in the original file.  Change "en" to the standard code for your
// target language, and replace the nulls with strings in the target
// language.
//
// Generated: `date`
.
nl -ba -nln |
sed -ne '
    # _kn_$("s") form -> "kn" context
    s/^\([0-9]\+\).*[^A-Za-z0-9_$]_kn_\$_\?(\(\("\([^\\"]\|\\.\)*"\|'"'"'\([^\\'"'"']\|\\.\)*'"'"'\)\).*/\/* \1 *\/ kn_localize("en","kn",\
\2,\
null);/p
    # $$("foo", "s") form -> arbitrary "foo" context
    s/^\([0-9]\+\).*[^A-Za-z0-9_$]\$\$_\?(\(\("\([^\\"]\|\\.\)*"\|'"'"'\([^\\'"'"']\|\\.\)*'"'"'\)\),[ 	]*\(\("\([^\\"]\|\\.\)*"\|'"'"'\([^\\'"'"']\|\\.\)*'"'"'\)\).*/\/* \1 *\/ kn_localize("en",\2,\
\6,\
null);/p
    # foo$("s") form -> arbitrary "foo" context
    s/^\([0-9]\+\).*[^A-Za-z0-9_$]\([A-Za-z_][A-Za-z0-9_]*\)\?\$_\?(\(\("\([^\\"]\|\\.\)*"\|'"'"'\([^\\'"'"']\|\\.\)*'"'"'\)\).*/\/* \1 *\/ kn_localize("en","\2",\
\3,\
null);/p
    '

# End of kn_localize.sh
