namespace NUnit.Tests
{
	using TestUtil;
	using System;
	using System.Collections;
	using NUnit.Framework;
	using LibKNDotNet;
	
	[TestFixture]
	public class messageTS
	{

		[SetUp] public void Init()
		{
			TestUtil.INIT_TESTSUITE("messageTS");
		}

		[TearDown] public void Deinit()
		{
		}

		[Test] public void testConstructor()
		{
			TestUtil.INIT_TESTCASE("testConstructor");
			
			Message m1 = new Message();
			
			Assertion.Assert("m1.IsEmpty()", m1.IsEmpty());
		}

		[Test] public void testEmpty()
		{
			TestUtil.INIT_TESTCASE("testEmpty");

			Message m1 = new Message();
			m1.Set("field1", "Value1");
			m1.Set("field2", "Value2");
			m1.Set("field3", "Value3");
			m1.Set("field4", "Value4");
			TestUtil.dumpMsg("Msg: m1", m1);

			m1.Empty();
			TestUtil.dumpMsg("Msg: m1", m1);
			Assertion.Assert("m1.IsEmpty()", m1.IsEmpty());
		}

		[Test] public void testCopy()
		{
			TestUtil.INIT_TESTCASE("testCopy");

			Message m1 = new Message();
			m1.Set("field1", "Value1");
			m1.Set("field2", "Value2");
			m1.Set("field3", "Value3");
			m1.Set("field4", "Value4");
			TestUtil.dumpMsg("Msg: m1", m1);

			Message m2 = new Message();
			m2.Copy(m1);
			TestUtil.dumpMsg("Msg: m1", m1);
			TestUtil.dumpMsg("Msg: m2", m2);

			Assertion.Assert("m1.IsEqual(m2)", m1.IsEqual(m2));
		}

		[Test] public void testEquals()
		{
			TestUtil.INIT_TESTCASE("testEquals");

			Message m1 = new Message();
			m1.Set("field1", "Value1");
			m1.Set("field2", "Value2");
			m1.Set("field3", "Value3");
			m1.Set("field4", "Value4");
			TestUtil.dumpMsg("Msg: m1", m1);

			Message m2 = new Message();
			m2.Set("field1", "Value1");
			m2.Set("field2", "Value2");
			m2.Set("field3", "Value3");
			m2.Set("field4", "Value4");
			TestUtil.dumpMsg("Msg: m2", m2);

			Assertion.Assert("m1.IsEqual(m2)", m1.IsEqual(m2));

			m2.Set("field5", "value5");
			TestUtil.dumpMsg("Msg: m2", m2);
			Assertion.Assert("!m1.IsEqual(m2)", !m1.IsEqual(m2));
		}

		[Test] public void testGet()
		{
			TestUtil.INIT_TESTCASE("testGet");

			Message m1 = new Message();
			m1.Set("field1", "Value1");
			m1.Set("field2", "Value2");
			m1.Set("field3", "Value3");
			m1.Set("field4", "Value4");
			TestUtil.dumpMsg("Msg: m1", m1);

			string str1 = "";
			bool result;
			result = m1.Get("field1", ref str1);
			TestUtil.dump("str1=" + str1);

			Assertion.Assert("result", result);
			Assertion.Assert("str1.Equals(\"Value1\")", str1.Equals("Value1"));
			Assertion.Assert("!str1.Equals(\"Value1\")", !str1.Equals("Value2"));

			result = m1.Get("field5", ref str1);
			Assertion.Assert("!result", !result);
		}

		[Test] public void testSerialization()
		{
			TestUtil.INIT_TESTCASE("testSerialization");

			Message m1 = new Message();
			m1.Set("field1", "Value1");
			m1.Set("field2", "Value2  ");
			m1.Set("field3", "  Value3");
			m1.Set("field4", "  Value4  ");
			TestUtil.dumpMsg("Msg: m1", m1);

			string str1 = m1.GetAsSimpleFormat();
			TestUtil.dump("str1=" + str1);

			Message m2 = new Message();
			bool result = m2.InitFromSimple(str1);
			Assertion.Assert("result", result);
			TestUtil.dumpMsg("Msg: m2", m2);
			
			string val1 = "";
			string val2 = "";
			string val3 = "";
			string val4 = "";
			string val5 = "";
			result = m2.Get("field1", ref val1);
			Assertion.Assert("result", result);
			result = m2.Get("field2", ref val2);
			Assertion.Assert("result", result);
			result = m2.Get("field3", ref val3);
			Assertion.Assert("result", result);
			result = m2.Get("field4", ref val4);
			Assertion.Assert("result", result);
			result = m2.Get("field5", ref val5);
			Assertion.Assert("!result", !result);

			Assertion.Assert("val1 = Value1", val1 == "Value1");
			Assertion.Assert("val2 = Value2", val2 == "Value2  ");
			Assertion.Assert("val3 = Value3", val3 == "  Value3");
			Assertion.Assert("val4 = Value4", val4 == "  Value4  ");

			Assertion.Assert("m2.IsEqual(m1)", m2.IsEqual(m1));
		}

		[Test] public void testGetType()
		{
			TestUtil.INIT_TESTCASE("testGetType");

			Message m1 = new Message();
			Type type = m1.GetType();
			TestUtil.dump("Type=" + type.ToString());
			Assertion.Assert("type.ToString() = LibKNDotNet.Message", type.ToString() == "LibKNDotNet.Message");
		}

		[Test] public void testGetEnumerator()
		{
			TestUtil.INIT_TESTCASE("testGetEnumerator");

			Message m1 = new Message();
			m1.Set("Field1", "Value1");
			m1.Set("Field2", "Value2");
			m1.Set("Field3", "Value3");
			m1.Set("Field4", "Value4");

			IEnumerator ienum = m1.GetEnumerator();

			MessageEntry msge;
			int count = 0;
			ArrayList fields = new ArrayList();
			ArrayList values = new ArrayList();
			while(ienum.MoveNext())
			{
				count++;
				msge = (MessageEntry)ienum.Current;
				TestUtil.dump("       " + msge.Field + ":" + msge.Value);
				fields.Add(msge.Field);
				values.Add(msge.Value);
			}
			
			Assertion.Assert("fields[0] == field1", (string)fields[0] == "Field1");
			Assertion.Assert("fields[1] == field2", (string)fields[1] == "Field2");
			Assertion.Assert("fields[2] == field3", (string)fields[2] == "Field3");
			Assertion.Assert("fields[3] == field4", (string)fields[3] == "Field4");
			Assertion.Assert("values[0] == value1", (string)values[0] == "Value1");
			Assertion.Assert("values[1] == value2", (string)values[1] == "Value2");
			Assertion.Assert("values[2] == value3", (string)values[2] == "Value3");
			Assertion.Assert("values[3] == value4", (string)values[3] == "Value4");
			Assertion.Assert("count = 4", count == 4);
		}

	}
}
