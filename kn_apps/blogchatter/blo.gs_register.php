#!/usr/local/bin/php

// Copyright 2003 Joyce Park.  All Rights Reserved.

// The function
function doRegister()
{
  $req = <<< EOBLOCK
<?xml version="1.0"?>
<methodCall>
<methodName>weblogUpdates.registerCloud</methodName>
<params>
<param><value><string>http://www.mod-pubsub.org/kn_apps/blogchatter/ping.php</string></value></param>
<param><value><string>xml-rpc</string></value></param>
<param><value><string>weblogUpdates.ping</string></value></param>
<param><value><string>blogchatter-test</string></value></param>
</params>
</methodCall>
EOBLOCK;



  $header .= "POST http://ping.blo.gs/ HTTP/1.0\r\n";
  $header .= "Content-Type: application/x-www-form-urlencoded\r\n";
  $header .= 'Content-Length: ' . strlen($req) . "\r\n\r\n";
  $fp = fsockopen ("http://ping.blo.gs/", 80, $errno, $errstr, 30);
    
  if (!$fp) {
    // ERROR
    error_log("$errstr [$errno]", 3, "/var/tmp/phplog");
  } else {   
    //put the data..
    fputs ($fp, $header . $req);
    while (!feof($fp)) {
      //read the data returned...
      $res = fgets($fp, 1024);
    }
  }
  fclose ($fp);
}



// Set it off
doRegister();

