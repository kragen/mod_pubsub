<?php

// Copyright 2003 Scott Andrew LePera  All Rights Reserved.
// Modifications copyright 2003 Joyce Park.  All Rights Reserved.



// The form front-end
$manualpingform = <<< EOFORM
<HTML>

<HEAD>
<TITLE>Joyce's crappy manual ping form</TITLE>
</HEAD>

<BODY>
<FORM METHOD="post" ACTION="$self">
Your blog's name:  <INPUT TYPE="text" NAME="blogname" SIZE=50><BR>
Your blog's URL:  <INPUT TYPE="text" NAME="blogurl" SIZE=50><BR>
<INPUT TYPE="submit" NAME="Ping" VALUE="Ping mod-pubsub blogs">
</FORM>
</BODY>
</HTML>
EOFORM;


// The function
function doPublish()
{
    $host = "www.mod-pubsub.org";
    $path = "/kn/what/apps/blogchatter/pings";

    $blogname = $_POST['blogname'];
    $blogurl = $_POST['blogurl'];
    
    $req = "kn_expires=" . urlencode("+1800") . ";blog_name=" . urlencode($blogname) . ";blog_url=" . urlencode($blogurl);


    $header .= "POST " . $path . " HTTP/1.0\r\n";
    $header .= "Content-Type: application/x-www-form-urlencoded\r\n";
    $header .= 'Content-Length: ' . strlen($req) . "\r\n\r\n";
    $fp = fsockopen ($host, 9000, $errno, $errstr, 30);
    
    if (!$fp) {
        // ERROR
        echo "$errstr ($errno)";
    } else {   
        //put the data..
        fputs ($fp, $header . $req);
        while (!feof($fp)) {
            //read the data returned...
            $res = fgets($fp, 1024);
    	}
        //echo $header.$req;
        // Echo a message
        // TODO: make this valid HTML instead of a ghetto echo
        echo "Pinged!";
    }
    fclose ($fp);
}



// Set it off
if ($_POST['Ping'] != 'Ping mod-pubsub blogs') {
  echo $manualpingform;
} else {
  doPublish();
}

?>

