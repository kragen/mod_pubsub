<?php
header("Content-Type: text/html; charset=utf-8");

?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
    <html>
    <head>
    <?php

require('pubsublib.php');

if (kn__gpc("show_source", false, array("_GET")))
{
    ?><title><?php echo kn_htmlEscape(__FILE__); ?></title>
        <body text="black" bgcolor="white"><?php
    show_source(__FILE__);
    exit;
}

# subscribe.php - pubsub subscriber for php

# Copyright 2003 KnowNow, Inc.  All Rights Reserved.
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

include('pubsublib.php');
include('eventloop.php');
include('pipefitting.php');

#
# FIXME: This doesn't actually work yet!!!
#

$ctx = array();

$ctx['topic'] = "/who/anonymous/s/" .
    uniqid("") .
    "/kn_journal";
$ctx['host'] = "127.0.0.1";
$ctx['port'] = "80";
$ctx['req'] =
    "GET /kn" .
    $ctx['topic'] .
    "?kn_response_format=simple HTTP/1.0\r\n\r\n";

$ctx['url'] =
    "http://" . $ctx['host'] . ":" . $ctx['port'] . "/kn" . $ctx['topic'];

$ctx['maxdelay'] = "60";

?><h1>subscriber for php</h1>
<p>Watching <code><a href="publish.php?kn_topic=<?php
    print kn_htmlEscape(rawurlencode($ctx['topic']));
?>" target="_blank"><?php
    print kn_htmlEscape($ctx['topic']);
?></a></code> on port <code><?php
    print kn_htmlEscape($ctx['port']);
?></code> for <strong><?php
    print $ctx['maxdelay'];
?></strong> seconds.</p><?php

$ctx_volatile = array();
# detect PHP3 vs. PHP4 syntax
if (5 == true)
{
    eval('$ctx["volatile"] = & $ctx_volatile;');
}
else
{
    $ctx["volatile"] = $ctx_volatile;
}

$ctx['volatile']['connected'] =
    false;

print "<title>" . $ctx['host'] . " " . $ctx['port'] . "</title>\n";

?>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
</head>
<body>
<?php


print "<pre><b>";

$ctx['evl'] =
    new EventLoop();
$ctx['evl']->init();

$ctx['start'] =
    $ctx['evl']->getTime();

$ctx['evl']->setTimeout('
                            if ($args["volatile"]["connected"])
                            {
                                print "\nTimed out waiting for response to finish.\n";
                            }
                            else
                            {
                                print "Timed out waiting for connection.\n";
                            }
                            $args["evl"]->clearRunning();
                        ',
                        $ctx['maxdelay'],
                        $ctx);

function onConnect($p, $ctx)
{
    $ctx['volatile']['connected'] = true;
    print kn_htmlEscape("Connected to " .
                        $ctx['host']) .
                        ".\n";
}

function onEOF($p, $ctx)
{
    print "Connection closed by foreign host.\n";
    $ctx['volatile']['connected'] = false;
    $ctx['evl']->clearRunning();
}

function onError($p, $why, $connected, $ctx)
{
    $ctx['volatile']['connected'] = $connected;
    if (! $ctx['volatile']['connected'])
    {
        print "subscribe: Unable to connect to remote host: ";
    }
    print kn_htmlEscape(socket_strerror(socket_last_error($ctx['sock']))) . "\n";
}

function onData($p, $buf, $ctx)
{
    print '</b>' .
        join("\n",
             split('&#10;',
                   kn_htmlEscape(join("\n",
                                      split("\r",
                                            join("\r",
                                                 split("\n\r",
                                                       join("\n\r",
                                                            split("\r\n",
                                                                  $buf))))))))) .
                                                       '<b>';
    flush();
}

function onClose($p, $ctx)
{
    if ($ctx['volatile']['connected']) print "Connection closed.\n";
    $ctx['volatile']['connected'] = false;
    $ctx['evl']->clearRunning();
}

$addrl = gethostbynamel($ctx['host']);
if ($addrl)
{
    $addr = $addrl[0];
    $reverse = @gethostbyaddr($addr);
    if (! kn_isEqualTo($reverse, false))
    {
        $ctx['host'] = $reverse;
    }
}
else
{
    $addr = @gethostbyaddr($ctx['host']);
}
$port = @getservbyname($ctx['port'], "tcp");
if (kn_isEqualTo($port, false))
{
    $rv = sscanf($ctx['port'], "%d%c", $port, $extra);
    if ($rv != 1) $port = 0;
}
if (! $addr)
{
    print kn_htmlEscape($ctx['host']) .
        ": No address associated with name\n";
}
else if (! ($port > 0 and $port <= 0xffff))
{
    print kn_htmlEscape($ctx['port']) .
        ": bad port number\n";
}
else
{
    print "Trying " . kn_htmlEscape($addr) . "...\n";
    $ctx['sock'] = socket_create(AF_INET, # only IPv4 for now...
                                 SOCK_STREAM,
                                 (defined("SOL_TCP") ? SOL_TCP : 0));
    if ($ctx['sock'])
    {
        if (socket_set_nonblock($ctx['sock']))
        {
            $rv = @socket_connect($ctx['sock'], $addr, $port);
            if ($rv or
                socket_last_error($ctx['sock']) == SOCKET_EAGAIN or
                socket_last_error($ctx['sock']) == SOCKET_EWOULDBLOCK or
                socket_last_error($ctx['sock']) == SOCKET_EINPROGRESS)
            {
                $p = new PipeFitting();
                $p->init($ctx['evl'], $ctx['sock'], $ctx['sock']);
                $p->setHandlers(array('onConnect' => 'onConnect',
                                      'onEOF' => 'onEOF',
                                      'onError' => 'onError',
                                      'onData' => 'onData',
                                      'onClose' => 'onClose'),
                                array($ctx));
                $p->write($ctx['req']);
                $p->finish_writing();
                $ctx['evl']->run();
                $p->destroy();
            }
            else
            {
                trigger_error("connect failed: " .
                              socket_strerror(socket_last_error()) . "\n",
                              E_USER_WARNING);
            }
        }
        @socket_close($ctx['sock']);
    }
}

print "</b></pre>\n";

?>
<hr
 size="1"
/>
<p
 align="right"
 style="margin-top: 0px"
>You may find the <a
href="<?php echo kn_htmlEscape(kn__gpc('PHP_SELF', false, array('_SERVER'))) . '?show_source=1'; ?>"
>PHP source for this application</a> instructive.<br />
    Current <a href="http://www.php.net/" target="_blank">PHP</a> version:
    <?php echo kn_htmlEscape(phpversion()); ?></p>
</body>
</html>
