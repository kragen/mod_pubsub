<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>Simple event dump</title>
    <script type="text/javascript" src="/kn?do_method=lib"></script>

<?php
// Copyright 2003 Joyce Park

// This is a very simple PHP-based tool that dumps out all the elements of
// each event when pointed at a topic.  It's less cool than the Introspect
// tool, but much faster when you know what you want.  The call to kn_lib
// has to be in the shared header, even though it's only used in the display
// screen, otherwise the form chokes.


// The form front-end
$php_self = $_SERVER['PHP_SELF'];
$subscribeform = <<< EOFORM
</head>

<body>
<form method="post" action="$php_self">
Topic:  <input type="text" name="topic" size="50"><br />
<input type="submit" name="Subscribe" value="Subscribe">
</form>
</body>
</html>
EOFORM;

$topic = $_POST['topic'];
// The resulting DHTML page
$subscribepage = <<< EOSUBPAGE
    <script type="text/javascript">

    function rssSanitize(temp_str)
    {
      // Makes sure you display RSS properly
      temp_str = temp_str.replace(/&/g, "&amp;");
      temp_str = temp_str.replace(/</g, "&lt;");
      temp_str = temp_str.replace(/>/g, "&gt;");
      return temp_str;
    }

    function init()
    {
        kn.subscribe("$topic",
                    dumpEvent,
                    {do_max_n:1},
		    {onSuccess:renderDisplay}
        );
    }
    
    var event = [];
    var busy = false;
    
    function dumpEvent(evt)
    {
        event[0] = evt;
        renderDisplay();   
    }
    
    function renderDisplay()
    {
        if (busy)
        {
        	setTimeout("renderDisplay();",5000);
        	return;
        }
        busy = true;
	var dump_str = '';
	for(var prop in event[0]) {
	  dump_str += prop + "  =>  " + rssSanitize(event[0][prop]) + "<br /><br />";
	}

	document.getElementById("dump_event").innerHTML = dump_str;
		
	busy = false;        
    }
    
    onload = init;
    
    </script>
</head>

<body>
<div id="dump_event">
Event content goes here
</div>
</body>
</html>
EOSUBPAGE;


if (!isSet($_POST['Subscribe'])) {
  echo $subscribeform;
} else {
  echo $subscribepage;
}

?>

