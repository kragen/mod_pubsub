Imports LibKNDotNet
Imports System
Imports System.Xml
Imports System.Collections
Imports System.IO.StringWriter

Module TestUtil
    Public TU_OK As String = "OK"

    Dim TU_VERBOSE_ALL As Boolean = False
    Dim TU_TESTSUITE As String = ""
    Dim TU_TESTCASE As String = ""
    Dim TU_SERVER As String = ""
    Dim TU_MISSINGSERVER As String = ""
    Dim TU_SERVERNAME As String = ""
    Dim tu_servers As New ArrayList()

    Dim tu_count_subListener_OnUpdate As Integer = 0
    Dim tu_count_subConnSH_OnConnStatus As Integer = 0
    Dim tu_count_subReqSH_OnError As Integer = 0
    Dim tu_count_subReqSH_OnSuccess As Integer = 0
    Dim tu_count_pubConnSH_OnConnStatus As Integer = 0
    Dim tu_count_pubReqSH_OnError As Integer = 0
    Dim tu_count_pubReqSH_OnSuccess As Integer = 0

    '/////////////////////////////////////////////////////////////////////////
    '/// Public Events
    '/////////////////////////////////////////////////////////////////////////

    Public Class TU_SubListener
        Inherits LibKNDotNet.IListener

        Public Overrides Sub OnUpdate(ByVal msg As LibKNDotNet.Message)
            TU_dumpMsg("TU_SubListener.OnUpdate", msg)
            tu_count_subListener_OnUpdate = tu_count_subListener_OnUpdate + 1
        End Sub

    End Class

    Public Class TU_SubConnStatusHandler
        Inherits LibKNDotNet.IConnectionStatusHandler

        Public Overrides Sub OnConnectionStatus(ByVal msg As LibKNDotNet.Message)
            TU_dumpMsg("TU_SubConnStatusHandler.OnConnectionStatus", msg)
            tu_count_subConnSH_OnConnStatus = tu_count_subConnSH_OnConnStatus + 1
        End Sub
    End Class

    Public Class TU_PubConnStatusHandler
        Inherits LibKNDotNet.IConnectionStatusHandler

        Public Overrides Sub OnConnectionStatus(ByVal msg As LibKNDotNet.Message)
            TU_dumpMsg("TU_PubConnStatusHandler.OnConnectionStatus", msg)
            tu_count_pubConnSH_OnConnStatus = tu_count_pubConnSH_OnConnStatus + 1
        End Sub
    End Class

    Public Class TU_SubRequestStatusHandler
        Inherits LibKNDotNet.IRequestStatusHandler

        Public Overrides Sub OnError(ByVal msg As LibKNDotNet.Message)
            TU_dumpMsg("TU_SubRequestStatusHandler.OnError", msg)
            tu_count_subReqSH_OnError = tu_count_subReqSH_OnError + 1
        End Sub

        Public Overrides Sub OnSuccess(ByVal msg As LibKNDotNet.Message)
            TU_dumpMsg("TU_SubRequestStatusHandler.OnSuccess", msg)
            tu_count_subReqSH_OnSuccess = tu_count_subReqSH_OnSuccess + 1
        End Sub
    End Class

    Public Class TU_PubRequestStatusHandler
        Inherits LibKNDotNet.IRequestStatusHandler

        Public Overrides Sub OnError(ByVal msg As LibKNDotNet.Message)
            TU_dumpMsg("TU_PubRequestStatusHandler.OnError", msg)
            tu_count_pubReqSH_OnError = tu_count_pubReqSH_OnError + 1
        End Sub

        Public Overrides Sub OnSuccess(ByVal msg As LibKNDotNet.Message)
            TU_dumpMsg("TU_PubRequestStatusHandler.OnSuccess", msg)
            tu_count_pubReqSH_OnSuccess = tu_count_pubReqSH_OnSuccess + 1
        End Sub
    End Class

    '/////////////////////////////////////////////////////////////////////////
    '/// Public
    '/////////////////////////////////////////////////////////////////////////

    Public Function TU_getCount_pubConnSH_OnConnStatus() As String
        Return tu_count_pubConnSH_OnConnStatus
    End Function

    Public Function TU_getCount_pubReqSH_OnError() As String
        Return tu_count_pubReqSH_OnError
    End Function

    Public Function TU_getCount_pubReqSH_OnSuccess() As String
        Return tu_count_pubReqSH_OnSuccess
    End Function

    Public Function TU_getCount_subListener_OnUpdate() As String
        Return tu_count_subListener_OnUpdate
    End Function

    Public Function TU_getCount_subConnSH_OnConnStatus() As String
        Return tu_count_subConnSH_OnConnStatus
    End Function

    Public Function TU_getCount_subReqSH_OnError() As String
        Return tu_count_subReqSH_OnError
    End Function

    Public Function TU_getCount_subReqSH_OnSuccess() As String
        Return tu_count_subReqSH_OnSuccess
    End Function


    Public Function TU_getTestsuiteName() As String
        Return TU_TESTSUITE
    End Function

    Public Function TU_getTestcaseName() As String
        Return TU_TESTCASE
    End Function

    Public Function TU_getServer() As String
        Return TU_SERVER
    End Function

    Public Function TU_getServer(ByVal index As Integer) As String
        If tu_servers.Count < index Then
            Return tu_servers.Item(index)
        Else
            Return ""
        End If
    End Function

    Public Function TU_getMissingServer() As String
        Return TU_MISSINGSERVER
    End Function

    ' * Use this if you are rerunning the same testsuite or testcase and
    ' * want to force re-reading from the XML params.xml file.
    Public Sub RESET_TESTSUITE()
        TU_VERBOSE_ALL = False
        TU_TESTSUITE = ""
        TU_TESTCASE = ""
        TU_SERVER = ""
        TU_MISSINGSERVER = ""
        TU_SERVERNAME = ""
        tu_servers.Clear()
    End Sub

    Public Sub TU_INIT_TESTSUITE(ByVal testSuiteName As String)
        ' if new testsuite name restart everything, 
        ' else same testsuite so leave everything the same.
        If testSuiteName.Equals(TU_TESTSUITE) Then
            Return
        End If

        ' reset everything.
        RESET_TESTSUITE()

        ' setup everything from XML params.xml
        TU_TESTSUITE = testSuiteName
        TU_VERBOSE_ALL = False
        Dim reader As XmlTextReader = New XmlTextReader("Params.xml")
        Do While (reader.Read())
            If reader.Name = "verbose" Then
                TU_VERBOSE_ALL = True
            ElseIf reader.Name = "serverName" Then
                If reader.IsStartElement Then
                    If reader.IsEmptyElement = False Then
                        reader.Read()
                        TU_SERVERNAME = reader.Value
                    End If
                End If
            ElseIf reader.Name = "server" Then
                If reader.IsStartElement Then
                    If reader.IsEmptyElement = False Then
                        reader.Read()
                        tu_servers.Add(reader.Value)
                    End If
                End If
            ElseIf reader.Name = "missingServer" Then
                If reader.IsStartElement Then
                    If reader.IsEmptyElement = False Then
                        reader.Read()
                        TU_MISSINGSERVER = reader.Value
                    End If
                End If
            End If
        Loop
        If tu_servers.Count() > 0 Then
            TU_SERVER = tu_servers.Item(0)
        End If

        If TU_VERBOSE_ALL = False Then
            Return
        End If

        Console.WriteLine()
        Console.WriteLine("===============================================================================")
        Console.WriteLine("TU_VERBOSE_ALL  =" & TU_VERBOSE_ALL)
        Console.WriteLine("TU_SERVERNAME   =" & TU_SERVERNAME)
        Console.WriteLine("TU_SERVER       =" & TU_SERVER)
        Console.WriteLine("TU_MISSINGSERVER=" & TU_MISSINGSERVER)
        Console.WriteLine("tu_servers      =" & tu_toString(tu_servers))
        Console.WriteLine("===============================================================================")
    End Sub

    Public Sub TU_INIT_TESTCASE(ByVal testCaseName As String)
        TU_TESTCASE = testCaseName
        If TU_VERBOSE_ALL = False Then
            Return
        End If
        Console.WriteLine()
        Console.WriteLine("===============================================================================")
        Console.WriteLine("=== Start test: " & TU_TESTSUITE & "." & testCaseName)
        Console.WriteLine("===============================================================================")
    End Sub

    Public Sub TU_dump(ByVal msg As String)
        If TU_VERBOSE_ALL = False Then
            Return
        End If

        Console.Write("<" & TU_TESTSUITE & "." & TU_TESTCASE & "> ")
        Console.WriteLine(msg)
    End Sub

    Public Sub TU_dumpMsg(ByVal msg As String, ByVal m As LibKNDotNet.Message)
        If TU_VERBOSE_ALL = False Then
            Return
        End If

        Dim msgEnt As MessageEntry

        Console.Write("<" & TU_TESTSUITE & "." & TU_TESTCASE & "> ")
        Console.WriteLine(msg)

        For Each msgEnt In m
            Console.WriteLine("        " & msgEnt.Field & ":" & msgEnt.Value)
        Next
    End Sub

    Public Sub TU_dumpCounters()
        If TU_VERBOSE_ALL = False Then
            Return
        End If

        Console.WriteLine("<" & TU_TESTSUITE & "." & TU_TESTCASE & "> Event Counters:")
        Console.WriteLine("        sub Listener OnUpdate     =" & tu_count_subListener_OnUpdate)
        Console.WriteLine("        sub ConnSH   OnConnStatus =" & tu_count_subConnSH_OnConnStatus)
        Console.WriteLine("        sub ReqSH    OnError      =" & tu_count_subReqSH_OnError)
        Console.WriteLine("        sub ReqSH    OnSuccess    =" & tu_count_subReqSH_OnSuccess)
        Console.WriteLine("        pub ConnSH   OnConnStatus =" & tu_count_pubConnSH_OnConnStatus)
        Console.WriteLine("        pub ReqSH    OnError      =" & tu_count_pubReqSH_OnError)
        Console.WriteLine("        pub ReqSH    OnSuccess    =" & tu_count_pubReqSH_OnSuccess)
    End Sub

    Public Sub TU_resetCounters()
        tu_count_subListener_OnUpdate = 0
        tu_count_subConnSH_OnConnStatus = 0
        tu_count_subReqSH_OnError = 0
        tu_count_subReqSH_OnSuccess = 0
        tu_count_pubConnSH_OnConnStatus = 0
        tu_count_pubReqSH_OnError = 0
        tu_count_pubReqSH_OnSuccess = 0
    End Sub

    Public Function TU_checkCounters(ByVal sL_OU As Integer _
        , ByVal sCSH_OCS As Integer _
        , ByVal sRSH_OE As Integer _
        , ByVal sRSH_OS As Integer _
        , ByVal pCSH_OCS As Integer _
        , ByVal pRSH_OE As Integer _
        , ByVal pRSH_OS As Integer _
    )
        Dim err As Boolean = False
        Dim sw As New IO.StringWriter()
        sw.WriteLine()
        If Not sL_OU = -1 Then
            sw.WriteLine("         sub Listener OnUpdate   Got:" & tu_count_subListener_OnUpdate & " Expected:" & sL_OU)
            If Not sL_OU = tu_count_subListener_OnUpdate Then
                err = True
            End If
        End If
        If Not sCSH_OCS = -1 Then
            sw.WriteLine("         sub ConnSH OnConnStatus Got:" & tu_count_subConnSH_OnConnStatus & " Expected:" & sCSH_OCS)
            If Not sCSH_OCS = tu_count_subConnSH_OnConnStatus Then
                err = True
            End If
        End If
        If Not sRSH_OE = -1 Then
            sw.WriteLine("         sub ReqSH OnError       Got:" & tu_count_subReqSH_OnError & " Expected:" & sRSH_OE)
            If Not sRSH_OE = tu_count_subReqSH_OnError Then
                err = True
            End If
        End If
        If Not sRSH_OS = -1 Then
            sw.WriteLine("         sub ReqSH OnSuccess     Got:" & tu_count_subReqSH_OnSuccess & " Expected:" & sRSH_OS)
            If Not sRSH_OS = tu_count_subReqSH_OnSuccess Then
                err = True
            End If
        End If
        If Not pCSH_OCS = -1 Then
            sw.WriteLine("         pub ConnSH OnConnStatus Got:" & tu_count_pubConnSH_OnConnStatus & " Expected:" & pCSH_OCS)
            If Not pCSH_OCS = tu_count_pubConnSH_OnConnStatus Then
                err = True
            End If
        End If
        If Not pRSH_OE = -1 Then
            sw.WriteLine("         pub ReqSH OnError       Got:" & tu_count_pubReqSH_OnError & " Expected:" & pRSH_OE)
            If Not pRSH_OE = tu_count_pubReqSH_OnError Then
                err = True
            End If
        End If
        If Not pRSH_OS = -1 Then
            sw.WriteLine("         pub ReqSH OnSuccess     Got:" & tu_count_pubReqSH_OnSuccess & " Expected:" & pRSH_OS)
            If Not pRSH_OS = tu_count_pubReqSH_OnSuccess Then
                err = True
            End If
        End If

        If err Then
            Return sw.ToString
        Else
            Return TU_OK
        End If
    End Function

    Public Function tu_toString(ByVal al As ArrayList)
        Dim sw As New IO.StringWriter()
        Dim val As String
        For Each val In al
            sw.Write(val & " ")
        Next
        Return sw.ToString()
    End Function

End Module
