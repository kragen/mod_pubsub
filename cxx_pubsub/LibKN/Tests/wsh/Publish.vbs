'
' $Log: Publish.vbs,v $
' Revision 1.2  2003/08/15 23:46:14  ifindkarma
' Added changes from August 2003.
'
' 
' 2     4/02/03 10:53a Thui
' Added check for publish failure
' 
' 1     3/03/03 9:54p Thui
'

option explicit

DIM argv
set argv = WScript.Arguments

' WScript.Echo argv.Count & " arguments"

IF argv.Count <> 2 THEN
    WScript.Echo "Expecting two arguments: [server] [topic]"
    WScript.Quit
END IF

SUB DumpMessage(msg, m)
   DIM display
   display = msg
   DIM messageEntry
   FOR EACH messageEntry in m
       display = display & vbCrLf & "  " & messageEntry.Field & ":" & messageEntry.Value
   NEXT
    WScript.Echo display
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

FUNCTION GetPayload()
   DIM s
   s = "Hello "
   WHILE len(s) < 300
       s = s & " " & Now
   WEND
   GetPayload = s
END FUNCTION

dim conn
set conn = WScript.CreateObject("LibKNCom.Connector", "CONNEVENT_")

dim params
set params = WScript.CreateObject("LibKNCom.Parameters")

'params.ServerUrl = "http://localhost:8000/kn"
params.ServerUrl = argv(0)

DIM b
b = conn.Open(params)

IF b THEN
    DIM p
    p = GetPayload
    ' WScript.Echo "p = " & p
    
    DIM m
    DIM n
    set m = CreateObject("LibKNCom.Message")
    m.Set "do_method", "notify"
    m.Set "kn_to", argv(1)
    m.Set "nickname", "WSH"
    m.Set "kn_payload", p
    m.Set "kn_response_format", "simple"
    ' DumpMessage "m", m
    set n = nothing
    
    b = conn.Publish(m, n)
    
    IF not b THEN
        WScript.Echo "Publish failed"
    END IF
    
    WScript.Sleep 1
    conn.Close
    
END IF

' WScript.Echo "Done"
