Imports NUnit.Framework
Imports LibKNDotNet

<TestFixture()> _
Public Class connectTS3
    <SetUp()> _
    Public Sub Init()
        TU_INIT_TESTSUITE("connectTS3")
        ' Initialization code here.
    End Sub

    <TearDown()> _
    Public Sub Deinit()
        ' De-initialization code here.
    End Sub

    '* Test that GetRouteId(), SubscribeRouteId() can substiture for Subscribe().
    '* HOWEVER, they are not supplied (or necessary) for VB-NET
    <Test()> _
    Public Sub testPubSubRouteID()
        TU_INIT_TESTCASE("testPubSubRouteID")
        Assertion.Assert(True)
    End Sub

    ' Test Subscribe() to something that does not exist.
    ' It should work as normal but the rid = 0.
    ' Tests bug1015.
    <Test()> _
    Public Sub testSubMissingServer()
        TU_INIT_TESTCASE("testSubMissingServer")
        TU_resetCounters()

        Dim topic As String = "/what/test/messages"

        ' Start the subscriber.
        Dim subConn As New LibKNDotNet.Connector()
        Dim subListener As New TestUtil.TU_SubListener()
        Dim subReqSH As New TestUtil.TU_SubRequestStatusHandler()
        Dim subITParams As New LibKNDotNet.Parameters()

        ' This is only necessary to get subConnSH_OnConnStatus
        Dim subConnSH As New TestUtil.TU_SubConnStatusHandler()
        subConn.AddConnectionStatusHandler(subConnSH)

        subITParams.ServerUrl = TestUtil.TU_getMissingServer()
        TU_dump("Missing server=" & TU_getMissingServer())
        If subConn.Open(subITParams) = False Then
            Assertion.Fail("Open(subITParams) failed.")
        End If

        Dim msg As New LibKNDotNet.Message()
        Dim rid As String = ""
        rid = subConn.Subscribe(topic, subListener, msg, subReqSH)
        TU_dump("rid=" & rid)
        Assertion.Assert("rid.Length() = 0", rid.Length() = 0)

        ' Unsubscribe should return true.
        Dim b As Boolean = False
        b = subConn.Unsubscribe(rid, subReqSH)
        Assertion.Assert("subConn.Unsubscribe(rid, subReqSH) = True", b = True)

        subConn.Close()
        subConn.RemoveConnectionStatusHandler(subConnSH)

        TU_dumpCounters()
        'pubReqSH    OnSuccess    : 10 pub connections ----------------------------+
        'pubReqSH    OnError      : --------------------------------------------+  |
        'pubConnSH   OnConnStatus : No conn status handler ------------------+  |  |
        'subReqSH    OnSuccess    : No sub -------------------------------+  |  |  |
        'subReqSH    OnError      : Subscribe() -----------------------+  |  |  |  |
        'subConnSH   OnConnStatus : 1st Subscribe() ----------------+  |  |  |  |  |
        'subListener OnUpdate     : No sub ----------------------+  |  |  |  |  |  |
        Dim counterResult1 As String = TestUtil.TU_checkCounters(0, 1, 1, 0, 0, 0, 0)
        Assertion.Assert(counterResult1, counterResult1 = TU_OK)

    End Sub

End Class
