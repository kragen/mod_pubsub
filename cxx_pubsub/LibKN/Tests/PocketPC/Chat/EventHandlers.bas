Attribute VB_Name = "EventHandlers"
Option Explicit

Private Sub DumpMessage(ByVal m As String, ByVal msg As LIBKNCOMLib.IMessage)
    Dim s As String
    Dim msgEvt As MessageEntry
    If m = "OnUpdate" Then
        Dim u As String
        u = msg.Get("displayname")
        If u = "" Then
            u = "(anonymous)"
        End If
        s = s & u & ": "
        s = s & msg.Get("kn_payload") & vbCrLf
    Else
        s = m & vbCrLf
        For Each msgEvt In msg
            s = s & "  " & msgEvt.Field & ":" & msgEvt.Value & vbCrLf
        Next
    End If
    Form1.Text1.Text = Form1.Text1.Text & s
    Form1.Text1.SelStart = Len(Form1.Text1.Text)
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
