Attribute VB_Name = "EventHandlers"
Option Explicit

Private Sub DumpMessage(ByVal m As String, ByVal msg As LIBKNCOMLib.IMessage)
    Dim s As String
    Dim msgEvt As MessageEntry
    If m = "OnUpdate" Then
        Dim title As String
        title = msg.Get("title")
        Dim desc As String
        desc = msg.Get("description")
        Dim link As String
        link = msg.Get("link")
        
        Dim myItem As ListItem
        Set myItem = RSSViewer.ListViewCtrl1.ListItems.Add(1, , title)
        myItem.SubItems(1) = desc
        myItem.SubItems(2) = link
    Else
        'Do something here
    End If
    
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

