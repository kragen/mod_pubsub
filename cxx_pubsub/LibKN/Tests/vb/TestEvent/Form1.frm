VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   3615
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   5415
   LinkTopic       =   "Form1"
   ScaleHeight     =   3615
   ScaleWidth      =   5415
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton UnsubBtn 
      Caption         =   "Unsubscribe"
      Height          =   615
      Left            =   240
      TabIndex        =   2
      Top             =   1680
      Width           =   1335
   End
   Begin VB.CommandButton SubBtn 
      Caption         =   "Subscribe"
      Height          =   615
      Left            =   240
      TabIndex        =   1
      Top             =   960
      Width           =   1335
   End
   Begin VB.CommandButton PubBtn 
      Caption         =   "Publish!"
      Height          =   615
      Left            =   240
      TabIndex        =   0
      Top             =   240
      Width           =   1335
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim WithEvents g_Connector As Connector
Attribute g_Connector.VB_VarHelpID = -1
Dim g_Rid As String

Private Sub DumpMessage(ByVal msg As LIBKNCOMLib.IMessage)
    Dim p As MessageEntry
    For Each p In msg
        Debug.Print "  " & p.Field & "=" & p.Value
    Next p
End Sub

Private Sub Form_Load()
    Set g_Connector = New Connector
    Dim p As New Parameters
    p.ServerUrl = "http://localhost:8000/kn"
    g_Connector.Open p
    g_Rid = Empty
End Sub

Private Sub Form_Unload(Cancel As Integer)
    UnsubBtn_Click
    g_Connector.Close
    Set g_Connector = Nothing
End Sub

Private Sub g_Connector_OnConnectionStatus(ByVal msg As LIBKNCOMLib.IMessage)
    Debug.Print "OnConnectionStatus"
    DumpMessage msg
End Sub

Private Sub g_Connector_OnStatus(ByVal msg As LIBKNCOMLib.IMessage)
    Debug.Print "OnStatus"
    DumpMessage msg
End Sub

Private Sub g_Connector_OnUpdate(ByVal msg As LIBKNCOMLib.IMessage)
    Debug.Print "OnUpdate"
    DumpMessage msg
End Sub

Private Sub PubBtn_Click()
    Dim m As New Message
    m.Set "do_method", "notify"
    m.Set "kn_payload", "Hello world"
    m.Set "nickname", "VB2"
    m.Set "kn_to", "/what/knchat/messages"
    m.Set "kn_response_format", "simple"
    g_Connector.Publish m
End Sub

Private Sub SubBtn_Click()
    If g_Rid = Empty Then
        g_Rid = g_Connector.Subscribe("/what/knchat/messages")
    End If
End Sub

Private Sub UnsubBtn_Click()
    If g_Rid <> Empty Then
        g_Connector.Unsubscribe g_Rid
        g_Rid = ""
    End If
End Sub
