#!/bin/bash --

# Copyright 2003 KnowNow, Inc.  All Rights Reserved.

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

# $Id: session2html.bash,v 1.2 2003/05/28 01:30:45 bsittler Exp $

if [ $# = 0 ]
then
    set session-*
fi
status=0
echo '<title>session summaries</title>'
echo '<table border="border">'
echo '<tr><th rowspan="3" valign="bottom">session</th><th colspan="7">latency test</th><th colspan="10">throughput test, second run</th></tr>'
echo '<tr><th colspan="3">events</th><th colspan="4">round trip</th><th colspan="5">subscriber</th><th colspan="5">publisher</th></tr>'
echo '<tr><th>transmitted</th><th>received</th><th>loss</th><th>min (ms)</th><th>avg (ms)</th><th>max (ms)</th><th>std. dev. (ms)</th><th>min elapsed (s)</th><th>avg elapsed (s)</th><th>max elapsed (s)</th><th>total events</th><th>avg throughput (Hz)</th><th>min elapsed (s)</th><th>avg elapsed (s)</th><th>max elapsed (s)</th><th>total events</th><th>avg throughput (Hz)</th></tr>'
for session in "$@"
do
 [ -f "$session"/latency* -a -f "$session"/Res*second.ini/*.ini ] ||
 status=$?
 echo "<tr><th align="left">$session</th>"
 echo '<!-- latency -->'
 tail -5 "$session"/latency* |
 sed -ne 's/\(.*\) events transmitted, \(.*\) events received, \(.*%\) event loss/<td><!-- events transmitted -->\1<\/td><td><!-- events received -->\2<\/td><td><!-- event loss -->\3<\/td>/p
s|round-trip min/avg/max = \(.*\)/\(.*\)/\(.*\) ms, std. dev. =\(.*\) ms|<td><!-- round-trip min (ms) -->\1</td><td><!-- round-trip avg (ms) -->\2</td><td><!-- round-trip max (ms) -->\3</td><td><!-- std. dev. (ms) -->\4</td>|p'
 echo '<!-- second throughput run, sub then pub -->'
 nl "$session"/Res*second.ini/*.ini | sed -ne 's/.*|Min Elapsed:\(.*\)|Max Elapsed:\(.*\)|Total Events:\(.*\)|Average Elapsed:\(.*\)|Average throughput:\(.*\)/<td><!-- min elapsed (s) -->\1<\/td><td><!-- avg elapsed (s) -->\4<\/td><td><!-- max elapsed (s) -->\2<\/td><td><!-- total events -->\3<\/td><td><!-- avg throughput (Hz) -->\5<\/td>/p'
 echo '</tr>'
done
echo '</table>'
exit "$status"
