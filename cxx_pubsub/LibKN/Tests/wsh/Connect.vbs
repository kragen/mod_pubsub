option explicit

DIM argv
set argv = WScript.Arguments

WScript.Echo argv.Count & " arguments"

IF argv.Count <> 1 THEN
    WScript.Echo "Expecting two arguments: [server]"
    WScript.Quit
END IF

SUB DumpMessage(msg, m)
   WScript.Echo msg
   DIM messageEntry
   FOR EACH messageEntry in m
       WScript.Echo "  " & messageEntry.Field & ":" & messageEntry.Value
   NEXT
END SUB

SUB CONNEVENT_OnUpdate(byval m)
   DumpMessage "OnUpdate", m
END SUB

SUB CONNEVENT_OnStatus(byval m)
   DumpMessage "OnStatus", m
END SUB

SUB CONNEVENT_OnConnectionStatus(byval m)
   DumpMessage "OnConnectionStatus", m
END SUB

dim conn
set conn = WScript.CreateObject("LibKNCom.Connector", "CONNEVENT_")

dim params
set params = WScript.CreateObject("LibKNCom.Parameters")

params.ServerUrl = argv(0)
params.ShowUI = false

DIM b
b = conn.Open(params)

IF b THEN
    WScript.Echo "Successfully opened"
    
        WHILE 1
            WScript.Sleep 10
        WEND
    
    WScript.Sleep 1 
    conn.Close
    WScript.Echo "Done"
else
    WScript.Echo "Cannot open connection to server"
END IF


