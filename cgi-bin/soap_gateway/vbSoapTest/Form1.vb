Public Class Form1
    Inherits System.Windows.Forms.Form

#Region " Windows Form Designer generated code "

    Public Sub New()
        MyBase.New()

        'This call is required by the Windows Form Designer.
        InitializeComponent()

        'Add any initialization after the InitializeComponent() call

    End Sub

    'Form overrides dispose to clean up the component list.
    Protected Overloads Overrides Sub Dispose(ByVal disposing As Boolean)
        If disposing Then
            If Not (components Is Nothing) Then
                components.Dispose()
            End If
        End If
        MyBase.Dispose(disposing)
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    Friend WithEvents TextBox1 As System.Windows.Forms.TextBox
    Friend WithEvents Label1 As System.Windows.Forms.Label
    Friend WithEvents Button1 As System.Windows.Forms.Button
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Me.TextBox1 = New System.Windows.Forms.TextBox()
        Me.Label1 = New System.Windows.Forms.Label()
        Me.Button1 = New System.Windows.Forms.Button()
        Me.SuspendLayout()
        '
        'TextBox1
        '
        Me.TextBox1.Location = New System.Drawing.Point(136, 16)
        Me.TextBox1.Name = "TextBox1"
        Me.TextBox1.TabIndex = 0
        Me.TextBox1.Text = "TextBox1"
        '
        'Label1
        '
        Me.Label1.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, (System.Drawing.FontStyle.Bold Or System.Drawing.FontStyle.Italic), System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label1.ImageAlign = System.Drawing.ContentAlignment.TopRight
        Me.Label1.Location = New System.Drawing.Point(32, 16)
        Me.Label1.Name = "Label1"
        Me.Label1.Size = New System.Drawing.Size(88, 23)
        Me.Label1.TabIndex = 1
        Me.Label1.Text = "Text to Publish:"
        Me.Label1.TextAlign = System.Drawing.ContentAlignment.TopRight
        '
        'Button1
        '
        Me.Button1.Location = New System.Drawing.Point(88, 64)
        Me.Button1.Name = "Button1"
        Me.Button1.Size = New System.Drawing.Size(104, 23)
        Me.Button1.TabIndex = 2
        Me.Button1.Text = "Click to Publish"
        '
        'Form1
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
        Me.ClientSize = New System.Drawing.Size(292, 266)
        Me.Controls.AddRange(New System.Windows.Forms.Control() {Me.Button1, Me.Label1, Me.TextBox1})
        Me.Name = "Form1"
        Me.Text = "Form1"
        Me.ResumeLayout(False)

    End Sub

#End Region

    Private Sub Label1_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Label1.Click

    End Sub

    Private Sub Button1_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button1.Click
        Dim kn As New pubsub.PubSubService()

        Dim myMap(1) As pubsub.MapItem

        Dim statusEvent As Object()
        Dim result As String

         myMap(0) = New pubsub.MapItem()

        myMap(0).key = "kn_payload"
        myMap(0).value = TextBox1.Text
        kn.pubsubrequest(myMap, statusEvent)
        If (statusEvent Is Nothing) Then
            MsgBox("statusEvent is nothing")
        Else
            Dim statusEventHash As Hashtable = New Hashtable()
            Dim idx As Integer
            For idx = 0 To statusEvent.Length - 1
                If (statusEvent(idx).GetType.Name = "XmlElement") Then
                    Dim xmlitem As Xml.XmlElement = statusEvent(idx)
                    Dim key As String = xmlitem.GetElementsByTagName("key").Item(0).InnerText
                    Dim value As String = xmlitem.GetElementsByTagName("value").Item(0).InnerText
                    statusEventHash.Add(key, value)
                End If
            Next
            Dim msg As String
            Dim item As DictionaryEntry
            For Each item In statusEventHash
                msg = msg + Chr(9) + item.Key + ": " + item.Value + Chr(13) + Chr(10)
            Next
            MsgBox("statusEvent: {" + Chr(13) + Chr(10) + msg + "}")
        End If
    End Sub
End Class
