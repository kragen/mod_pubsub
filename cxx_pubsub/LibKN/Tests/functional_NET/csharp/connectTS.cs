namespace NUnit.Tests
{
	using TestUtil;
	using System;
	using System.Collections;
	using NUnit.Framework;
	using LibKNDotNet;
	
	[TestFixture] 
	public class connectTS
	{
		[SetUp] public void Init()
		{
			TestUtil.INIT_TESTSUITE("connectTS");
		}

		[TearDown] public void Deinit()
		{
		}

		[Test] public void testConstructor()
		{
			TestUtil.INIT_TESTCASE("testConstructor");
			
			Connector c1 = new Connector();
			
			Assertion.Assert("!c1.IsConnected()", !c1.IsConnected());
		}

		[Test] public void testConnect()
		{
			TestUtil.INIT_TESTCASE("testConnect");
	
			Connector c1 = new Connector();
			Parameters p1 = new Parameters();
			bool b;

			b = c1.Open(p1);
			Assertion.Assert("!b", !b);
			Assertion.Assert("!c1.IsConnected()", !c1.IsConnected());

	        p1.ServerUrl = TestUtil.getServer();
		    b = c1.Open(p1);
			Assertion.Assert("b", b);
			Assertion.Assert("c1.IsConnected()", c1.IsConnected());
		}

		/* Open() and IsConnected() will return true in .NET because
		 * not connection is established until an actual server request is made.
		 * Then IsConnected() should return true or false is warrented.
		 */
		[Test] public void testConnectedBad()
		{
			TestUtil.INIT_TESTCASE("testConnectedBad");
	
			Connector c1 = new Connector();
			Parameters p1 = new Parameters();
			bool b;

			p1.ServerUrl = TestUtil.getMissingServer();
			b = c1.Open(p1);
			Assertion.Assert("b", b);
			Assertion.Assert("c1.IsConnected()", c1.IsConnected());
		}

		[Test] public void testPub()
		{
			TestUtil.INIT_TESTCASE("testPub");
			TestUtil.resetCounters();
	
			Connector c1 = new Connector();
			Parameters p1 = new Parameters();
			TestUtil.PubRequestStatusHandler prsh = new TestUtil.PubRequestStatusHandler();
			bool b;

			p1.ServerUrl = TestUtil.getServer();
			b = c1.Open(p1);
			Assertion.Assert("b", b);
			Assertion.Assert("c1.IsConnected()", c1.IsConnected());

			Message m1 = new Message();
			m1.Set("do_method", "notify");
			m1.Set("kn_to", "/what/knchat/messages");
			m1.Set("nickname", "abc");
			m1.Set("kn_response_format", "simple");

			for(int i = 1; i <= 10; i++)
			{
				m1.Set("kn_payload", "Hello-" + i);
				b = c1.Publish(m1, prsh);
			}

			TestUtil.dumpCounters();
			//pubReqSH    OnSuccess    : 10 pub connections -----------------+
			//pubReqSH    OnError      : ---------------------------------+  |
			//pubConnSH   OnConnStatus : No conn status handler -------+  |  |
			//subReqSH    OnSuccess    : No sub --------------------+  |  |  |
			//subReqSH    OnError      : No sub -----------------+  |  |  |  |
			//subConnSH   OnConnStatus : No sub --------------+  |  |  |  |  |
			//subListener OnUpdate     : No sub -----------+  |  |  |  |  |  |
			string counterResult1 = TestUtil.checkCounters(0, 0, 0, 0, 0, 0, 10);
			Assertion.Assert(counterResult1, counterResult1 == TestUtil.TU_OK);
		}
	}
}
