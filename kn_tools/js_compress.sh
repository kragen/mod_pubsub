#!/bin/bash --

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
# $Id: js_compress.sh,v 1.2 2003/03/19 05:34:39 ifindkarma Exp $

## JAVASCRIPT COMPRESSION FILTER
# This heuristic filter automatically compresses a JavaScript file
# (such as pubsub.js) which uses kn._xxx or _kn_xxx for internal
# property names, and _l_foo for local variable and parameter names.
#
# It inserts a copyright notice at the beginning of the output.
#
# It adds a blank line at the end of the output.

## BUGS
# It can't handle RegExp constants very well; that's what the
# ___quoted_slash___ stuff is about. It doesn't handle C-style /*
# ... */ multiline comments, so don't use them!

# Notes on notation:
# 1. local variable and parameter names with four or more
#    characters prefixed by _l_ are compressed; at least one
#    reference to each such variable should appear on a line with no
#    other compressed variables
# 2. microserver-internal variables with names like _kn_xxx and kn._xxx
#    are compressed; at least one reference to each such variable
#    should appear on a line with no other compressed variables
# 3. "internal-but-visible" variables with names like kn__xxx are not
#    compressed
# 4. a //_ comment marks a point where a newline has been inserted
#    to guarantee that a particular compressed variable appears
#    alone on at least one line
# 5. a //# comment at the end of a line marks a line which will
#    be removed from the "production" version of the microserver
# 6. _kn_$("x") wraps a literal string "x" in the "kn" localization context;
#    at most one wrapper can appear on a particular line
# 7. _kn_$_("x", ...) wraps a literal formatting template "x" in the "kn"
#    localization context; at most one wrapper can appear on a
#    particular line

# We need GNU tools, not old broken UNIX tools (sed, in particular.)
PATH=/usr/local/bin:/usr/bin:/bin:"$PATH"

# We store intermediate results in temporary files:
tmp="${TMPDIR:-/tmp}/$(basename "$0")$$"
tmp2="${tmp}.short"
tmp3="${tmp}.sed"

# Clean up temporary files:
trap '/bin/rm -f -- "$tmp" "$tmp2" "$tmp3"' EXIT

cat > "$tmp" &&
(
cat <<.
// Copyright 2000-2002 KnowNow, Inc.  All Rights Reserved.
.
expand < "$tmp" |
    # strip comments, blank lines, and leading and trailing whitespace
    # also strip spaces around simple assignment operations
    # and handle a very special kn_localize case
    sed -ne '
        s/\\\//___quoted_slash___/g;
        s/^\(\(\/\?\([^'"'"'"/]\|'"'"'\([^\\'"'"']\|\\.\)*'"'"''\
'\|"\([^\\"]\|\\.\)*"\)\)*\)\/\/.*$/\1/;
        s/^ \+//;
        s/ \+$//;
        /^$/b;
        s/^\/\*[^*]*\*\/ *//;
        s/^\(kn_localize("\([^"\\]\|\\.\)*"\),"kn"/_\1/;
        s/___quoted_slash___/\\\//g;
        s/^\(\([A-Za-z_$][A-Za-z0-9_$][ .]\+\)*[A-Za-z_$][A-Za-z0-9_$]*\) *\([={};()+?:]\|\[\|\]\) *\([A-Za-z0-9_$'"'"'"]\)/\1\3\4/;
        p;
    ' |
    # collapse lots of whitespace
    perl -pe '
        $/="";
        $q="'"'"'";
        s/" \+\n"//g;
        s/$q \+\n$q//g;
        s/\n(["$q])/$1/g;
        s/(["$q])\n/$1/g;
        s/(\n[^\/$q"]*[A-Za-z0-9_\$()]+) *([^A-Za-z0-9_\$])/\1\2/g;
        s/\n *([{}]+|if|while|for|;)[ \n]/$1/g;
        s/\n* *([{}();,]) *\n+/$1/g;
    ' > "$tmp2" &&
sed -ne '
        s/.*\(^\|[^A-Za-z0-9_$]\)\(\(kn\._\|_kn_\|_l_\)[A-Za-z0-9_$]*\).*/\2/p;
        ' < "$tmp" |
        sort |
        uniq |
        nl |
        expand |
        sed -ne '
           s/\./\\./g;
           h;
           s/^ *\([^ ]\+\) \+\([^_]*_\)\(.*\)/s\/\\(^\\|[^A-Za-z0-9_$]\\)'\
'\\(\2\3\\)\\([^A-Za-z0-9_$]\\|$\\)\/'\
'\\1 ___internal_id___ \\2 ___internal_id___ \\3\/g/gp;
           g;
           s/^ *\([^ ]\+\) \+\([^_]*_\)\(.*\)/s\/\\(^\\|[^A-Za-z0-9_$]\\)'\
'\2\3\\([^A-Za-z0-9_$]\\|$\\)\/\\1\2\1\\2\/g/gp;
       ' > "$tmp3" &&
sed -f "$tmp3" -e "
    # clean up the ___internal_id___ fluff we added to space out identifiers
    s/ \?___internal_id___ \?//g;
" < "$tmp2"
cat <<.

.
)

# End of js_compress.sh
