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

#  For a sample subscriber application, see mod_pubsub/kn_apps/chat1/ .
#  Note that chat1 is a JavaScript PubSub Client Library subscriber.

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

$RCSID = '$Id: publish.php,v 1.4 2003/03/28 06:57:19 bsittler Exp $';

?>
<title>pubsub publisher for php</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
</head>
<body>
<?php

if (! isset($kn_topic))
{
    $kn_topic = "/what/chat";
}

$publish_topic =
    kn__gpc("publish_topic",
            $kn_topic);

$publish_payload =
    kn__gpc("publish_payload",
            "Publishing Sample for PHP: Hello, world!");

if (kn__gpc("publish", PUBSUB_NULL, array("_GET", "_POST")) == PUBSUB_NULL)
{
    ?><h1>Publisher for PHP</h1><?php
}
else
{
    $kn->publish($publish_topic,
                 $publish_payload,
'
    $statusEvent = $args[0];
    echo "<h1>" . kn_htmlEscape($statusEvent["status"]) . "</h1>";
    echo "<pre>" . kn_htmlEscape($statusEvent["publish_payload"]) . "</pre>";

');

}

?>
<form action="<?php echo kn_htmlEscape(kn__gpc('PHP_SELF', false, array('_SERVER'))); ?>" method="POST">
<dl>
    <?php

$fields =
    array("publish_topic" => array($publish_topic,
                                   "Topic"),
          "publish_payload" => array($publish_payload,
                                     "Payload"));
while (list($name, $value) = each($fields))
{
    ?>
        <dt><?php echo kn_htmlEscape($value[1]); ?>:</dt>
        <dd><input
        size="40"
        name="<?php echo kn_htmlEscape($name); ?>"
        value="<?php echo kn_htmlEscape($value[0]); ?>"
        /></dd>
        <?php
}

?>
    </dl>
    <input
    type="submit"
    name="publish"
    value="Publish"
    />
    </form>

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
