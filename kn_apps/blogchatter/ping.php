<?php

/*

  Copyright 2003 Scott Andrew LePera  All Rights Reserved.

*/

include_once("xmlrpc.inc");
include_once("xmlrpcs.inc");


function forward_ping($m)
{

		$host = "www.mod-pubsub.org";
    $path = "/kn/what/apps/blogchatter/pings";

    $param = $m->getParam(0);
    $name = $param->scalarval();
    
    $param = $m->getParam(1);
    $url = $param->scalarval();
    
    $req = "kn_expires=" . urlencode("+1800") . ";blog_name=" .urlencode($name) . ";blog_url=" . urlencode($url);
    
		doPublish($host,$path,$req);
            
     $ret = new xmlrpcval(array(
                "flerror" => new xmlrpcval(0,"boolean"),
                "message" => new xmlrpcval("Ping received","string")
                 ),"struct");
            
    $msg = new xmlrpcresp($ret);
    return $msg;

}

function doPublish($host,$path,$req)
{
    $header .= "POST " . $path . " HTTP/1.0\r\n";
    $header .= "Content-Type: application/x-www-form-urlencoded\r\n";
    $header .= 'Content-Length: ' . strlen($req) . "\r\n\r\n";
    $fp = fsockopen ($host, 9000, $errno, $errstr, 30);
    
    if (!$fp) {
        // ERROR
        echo "$errstr ($errno)";
    } else 
    {   
        //put the data..
        fputs ($fp, $header . $req);
        while (!feof($fp))
	{
        //read the data returned...
        $res = fgets($fp, 1024);
    	}
    }
    fclose ($fp);
}


// dispatch map

$server=new xmlrpc_server
(
    array
    ("weblogUpdates.ping" => array
        (
        "function" => "forward_ping"
        )
    )
);

?>

