Imports NUnit.Framework
Imports LibKNDotNet

<TestFixture()> _
Public Class messageTS
    <SetUp()> _
    Public Sub Init()
        TU_INIT_TESTSUITE("messageTS")
        ' Initialization code here.
    End Sub

    <TearDown()> _
    Public Sub Deinit()
        ' De-initialization code here.
    End Sub

    <Test()> _
    Public Sub testConstructor()
        TU_INIT_TESTCASE("testConstructor")

        Dim m1 As New LibKNDotNet.Message()

        Assertion.Assert("m1.IsEmpty()", m1.IsEmpty())
    End Sub

    <Test()> _
    Public Sub testEmpty()
        TU_INIT_TESTCASE("testEmpty")

        Dim m1 As New LibKNDotNet.Message()
        m1.Set("field1", "Value1")
        m1.Set("field2", "Value2")
        m1.Set("field3", "Value3")
        m1.Set("field4", "Value4")
        TU_dumpMsg("Msg: m1", m1)

        m1.Empty()
        TU_dumpMsg("Msg: m1", m1)
        Assertion.Assert("m1.IsEmpty()", m1.IsEmpty())
    End Sub

    <Test()> _
    Public Sub testCopy()
        TU_INIT_TESTCASE("testCopy")

        Dim m1 As New LibKNDotNet.Message()
        m1.Set("field1", "Value1")
        m1.Set("field2", "Value2")
        m1.Set("field3", "Value3")
        m1.Set("field4", "Value4")
        TU_dumpMsg("Msg: m1", m1)

        Dim m2 As New LibKNDotNet.Message()
        m2.Copy(m1)
        TU_dumpMsg("Msg: m1", m1)
        TU_dumpMsg("Msg: m2", m2)

        Assertion.Assert("m1.IsEqual(m2)", m1.IsEqual(m2))
    End Sub

    <Test()> _
    Public Sub testEquals()
        TU_INIT_TESTCASE("testEquals")

        Dim m1 As New LibKNDotNet.Message()
        m1.Set("field1", "Value1")
        m1.Set("field2", "Value2")
        m1.Set("field3", "Value3")
        m1.Set("field4", "Value4")
        TU_dumpMsg("Msg: m1", m1)

        Dim m2 As New LibKNDotNet.Message()
        m2.Set("field1", "Value1")
        m2.Set("field2", "Value2")
        m2.Set("field3", "Value3")
        m2.Set("field4", "Value4")
        TU_dumpMsg("Msg: m2", m2)

        Assertion.Assert("m1.IsEqual(m2)", m1.IsEqual(m2))

        m2.Set("field5", "value5")
        TU_dumpMsg("Msg: m2", m2)
        Assertion.Assert("Not m1.IsEqual(m2)", Not m1.IsEqual(m2))
    End Sub

    <Test()> _
    Public Sub testGet()
        TU_INIT_TESTCASE("testGet")

        Dim m1 As New LibKNDotNet.Message()
        m1.Set("field1", "Value1")
        m1.Set("field2", "Value2")
        m1.Set("field3", "Value3")
        m1.Set("field4", "Value4")
        TU_dumpMsg("Msg: m1", m1)

        Dim str1 As String
        Dim result As Boolean
        result = m1.Get("field1", str1)
        TU_dump("str1=" & str1)

        Assertion.Assert("result", result)
        Assertion.Assert("str1 = Value1", str1 = "Value1")
        Assertion.Assert("Not str1 = Value1", Not str1 = "Value2")

        result = m1.Get("field5", str1)
        Assertion.Assert("Not result", Not result)
    End Sub

    <Test()> _
    Public Sub testSerialization()
        TU_INIT_TESTCASE("testSerialization")

        Dim m1 As New LibKNDotNet.Message()
        m1.Set("field1", "Value1")
        m1.Set("field2", "Value2  ")
        m1.Set("field3", "  Value3")
        m1.Set("field4", "  Value4  ")
        TU_dumpMsg("Msg: m1", m1)

        Dim str1 As String
        str1 = m1.GetAsSimpleFormat()
        TU_dump("str1=" & str1)

        Dim m2 As New LibKNDotNet.Message()
        Dim result As Boolean
        result = m2.InitFromSimple(str1)
        Assertion.Assert("result = True", result)
        TU_dumpMsg("Msg: m2", m2)

        Dim val1 As String
        Dim val2 As String
        Dim val3 As String
        Dim val4 As String
        Dim val5 As String
        result = m2.Get("field1", val1)
        Assertion.Assert("result", result)
        result = m2.Get("field2", val2)
        Assertion.Assert("result", result)
        result = m2.Get("field3", val3)
        Assertion.Assert("result", result)
        result = m2.Get("field4", val4)
        Assertion.Assert("result", result)
        result = m2.Get("field5", val5)
        Assertion.Assert("Not result", Not result)

        Assertion.Assert("val1 = Value1", val1 = "Value1")
        Assertion.Assert("val2 = Value2", val2 = "Value2  ")
        Assertion.Assert("val3 = Value3", val3 = "  Value3")
        Assertion.Assert("val4 = Value4", val4 = "  Value4  ")

        Assertion.Assert("m2.IsEqual(m1)", m2.IsEqual(m1))

    End Sub

    <Test()> _
    Public Sub testGetType()
        TU_INIT_TESTCASE("testGetType")

        Dim m1 As New LibKNDotNet.Message()
        Dim type As Type = m1.GetType()
        TU_dump("Type=" & type.ToString())
        Assertion.Assert("type.ToString() = LibKNDotNet.Message", type.ToString() = "LibKNDotNet.Message")
    End Sub

    <Test()> _
    Public Sub testGetEnumerator()
        TU_INIT_TESTCASE("testGetEnumerator")

        Dim m1 As New LibKNDotNet.Message()
        m1.Set("Field1", "Value1")
        m1.Set("Field2", "Value2")
        m1.Set("Field3", "Value3")
        m1.Set("Field4", "Value4")

        Dim ienum As Collections.IEnumerator = m1.GetEnumerator()

        Dim msge As MessageEntry
        Dim count As Integer = 0
        Dim fields(4) As String
        Dim values(4) As String
        Do While ienum.MoveNext()
            count = count + 1
            msge = ienum.Current()
            Dim field As New String(msge.Field)
            Dim value As New String(msge.Value)
            TU_dump("       " & field & ":" & value)
            fields(count) = field
            values(count) = value
        Loop

        Assertion.Assert("fields(1) = field1", fields(1) = "Field1")
        Assertion.Assert("fields(2) = field2", fields(2) = "Field2")
        Assertion.Assert("fields(3) = field3", fields(3) = "Field3")
        Assertion.Assert("fields(4) = field4", fields(4) = "Field4")
        Assertion.Assert("values(1) = value1", values(1) = "Value1")
        Assertion.Assert("values(2) = value2", values(2) = "Value2")
        Assertion.Assert("values(3) = value3", values(3) = "Value3")
        Assertion.Assert("values(4) = value4", values(4) = "Value4")
        Assertion.Assert("count = 4", count = 4)

    End Sub

End Class
