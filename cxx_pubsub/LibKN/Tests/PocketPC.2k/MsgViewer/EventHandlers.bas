Attribute VB_Name = "EventHandlers"
Option Explicit

Private Sub DumpMessage(ByVal m As String, ByVal msg As LIBKNCOMLib.IMessage)
    Form1.DumpMessage m, msg
End Sub

Private Sub MyConnector_OnConnectionStatus(ByVal msg As LIBKNCOMLib.IMessage)
    DumpMessage "OnConnectionStatus", msg
End Sub

Private Sub MyConnector_OnStatus(ByVal msg As LIBKNCOMLib.IMessage)
    ' DumpMessage "OnStatus", msg
End Sub

Private Sub MyConnector_OnUpdate(ByVal msg As LIBKNCOMLib.IMessage)
    DumpMessage "OnUpdate", msg
End Sub

