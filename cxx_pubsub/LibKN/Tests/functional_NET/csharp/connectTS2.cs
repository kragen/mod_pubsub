namespace NUnit.Tests
{
	using TestUtil;
	using System;
	using System.Collections;
	using NUnit.Framework;
	using LibKNDotNet;
	
	[TestFixture] 
	public class connectTS2
	{
		[SetUp] public void Init()
		{
			TestUtil.INIT_TESTSUITE("connectTS2");
		}

		[TearDown] public void Deinit()
		{
		}

		[Test] public void testPubCloseReOpen()
		{
			TestUtil.INIT_TESTCASE("testPubCloseReOpen");
			TestUtil.resetCounters();

			string serverUrl = TestUtil.getServer();
			string topic = "/what/Tests/functional_NET/csharp/"
				+ TestUtil.getTestsuiteName() + "/" + TestUtil.getTestcaseName();
	
			Connector pubConn = new Connector();
			TestUtil.PubRequestStatusHandler pubReqSH = new TestUtil.PubRequestStatusHandler();
			Parameters pubITParams = new Parameters();
			Message pubMsg = new Message();

			// This is only necessary to get pubConnSH_OnConnStatus
			TestUtil.PubConnStatusHandler pubConnSH = new TestUtil.PubConnStatusHandler();
			pubConn.AddConnectionStatusHandler(pubConnSH);

			pubITParams.ServerUrl = serverUrl;
			if( ! pubConn.Open(pubITParams) )
			{
				Assertion.Fail("tp=1 Open(pubITParams) failed.");
			}

			Assertion.Assert("tp=2", pubConn.EnsureConnected());

			// Publish one message
			pubMsg.Set("do_method", "notify");
			pubMsg.Set("kn_to", topic);
			pubMsg.Set("kn_payload", "Hello-X");
			pubMsg.Set("nickname", "abc");
			pubMsg.Set("kn_response_format", "simple");

			Assertion.Assert("tp=3", pubConn.Publish(pubMsg, pubReqSH));
			Assertion.Assert("tp=4", pubConn.IsConnected());
			Assertion.Assert("tp=5", pubConn.EnsureConnected());

			Assertion.Assert("tp=6", pubConn.Close());
			Assertion.Assert("tp=7", !pubConn.IsConnected());

			Assertion.Assert("tp=8", pubConn.Publish(pubMsg, pubReqSH)); //Publish will re open.
			Assertion.Assert("tp=9", pubConn.IsConnected());

			Assertion.Assert("tp=10", pubConn.Close());
			Assertion.Assert("tp=11", !pubConn.IsConnected());

			Assertion.Assert("tp=12", pubConn.EnsureConnected()); //EnsureConnected will re open.
			Assertion.Assert("tp=13", pubConn.IsConnected());

			//Close and Publish one more time to make sure really working
			Assertion.Assert("tp=14", pubConn.Close());
			Assertion.Assert("tp=15", !pubConn.IsConnected());

			Assertion.Assert("tp=16", pubConn.Publish(pubMsg, pubReqSH)); //Publish will re open.
			Assertion.Assert("tp=17", pubConn.IsConnected());

			TestUtil.dumpCounters();
			//pubReqSH    OnSuccess    : 3 pub connections ------------------+
			//pubReqSH    OnError      : ---------------------------------+  |
			//pubConnSH   OnConnStatus : 3 pub connections ------------+  |  |
			//subReqSH    OnSuccess    : No sub --------------------+  |  |  |
			//subReqSH    OnError      : No sub -----------------+  |  |  |  |
			//subConnSH   OnConnStatus : No sub --------------+  |  |  |  |  |
			//subListener OnUpdate     : No sub -----------+  |  |  |  |  |  |
			string counterResult1 = TestUtil.checkCounters(0, 0, 0, 0, 3, 0, 3);
			Assertion.Assert("tp=18"+counterResult1, counterResult1 == TestUtil.TU_OK);
		}

		/*
		 * Test simple Publish() Subscribe(). Pub 1 message then do Sub then pub 5 messages.
		 * Sub should see 5 messages.
		 */
		[Test] public void testPubSub()
		{
			TestUtil.INIT_TESTCASE("testPubSub");
			
		}

		/*
		 *  Low priority to test. Tested in C++.
		 */
		[Test] public void testRemoveCSHs()
		{
			TestUtil.INIT_TESTCASE("testRemoveCSHs");
			
		}

		/*
		 *  Low priority to test. Tested in C++.
		 */
		[Test] public void testOverrideOnStatus()
		{
			TestUtil.INIT_TESTCASE("testOverrideOnStatus");
			
		}

		[Test] public void testUnsubscribeReSubscribe()
		{
			TestUtil.INIT_TESTCASE("testUnsubscribeReSubscribe");
			
		}

	}
}
