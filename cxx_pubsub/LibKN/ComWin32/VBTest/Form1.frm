VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   3435
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   4935
   LinkTopic       =   "Form1"
   ScaleHeight     =   3435
   ScaleWidth      =   4935
   StartUpPosition =   3  'Windows Default
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

Dim g_Connector As New Connector
Dim g_Parameters As New Parameters
Dim g_MyStatusHandler As New MyStatusHandler
Dim g_MyListener As New MyListener
Dim g_Rid As String


Private Sub Form_Load()
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
    
End Sub

Private Sub SubscribeBtn_Click()
    If g_Rid = "" Then
        g_Rid = g_Connector.Subscribe("/what/knchat/messages", g_MyListener, g_MyStatusHandler)
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
