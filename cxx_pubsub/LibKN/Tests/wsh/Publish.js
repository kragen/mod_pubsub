/*
 * $Log: Publish.js,v $
 * Revision 1.2  2003/08/15 23:46:14  ifindkarma
 * Added changes from August 2003.
 *
 * 
 * 2     3/03/03 9:55p Thui
 */
 
var JArgUtil = WScript.CreateObject("Torrboy.JArgUtility");


var argv = WScript.arguments;

if (argv.length != 2)
{
	WScript.Echo("Expecting two arguments: [server] [topic]");
	WScript.Quit();
}

function DumpMessage(msg, m)
{
	var display = msg + "\n";

	for (var e = new Enumerator(m); !e.atEnd(); e.moveNext())
	{
		var messageEntry = e.item();
		display += "\t" + messageEntry.Field + ":" + messageEntry.Value + "\n";
	}

	return display;
}

function CONNEVENT_OnStatus(m)
{
	DumpMessage("OnStatus", m);
}

function CONNEVENT_OnConnectionStatus(m)
{
	DumpMessage("OnConnectionStatus", m);
}

function CONNEVENT_OnUpdate(m)
{
	DumpMessage("OnUpdate", m);
}

var conn = WScript.CreateObject("LibKNCom.Connector", "CONNEVENT_");
var params = WScript.CreateObject("LibKNCom.Parameters");

params.ServerUrl = argv(0);

if (conn.open(params))
{
	var m = WScript.CreateObject("LibKNCom.Message");

	m.Set("do_method", "notify");
	m.Set("kn_to", argv(1));
	m.Set("nickname", "JScript");
	m.Set("kn_payload", "Hello " + new Date());
	m.Set("kn_response_format", "simple");

	WScript.Echo(DumpMessage("m=", m));

	var b = conn.Publish(m, JArgUtil.Nothing);

	WScript.Sleep(100);
	conn.close();
}


