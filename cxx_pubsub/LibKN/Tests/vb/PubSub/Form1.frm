VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   4395
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   6555
   LinkTopic       =   "Form1"
   ScaleHeight     =   4395
   ScaleWidth      =   6555
   StartUpPosition =   3  'Windows Default
   Begin VB.ListBox MsgList 
      Height          =   3960
      Left            =   1680
      TabIndex        =   3
      Top             =   240
      Width           =   4695
   End
   Begin VB.CommandButton UnsubBtn 
      Caption         =   "Unsubscribe"
      Height          =   495
      Left            =   120
      TabIndex        =   2
      Top             =   1440
      Width           =   1455
   End
   Begin VB.CommandButton SubscribeBtn 
      Caption         =   "Subscribe!"
      Height          =   495
      Left            =   120
      TabIndex        =   1
      Top             =   840
      Width           =   1455
   End
   Begin VB.CommandButton testPubBtn 
      Caption         =   "Publish!"
      Height          =   495
      Left            =   120
      TabIndex        =   0
      Top             =   240
      Width           =   1455
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
Dim g_Parameters As New Parameters
Dim g_MyStatusHandler As New MyStatusHandler
Dim g_MyListener As New MyListener
Dim g_Rid As String


Private Sub Form_Load()
    Set g_Connector = New Connector
    Dim b As Boolean
    g_Parameters.ServerUrl = "http://localhost:8000/kn"
    b = g_Connector.Open(g_Parameters)
    If Not b Then
        Debug.Print "Failed to connect"
        Exit Sub
    End If
    
    g_Rid = ""
    UpdateUI

'    Dim m As New Message
'    Dim m2 As New Message
'
'    m.Set "a", "b"
'    Debug.Print "m = "
'    DumpMessage m
'
'    m2.Copy m
'    Debug.Print "m2 = "
'    DumpMessage m2
'
End Sub

Private Sub UpdateUI()
    If g_Rid = "" Then
        SubscribeBtn.Enabled = True
        UnsubBtn.Enabled = False
    Else
        SubscribeBtn.Enabled = False
        UnsubBtn.Enabled = True
    End If
    
End Sub

Private Sub Form_Unload(Cancel As Integer)
    g_Connector.Close
    Set g_Connector = Nothing
End Sub

Private Sub LBDumpMessage(ByVal msg As String, ByVal m As Message)
    Dim msgEnt As MessageEntry
    MsgList.AddItem msg
    For Each msgEnt In m
        MsgList.AddItem "  " & msgEnt.Field & ":" & msgEnt.Value
    Next
End Sub

Private Sub g_Connector_OnConnectionStatus(ByVal msg As LIBKNCOMLib.IMessage)
    LBDumpMessage "OnConnectionStatus", msg
End Sub

Private Sub g_Connector_OnStatus(ByVal msg As LIBKNCOMLib.IMessage)
    LBDumpMessage "OnStatus", msg
End Sub

Private Sub g_Connector_OnUpdate(ByVal msg As LIBKNCOMLib.IMessage)
    LBDumpMessage "OnUpdate", msg
End Sub

Private Sub SubscribeBtn_Click()
    Dim options As New Message
    
    If g_Rid = "" Then
        g_Rid = g_Connector.Subscribe("/what/knchat/messages", g_MyListener, options, g_MyStatusHandler)
    Else
    End If
    UpdateUI
    
End Sub

Private Sub testPubBtn_Click()
    Dim m As New Message
    m.Set "do_method", "notify"
    m.Set "kn_to", "/what/knchat/messages"
    m.Set "nickname", "vbTest"
    m.Set "kn_payload", "Hello"
    g_Connector.Publish m, g_MyStatusHandler
    
End Sub

Private Sub UnsubBtn_Click()
    If g_Rid = "" Then
    Else
        g_Connector.Unsubscribe g_Rid, g_MyStatusHandler
        g_Rid = ""
    End If
    UpdateUI
End Sub
