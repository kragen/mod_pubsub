VERSION 5.00
Object = "{6B7E6392-850A-101B-AFC0-4210102A8DA7}#1.3#0"; "Comctl32.ocx"
Begin VB.Form Form1 
   Caption         =   "MsgViewer"
   ClientHeight    =   4530
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   4515
   LinkTopic       =   "Form1"
   ScaleHeight     =   4530
   ScaleWidth      =   4515
   StartUpPosition =   3  'Windows Default
   Begin ComctlLib.ListView ListView1 
      Height          =   3135
      Left            =   120
      TabIndex        =   6
      Top             =   1320
      Width           =   4215
      _ExtentX        =   7435
      _ExtentY        =   5530
      View            =   3
      LabelWrap       =   -1  'True
      HideSelection   =   -1  'True
      _Version        =   327682
      ForeColor       =   -2147483640
      BackColor       =   -2147483643
      BorderStyle     =   1
      Appearance      =   1
      NumItems        =   0
   End
   Begin VB.CommandButton UnsubBtn 
      Caption         =   "Unsub"
      Height          =   375
      Left            =   960
      TabIndex        =   5
      Top             =   840
      Width           =   855
   End
   Begin VB.CommandButton SubBtn 
      Caption         =   "Sub"
      Height          =   375
      Left            =   120
      TabIndex        =   4
      Top             =   840
      Width           =   735
   End
   Begin VB.TextBox TopicTxt 
      Height          =   285
      Left            =   960
      TabIndex        =   3
      Text            =   "/what/chat"
      Top             =   480
      Width           =   2295
   End
   Begin VB.TextBox ServerTxt 
      Height          =   285
      Left            =   960
      TabIndex        =   2
      Text            =   "http://sales.knownow.com/kn"
      Top             =   120
      Width           =   2295
   End
   Begin VB.Label Label2 
      Caption         =   "Topic"
      Height          =   255
      Left            =   120
      TabIndex        =   1
      Top             =   480
      Width           =   735
   End
   Begin VB.Label Label1 
      Caption         =   "Server"
      Height          =   255
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   735
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim g_Rid As String
Dim WithEvents g_Connector As Connector
Attribute g_Connector.VB_VarHelpID = -1
Dim g_Logger As New Logger

Private Sub ClearGrid()
    Dim ch As ColumnHeader
    ListView1.ColumnHeaders.Clear
    Set ch = ListView1.ColumnHeaders.Add(Text:="Field")
    Set ch = ListView1.ColumnHeaders.Add(Text:="Value")
    ch.Width = 4000
    ListView1.ListItems.Clear
End Sub

Private Sub Form_Load()
    ClearGrid
    g_Logger.Log "MsgViewer", MethodEntryExit, "Test"
    g_Rid = Empty
    Set g_Connector = New Connector
    UpdateUI
End Sub

Private Sub UpdateUI()
    Dim ridEmpty As Boolean
    
    If g_Rid = Empty Then
        ridEmpty = True
    Else
        ridEmpty = False
    End If
    
    SubBtn.Enabled = ridEmpty
    UnsubBtn.Enabled = Not ridEmpty
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Set g_Connector = Nothing
End Sub

Private Sub DumpMessage(ByVal m As String, ByVal msg As LIBKNCOMLib.IMessage)
    ListView1.ListItems.Add Text:=m
End Sub

Private Sub g_Connector_OnConnectionStatus(ByVal msg As LIBKNCOMLib.IMessage)
    DumpMessage "OnConnectionStatus", msg
End Sub

Private Sub g_Connector_OnStatus(ByVal msg As LIBKNCOMLib.IMessage)
    DumpMessage "OnStatus", msg
End Sub

Private Sub g_Connector_OnUpdate(ByVal msg As LIBKNCOMLib.IMessage)
    DumpMessage "OnUpdate", msg
End Sub

Private Sub SubBtn_Click()
    ClearGrid
    Dim p As Parameters
    Set p = New Parameters
    p.ServerUrl = ServerTxt.Text
    
    If Not g_Connector.Open(p) Then
        MsgBox "Cannot connect to the server"
        Exit Sub
    End If
    
    g_Rid = g_Connector.Subscribe(TopicTxt.Text)
    UpdateUI
End Sub

Private Sub UnsubBtn_Click()
    g_Connector.Unsubscribe g_Rid
    g_Connector.Close
    g_Rid = Empty
    UpdateUI
End Sub
