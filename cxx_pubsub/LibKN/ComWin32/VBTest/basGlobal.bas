Attribute VB_Name = "basGlobal"
Option Explicit


Public Sub DumpMessage(ByVal msg As String, ByVal m As Message)
    Dim msgEnt As MessageEntry
    Debug.Print msg
    For Each msgEnt In m
        Debug.Print msgEnt.Field & ":" & msgEnt.Value
    Next
End Sub


