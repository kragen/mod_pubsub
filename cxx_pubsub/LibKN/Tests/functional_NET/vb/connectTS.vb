Imports NUnit.Framework
Imports LibKNDotNet

<TestFixture()> _
Public Class connectTS
    <SetUp()> _
    Public Sub Init()
        TU_INIT_TESTSUITE("connectTS")
    End Sub

    <TearDown()> _
    Public Sub Deinit()
    End Sub

    <Test()> _
    Public Sub testConstructor()
        TU_INIT_TESTCASE("testConstructor")

        Dim c1 As New LibKNDotNet.Connector()

        Assertion.Assert("Not c1.IsConnected()", Not c1.IsConnected())
    End Sub

    <Test()> _
    Public Sub testConnect()
        TU_INIT_TESTCASE("testConnect")

        Dim c1 As New LibKNDotNet.Connector()
        Dim p1 As New LibKNDotNet.Parameters()
        Dim b As Boolean

        b = c1.Open(p1)
        Assertion.Assert("b = c1.Open(p1) = FALSE", b = False)
        Assertion.Assert("c1.IsConnected() = False", c1.IsConnected() = False)

        p1.ServerUrl = TU_getServer()
        b = c1.Open(p1)
        Assertion.Assert("b = c1.Open(p1) = FALSE", b)
        Assertion.Assert("c1.IsConnected() = True", c1.IsConnected())
    End Sub

    ' Open() and IsConnected() will return true in .NET because
    ' not connection is established until an actual server request is made.
    ' Then IsConnected() should return true or false is warrented.
    <Test()> _
    Public Sub testConnectedBad()
        TU_INIT_TESTCASE("testConnectedBad")

        Dim c1 As New LibKNDotNet.Connector()
        Dim p1 As New LibKNDotNet.Parameters()
        Dim b As Boolean

        p1.ServerUrl = TU_getMissingServer()
        TU_dump("Missing server=" & TU_getMissingServer())
        b = c1.Open(p1)

        Assertion.Assert("b", b)
        Assertion.Assert("c1.IsConnected()", c1.IsConnected())
    End Sub

    <Test()> _
    Public Sub testPub()
        TU_INIT_TESTCASE("testPub")
        TU_resetCounters()

        Dim c1 As New LibKNDotNet.Connector()
        Dim p1 As New LibKNDotNet.Parameters()
        Dim prsh As New TU_PubRequestStatusHandler()
        Dim b As Boolean

        p1.ServerUrl = TU_getServer()
        b = c1.Open(p1)
        Assertion.Assert("b = c1.Open(p1) = True", b)
        Assertion.Assert("c1.IsConnected() = True", c1.IsConnected())

        Dim m1 As New LibKNDotNet.Message()
        m1.Set("do_method", "notify")
        m1.Set("kn_to", "/what/knchat/messages")
        m1.Set("nickname", "abc")
        m1.Set("kn_response_format", "simple")


        Dim counter As Integer
        For counter = 1 To 10
            m1.Set("kn_payload", "Hello-" & counter)
            b = c1.Publish(m1, prsh)
        Next

        TU_dumpCounters()
        'pubReqSH    OnSuccess    : 10 pub connections ----------------------------+
        'pubReqSH    OnError      : --------------------------------------------+  |
        'pubConnSH   OnConnStatus : No conn status handler ------------------+  |  |
        'subReqSH    OnSuccess    : No sub -------------------------------+  |  |  |
        'subReqSH    OnError      : No sub ----------------------------+  |  |  |  |
        'subConnSH   OnConnStatus : No sub -------------------------+  |  |  |  |  |
        'subListener OnUpdate     : No sub ----------------------+  |  |  |  |  |  |
        Dim counterResult1 As String = TestUtil.TU_checkCounters(0, 0, 0, 0, 0, 0, 10)
        Assertion.Assert(counterResult1, counterResult1 = TU_OK)

    End Sub


End Class
