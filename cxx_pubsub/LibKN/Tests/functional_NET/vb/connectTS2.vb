Imports NUnit.Framework
Imports LibKNDotNet

<TestFixture()> _
Public Class connectTS2
    <SetUp()> _
    Public Sub Init()
        TU_INIT_TESTSUITE("connectTS2")
    End Sub

    <TearDown()> _
    Public Sub Deinit()
    End Sub

    ' * Test that Close() closes and Publish re-opens.
    <Test()> _
    Public Sub testPubCloseReOpen()
        TU_INIT_TESTCASE("testPubCloseReOpen")
        TU_resetCounters()

        Dim serverUrl As String = TestUtil.TU_getServer
        Dim topic As String = "/what/Tests/functional_NET/vb/" _
            & TU_getTestsuiteName() & "/" & TU_getTestcaseName()

        Dim pubConn As New LibKNDotNet.Connector()
        Dim pubReqSH As New TestUtil.TU_PubRequestStatusHandler()
        Dim pubITParams As New LibKNDotNet.Parameters()
        Dim pubMsg As New LibKNDotNet.Message()

        ' This is only necessary to get pubConnSH_OnConnStatus
        Dim pubConnSH As New TestUtil.TU_PubConnStatusHandler()
        pubConn.AddConnectionStatusHandler(pubConnSH)

        pubITParams.ServerUrl = serverUrl
        If Not pubConn.Open(pubITParams) Then
            Assertion.Fail("tp=1 Open(pubITParams) failed.")
        End If

        Assertion.Assert("tp=2 pubConn.EnsureConnected()", pubConn.EnsureConnected())

        ' Publish one message
        pubMsg.Set("do_method", "notify")
        pubMsg.Set("kn_to", topic)
        pubMsg.Set("kn_payload", "Hello-X")
        pubMsg.Set("nickname", "abc")
        pubMsg.Set("kn_response_format", "simple")

        Assertion.Assert("tp=3", pubConn.Publish(pubMsg, pubReqSH))
        Assertion.Assert("tp=4", pubConn.IsConnected())
        Assertion.Assert("tp=5", pubConn.EnsureConnected())

        Assertion.Assert("tp=6", pubConn.Close())
        Assertion.Assert("tp=7", pubConn.IsConnected() = False)

        Assertion.Assert("tp=8", pubConn.Publish(pubMsg, pubReqSH)) 'Publish will re open.
        Assertion.Assert("tp=9", pubConn.IsConnected())

        Assertion.Assert("tp=10", pubConn.Close())
        Assertion.Assert("tp=11", pubConn.IsConnected() = False)

        Assertion.Assert("tp=12", pubConn.EnsureConnected()) 'EnsureConnected will re open.
        Assertion.Assert("tp=13", pubConn.IsConnected())

        'Close and Publish one more time to make sure really working
        Assertion.Assert("tp=14", pubConn.Close())
        Assertion.Assert("tp=15", pubConn.IsConnected() = False)

        Assertion.Assert("tp=16", pubConn.Publish(pubMsg, pubReqSH)) 'Publish will re open.
        Assertion.Assert("tp=17", pubConn.IsConnected())

        TU_dumpCounters()
        'pubReqSH    OnSuccess    : 3 pub connections -----------------------------+
        'pubReqSH    OnError      : --------------------------------------------+  |
        'pubConnSH   OnConnStatus : 3 pub connections -----------------------+  |  |
        'subReqSH    OnSuccess    : No sub -------------------------------+  |  |  |
        'subReqSH    OnError      : No sub ----------------------------+  |  |  |  |
        'subConnSH   OnConnStatus : No sub -------------------------+  |  |  |  |  |
        'subListener OnUpdate     : No sub ----------------------+  |  |  |  |  |  |
        Dim counterResult1 As String = TestUtil.TU_checkCounters(0, 0, 0, 0, 3, 0, 3)
        Assertion.Assert("tp=18" & counterResult1, counterResult1 = TU_OK)

    End Sub

    ' * Test simple Publish() Subscribe(). Pub 1 message then do Sub then pub 5 messages.
    ' * Sub should see 5 messages.
    <Test()> _
    Public Sub testPubSub()
        TU_INIT_TESTCASE("testPubSub")
        TU_resetCounters()

        Dim serverUrl As String = TestUtil.TU_getServer
        Dim topic As String = "/what/test/messages"

        Dim pubConn As New LibKNDotNet.Connector()
        Dim pubReqSH As New TestUtil.TU_PubRequestStatusHandler()
        Dim pubITParams As New LibKNDotNet.Parameters()
        Dim pubMsg As New LibKNDotNet.Message()

        ' This is only necessary to get pubConnSH_OnConnStatus
        Dim pubConnSH As New TestUtil.TU_PubConnStatusHandler()
        pubConn.AddConnectionStatusHandler(pubConnSH)

        pubITParams.ServerUrl = serverUrl
        If pubConn.Open(pubITParams) = False Then
            Assertion.Fail("tp=1 Open(pubITParams) failed.")
        End If

        ' Publish one message
        pubMsg.Set("do_method", "notify")
        pubMsg.Set("kn_to", topic)
        pubMsg.Set("kn_payload", "Hello-X")
        pubMsg.Set("nickname", "abc")
        pubMsg.Set("kn_response_format", "simple")

        pubConn.Publish(pubMsg, pubReqSH)

        If pubConn.HasItems() Then
            Assertion.Fail("tp=2 There's something in the queue (it should be empty).")
        End If

        ' Start the subscriber.
        Dim subConn As New LibKNDotNet.Connector()
        Dim subListener As New TestUtil.TU_SubListener()
        Dim subReqSH As New TestUtil.TU_SubRequestStatusHandler()
        Dim subITParams As New LibKNDotNet.Parameters()

        ' This is only necessary to get subConnSH_OnConnStatus
        Dim subConnSH As New TestUtil.TU_SubConnStatusHandler()
        subConn.AddConnectionStatusHandler(subConnSH)

        subITParams.ServerUrl = serverUrl
        If subConn.Open(subITParams) = False Then
            Assertion.Fail("tp=3 Open(subITParams) failed.")
        End If

        Dim rid As String
        Dim msg As New LibKNDotNet.Message()
        rid = subConn.Subscribe(topic, subListener, msg, subReqSH)
        TU_dump("rid=" & rid)

        If rid.Length() = 0 Then
            Assertion.Fail("tp=4 Failed to subscribe.")
        End If

        Dim hello As String = "Hello-"
        Dim counter As Integer
        For counter = 1 To 5
            TU_dump("notify: " & counter)
            pubMsg.Set("kn_payload", hello & counter)
            pubConn.Publish(pubMsg, pubReqSH)
        Next

        TU_dump("Sleep(1000)")
        System.Threading.Thread.Sleep(1000)

        TU_dump("Unsubscribing.")
        If Not subConn.Unsubscribe(rid, subReqSH) Then
            Assertion.Fail("tp=5 Failed to unsubscribe.")
        End If

        If pubConn.HasItems() Then
            Assertion.Fail("tp=6 There's something in the queue (it should be empty).")
        End If

        pubConn.Close()
        subConn.Close()

        subConn.RemoveConnectionStatusHandler(subConnSH)

        TU_dumpCounters()
        'pubReqSH    OnSuccess    : initial pub and 5 updates----------------------+ 
        'pubReqSH    OnError      : --------------------------------------------+  | 
        'pubConnSH   OnConnStatus : 1st pub ---------------------------------+  |  | This test fails intermittantly. Probably need a log to catch what is going on.
        'subReqSH    OnSuccess    : Subscribe(),Unsubscribe() ------------+  |  |  |
        'subReqSH    OnError      : -----------------------------------+  |  |  |  |
        'subConnSH   OnConnStatus : 1st Subscribe() ----------------+  |  |  |  |  | This test fails intermittantly. Probably need a log to catch what is going on.
        'subListener OnUpdate     : 5 updates -------------------+  |  |  |  |  |  | This test fails intermittantly. Need to wait until 5th update with time out. 
        Dim counterResult1 As String = TestUtil.TU_checkCounters(5, 1, 0, 2, 1, 0, 6)
        Assertion.Assert("tp=7" & counterResult1, counterResult1 = TU_OK)
    End Sub

    ' Low priority to test. Tested in C++.
    <Test()> _
    Public Sub testRemoveCSHs()
        TU_INIT_TESTCASE("testRemoveCSHs")
        Assertion.Assert("tp=", True)
    End Sub

    ' Low priority to test. Tested in C++.
    <Test()> _
    Public Sub testOverrideOnStatus()
        TU_INIT_TESTCASE("testOverrideOnStatus")
        Assertion.Assert("tp=", True)
    End Sub

    <Test()> _
    Public Sub testUnsubscribeReSubscribe()
        TU_INIT_TESTCASE("testUnsubscribeReSubscribe")
        TU_resetCounters()

        Dim serverUrl As String = TestUtil.TU_getServer
        Dim topic As String = "/what/test/messages"

        Dim pubConn As New LibKNDotNet.Connector()
        Dim pubReqSH As New TestUtil.TU_PubRequestStatusHandler()
        Dim pubITParams As New LibKNDotNet.Parameters()
        Dim pubMsg As New LibKNDotNet.Message()

        ' This is only necessary to get pubConnSH_OnConnStatus
        Dim pubConnSH As New TestUtil.TU_PubConnStatusHandler()
        pubConn.AddConnectionStatusHandler(pubConnSH)

        pubITParams.ServerUrl = serverUrl
        If pubConn.Open(pubITParams) = False Then
            Assertion.Fail("tp=1 Open(pubITParams) failed.")
        End If

        ' Publish one message to make sure topic will exist.
        pubMsg.Set("do_method", "notify")
        pubMsg.Set("kn_to", topic)
        pubMsg.Set("kn_payload", "Hello-X")
        pubMsg.Set("nickname", "abc")
        pubMsg.Set("kn_response_format", "simple")

        pubConn.Publish(pubMsg, pubReqSH)

        If pubConn.HasItems() Then
            Assertion.Fail("tp=2 There's something in the queue (it should be empty).")
        End If

        ' Start the subscriber.
        Dim subConn As New LibKNDotNet.Connector()
        Dim subListener As New TestUtil.TU_SubListener()
        Dim subReqSH As New TestUtil.TU_SubRequestStatusHandler()
        Dim subITParams As New LibKNDotNet.Parameters()

        ' This is only necessary to get subConnSH_OnConnStatus
        Dim subConnSH As New TestUtil.TU_SubConnStatusHandler()
        subConn.AddConnectionStatusHandler(subConnSH)

        subITParams.ServerUrl = serverUrl
        If subConn.Open(subITParams) = False Then
            Assertion.Fail("tp=3 Open(subITParams) failed.")
        End If

        Dim rid As String
        Dim msg As New LibKNDotNet.Message()
        rid = subConn.Subscribe(topic, subListener, msg, subReqSH)
        TU_dump("rid=" & rid)

        If rid.Length() = 0 Then
            Assertion.Fail("tp=4 Failed to subscribe.")
        End If

        Dim hello As String = "Hello-"
        Dim counter As Integer
        For counter = 0 To 4
            TU_dump("notify: " & counter)
            pubMsg.Set("kn_payload", hello & counter)
            pubConn.Publish(pubMsg, pubReqSH)
        Next

        TU_dump("Sleep(500)")
        System.Threading.Thread.Sleep(500)

        TU_dump("Unsubscribing.")
        If Not subConn.Unsubscribe(rid, subReqSH) Then
            Assertion.Fail("tp=5 Failed to unsubscribe.")
        End If

        For counter = 5 To 9
            TU_dump("notify: " & counter)
            pubMsg.Set("kn_payload", hello & counter)
            pubConn.Publish(pubMsg, pubReqSH)
        Next

        'Now, make sure we can re-subscribe.
        rid = subConn.Subscribe(topic, subListener, msg, subReqSH)
        TU_dump("rid=" & rid)

        If rid.Length() = 0 Then
            Assertion.Fail("tp=6 Failed to subscribe.")
        End If

        For counter = 10 To 14
            TU_dump("notify: " & counter)
            pubMsg.Set("kn_payload", hello & counter)
            pubConn.Publish(pubMsg, pubReqSH)
        Next

        TU_dump("Sleep(500)")
        System.Threading.Thread.Sleep(500)

        TU_dump("Unsubscribing.")
        If Not subConn.Unsubscribe(rid, subReqSH) Then
            Assertion.Fail("tp=7 Failed to unsubscribe.")
        End If

        ' These updates should be missed by the subscriber.
        For counter = 15 To 19
            TU_dump("notify: " & counter)
            pubMsg.Set("kn_payload", hello & counter)
            pubConn.Publish(pubMsg, pubReqSH)
        Next

        If pubConn.HasItems() Then
            Assertion.Fail("tp=8 There's something in the queue (it should be empty).")
        End If

        pubConn.Close()
        subConn.Close()

        subConn.RemoveConnectionStatusHandler(subConnSH)

        TU_dumpCounters()
        'pubReqSH    OnSuccess    : initial pub and 20 updates----------------------+ 
        'pubReqSH    OnError      : ---------------------------------------------+  | 
        'pubConnSH   OnConnStatus : 1st pub ----------------------------------+  |  | 
        'subReqSH    OnSuccess    : Sub() Unsub()  Sub() Unsub() ----------+  |  |  |
        'subReqSH    OnError      : ------------------------------------+  |  |  |  |
        'subConnSH   OnConnStatus : 1st Subscribe() -----------------+  |  |  |  |  | 
        'subListener OnUpdate     : 5 seen 5miss 5 seen 5 miss --+   |  |  |  |  |  |  
        Dim counterResult1 As String = TestUtil.TU_checkCounters(10, 1, 0, 4, 1, 0, 21)
        Assertion.Assert("tp=9" & counterResult1, counterResult1 = TU_OK)
    End Sub

    <Test()> _
    Public Sub testPubSubReOpenSub()
        TU_INIT_TESTCASE("testPubSubReOpenSub")
        TU_resetCounters()

        Dim serverUrl As String = TestUtil.TU_getServer
        Dim topic As String = "/what/test/messages"

        Dim pubConn As New LibKNDotNet.Connector()
        Dim pubReqSH As New TestUtil.TU_PubRequestStatusHandler()
        Dim pubITParams As New LibKNDotNet.Parameters()
        Dim pubMsg As New LibKNDotNet.Message()

        ' This is only necessary to get pubConnSH_OnConnStatus
        Dim pubConnSH As New TestUtil.TU_PubConnStatusHandler()
        pubConn.AddConnectionStatusHandler(pubConnSH)

        pubITParams.ServerUrl = serverUrl
        If pubConn.Open(pubITParams) = False Then
            Assertion.Fail("tp=1 Open(pubITParams) failed.")
        End If

        ' Publish one message to make sure topic will exist.
        pubMsg.Set("do_method", "notify")
        pubMsg.Set("kn_to", topic)
        pubMsg.Set("kn_payload", "Hello-X")
        pubMsg.Set("nickname", "abc")
        pubMsg.Set("kn_response_format", "simple")

        pubConn.Publish(pubMsg, pubReqSH)

        ' Start the subscriber.
        Dim subConn As New LibKNDotNet.Connector()
        Dim subListener As New TestUtil.TU_SubListener()
        Dim subReqSH As New TestUtil.TU_SubRequestStatusHandler()
        Dim subITParams As New LibKNDotNet.Parameters()

        ' This is only necessary to get subConnSH_OnConnStatus
        Dim subConnSH As New TestUtil.TU_SubConnStatusHandler()
        subConn.AddConnectionStatusHandler(subConnSH)

        subITParams.ServerUrl = serverUrl
        If subConn.Open(subITParams) = False Then
            Assertion.Fail("tp=2 Open(subITParams) failed.")
        End If

        Dim rid As String
        Dim msg As New LibKNDotNet.Message()
        rid = subConn.Subscribe(topic, subListener, msg, subReqSH)
        TU_dump("rid=" & rid)

        If rid.Length() = 0 Then
            Assertion.Fail("tp=3 Failed to subscribe.")
        End If

        Dim hello As String = "Hello-"
        Dim counter As Integer
        For counter = 0 To 4
            TU_dump("notify: " & counter)
            pubMsg.Set("kn_payload", hello & counter)
            pubConn.Publish(pubMsg, pubReqSH)
        Next

        TU_dump("Sleep(500)")
        System.Threading.Thread.Sleep(500)

        TU_dump("Unsubscribing.")
        If Not subConn.Unsubscribe(rid, subReqSH) Then
            Assertion.Fail("tp=4 Failed to unsubscribe.")
        End If

        TU_dumpCounters()
        'pubReqSH    OnSuccess    : initial pub and 5 updates----------------------+ 
        'pubReqSH    OnError      : --------------------------------------------+  | 
        'pubConnSH   OnConnStatus : 1st pub ---------------------------------+  |  | This test fails intermittantly. Probably need a log to catch what is going on.
        'subReqSH    OnSuccess    : Sub() Unsub() ------------------------+  |  |  |
        'subReqSH    OnError      : -----------------------------------+  |  |  |  |
        'subConnSH   OnConnStatus : 1st Subscribe() ----------------+  |  |  |  |  | 
        'subListener OnUpdate     : 5 seen ----------------------+  |  |  |  |  |  |  
        Dim counterResult1 As String = TestUtil.TU_checkCounters(5, 1, 0, 2, 1, 0, 6)
        Assertion.Assert("tp=5" & counterResult1, counterResult1 = TU_OK)

        ' Close subscriber. And reset the counters.
        Assertion.Assert("tp=6 subConn.Close()", subConn.Close())
        Assertion.Assert("tp=7 Not subConn.IsConnected", Not subConn.IsConnected)
        TU_resetCounters()

        ' Do some more publishing.
        ' The subscriber should not see them.
        For counter = 5 To 9
            TU_dump("notify: " & counter)
            pubMsg.Set("kn_payload", hello & counter)
            pubConn.Publish(pubMsg, pubReqSH)
        Next

        TU_dumpCounters()
        'pubReqSH    OnSuccess    : initial pub and 5 updates----------------------+ 
        'pubReqSH    OnError      : --------------------------------------------+  | 
        'pubConnSH   OnConnStatus : -----------------------------------------+  |  |
        'subReqSH    OnSuccess    : --------------------------------------+  |  |  |
        'subReqSH    OnError      : -----------------------------------+  |  |  |  |
        'subConnSH   OnConnStatus : --------------------------------+  |  |  |  |  | 
        'subListener OnUpdate     : -----------------------------+  |  |  |  |  |  |  
        Dim counterResult2 As String = TestUtil.TU_checkCounters(0, 0, 0, 0, 0, 0, 5)
        Assertion.Assert("tp=8" & counterResult2, counterResult2 = TU_OK)

        ' EnsureConnected will re-open.
        TU_resetCounters()
        Assertion.Assert("tp=9 Not subConn.IsConnected()", Not subConn.IsConnected())
        Assertion.Assert("tp=10 subConn.EnsureConnected()", subConn.EnsureConnected())
        Assertion.Assert("tp=11 subConn.IsConnected()", subConn.IsConnected())

        ' Do some more publishing.
        ' The subscriber should not see them even though it is reconnected.
        For counter = 10 To 14
            TU_dump("notify: " & counter)
            pubMsg.Set("kn_payload", hello & counter)
            pubConn.Publish(pubMsg, pubReqSH)
        Next

        TU_dumpCounters()
        'pubReqSH    OnSuccess    : initial pub and 5 updates----------------------+ 
        'pubReqSH    OnError      : --------------------------------------------+  | 
        'pubConnSH   OnConnStatus : -----------------------------------------+  |  |
        'subReqSH    OnSuccess    : --------------------------------------+  |  |  |
        'subReqSH    OnError      : -----------------------------------+  |  |  |  |
        'subConnSH   OnConnStatus : --------------------------------+  |  |  |  |  | 
        'subListener OnUpdate     : -----------------------------+  |  |  |  |  |  |  
        Dim counterResult3 As String = TestUtil.TU_checkCounters(0, 0, 0, 0, 0, 0, 5)
        Assertion.Assert("tp=12 " & counterResult3, counterResult3 = TU_OK)

        ' Subscribe one more time to make sure really working
        TU_resetCounters()

        rid = subConn.Subscribe(topic, subListener, msg, subReqSH)
        TU_dump("rid=" & rid)

        If rid.Length() = 0 Then
            Assertion.Fail("tp=13 Failed to subscribe.")
        End If

        For counter = 15 To 19
            TU_dump("notify: " & counter)
            pubMsg.Set("kn_payload", hello & counter)
            pubConn.Publish(pubMsg, pubReqSH)
        Next

        TU_dump("Sleep(500)")
        System.Threading.Thread.Sleep(500)

        TU_dump("Unsubscribing.")
        If Not subConn.Unsubscribe(rid, subReqSH) Then
            Assertion.Fail("tp=14 Failed to unsubscribe.")
        End If

        pubConn.Close()
        subConn.Close()

        subConn.RemoveConnectionStatusHandler(subConnSH)

        TU_dumpCounters()
        'pubReqSH    OnSuccess    : 5 updates--------------------------------------+ 
        'pubReqSH    OnError      : --------------------------------------------+  | 
        'pubConnSH   OnConnStatus : -----------------------------------------+  |  | 
        'subReqSH    OnSuccess    : Sub() Unsub() ------------------------+  |  |  |
        'subReqSH    OnError      : -----------------------------------+  |  |  |  |
        'subConnSH   OnConnStatus : Sub() --------------------------+  |  |  |  |  | 
        'subListener OnUpdate     : 5 seen ----------------------+  |  |  |  |  |  |  
        Dim counterResult4 As String = TestUtil.TU_checkCounters(5, 1, 0, 2, 0, 0, 5)
        Assertion.Assert("tp=15" & counterResult4, counterResult4 = TU_OK)
    End Sub

End Class
