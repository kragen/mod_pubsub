Attribute VB_Name = "EventHandlers"
Option Explicit

Private Sub DumpMessage(ByVal m As String, ByVal msg As LIBKNCOMLib.IMessage)
    Dim s As String
    s = m & vbCrLf
    Dim msgEvt As MessageEntry
    For Each msgEvt In msg
        s = s & "  " & msgEvt.Field & ":" & msgEvt.Value & vbCrLf
    Next
    Form1.Text1.Text = Form1.Text1.Text & s
    Form1.Text1.SelStart = Len(Form1.Text1.Text)
End Sub

Private Sub MyConnector_OnConnectionStatus(ByVal msg As LIBKNCOMLib.IMessage)
    DumpMessage "OnConnectionStatus", msg
End Sub

Private Sub MyConnector_OnStatus(ByVal msg As LIBKNCOMLib.IMessage)
    DumpMessage "OnStatus", msg
End Sub

Private Sub MyConnector_OnUpdate(ByVal msg As LIBKNCOMLib.IMessage)
    DumpMessage "OnUpdate", msg
End Sub

