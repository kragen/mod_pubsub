<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<!--

 For a sample subscriber application, see mod_pubsub/kn_apps/chat1/ .
 Note that chat1 is a JavaScript PubSub Client Library subscriber.

-->

<!--

 Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.

 @KNOWNOW_LICENSE_START@
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 
 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in
 the documentation and/or other materials provided with the
 distribution.
 
 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
 be used to endorse or promote any product without prior written
 permission from KnowNow, Inc.
 
 THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 @KNOWNOW_LICENSE_END@

 $Id: publish.php,v 1.1 2003/03/22 09:08:47 ifindkarma Exp $

-->

<html>
<head>
<title>pubsub php publisher</title>
</head>
<body>
<?php;
if ($kn_server == "") {
    $kn_server = "http://${SERVER_ADDR}:${SERVER_PORT}/kn";
}
if ($kn_topic == "") {
    $kn_topic = $QUERY_STRING;
}
if ($kn_topic == "") {
    $kn_topic = "/what/chat";
}
?><h1>PHP Publisher: <?php;
    echo htmlentities($kn_topic);
?></h1><?php;
if ($kn_payload == "") {
    $kn_payload = "PHP Publishing Sample: Hello, world!";
}
if ($kn_userid == "") {
    $kn_userid = "anonymous";
}
if ($kn_displayname == "") {
    $kn_displayname = "Anonymous User";
}
if (stristr($kn_server, "http://") != $kn_server &&
    stristr($kn_server, "https://") != $kn_server)
{
    echo "<p>Unsupported scheme in PubSub Server URL.</p>\n";
}
else
{
    @$result =
	fopen($kn_server .
	      "?" . "do_method"
	      . "=" . "notify" .
	      ";" . "kn_response_format"
	      . "=" . "simple" .
	      ";" . "kn_to"
	      . "=" . urlencode($kn_topic) .
	      ";" . "displayname"
	      . "=" . urlencode($kn_displayname) .
	      ";" . "userid"
	      . "=" . urlencode($kn_userid) .
	      ";" . "kn_payload"
	      . "=" . urlencode($kn_payload),
	      "rb");
    if (!$result) {
	echo "<p>Unable to publish: HTTP GET request failed.</p>\n";
    }
    else
    {
	$match = 0;
	while (!feof ($result)) {
	    # FIXME: lines might be longer than this
	    $line = fgets($result, 1024);
	    if ($line == "")
	    {
		break;
	    }
	    # FIXME: those chars could be encoded
	    if (strstr($line, "status: ") == $line)
	    {
		$match = 1;
		?><p>Publish <?php;
		echo htmlentities(quoted_printable_decode($line));
		?></p><?php;
		break;
	    }
	}
	fclose($result);
	if (! $match)
	{
	    ?><p>Could not parse status event from PubSub Server.</p><?php;
	}
    }
}
?>
<form method="POST">
<dl>
<dt>PubSub Server:</dt>
<dd><input size="40" name="kn_server" value="<?php;
echo htmlentities($kn_server);
?>" /></dd>
<dt>Topic:</dt>
<dd><input size="40" name="kn_topic" value="<?php;
echo htmlentities($kn_topic);
?>" /></dd>
<dt>User ID:</dt>
<dd><input size="40" name="kn_userid" value="<?php;
echo htmlentities($kn_userid);
?>" /></dd>
<dt>Display Name:</dt>
<dd><input size="40" name="kn_displayname" value="<?php;
echo htmlentities($kn_displayname);
?>" /></dd>
<dt>Payload:</dt>
<dd><input size="40" name="kn_payload" value="<?php;
echo htmlentities($kn_payload);
?>" /></dd>
</dl>
<input type="submit" value="Publish" />
</form>
</body>
</html>
