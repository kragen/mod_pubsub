option explicit

DIM argv
set argv = WScript.Arguments

WScript.Echo argv.Count & " arguments"

IF argv.Count <> 2 THEN
    WScript.Echo "Expecting two arguments: [server] [topic]"
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

'params.ServerUrl = "http://localhost:8000/kn"
params.ServerUrl = argv(0)

DIM b
b = conn.Open(params)

IF b THEN
    WScript.Echo "Successfully opened"
    
    DIM m
    set m = WScript.CreateObject("LibKNCom.Message")
    m.Set "Hello", "World"
    m.Set "do_max_n", "1"
    DIM rid
    DIM n
    set n = nothing
    WScript.Echo "Subscribing to " & argv(1)
    rid = conn.Subscribe(argv(1), n, m, n)
    
    WScript.Echo len(rid) & " " & rid
    
    IF len(rid) = 0 THEN
        WScript.Echo "Failed to subscribe, exiting."
        conn.Close
        WScript.Quit
    END IF
    
    WHILE 1
        WScript.Sleep 10
    WEND
    
    conn.Close
    WScript.Echo "Done"
else
    WScript.Echo "Cannot open connection to server"
END IF


