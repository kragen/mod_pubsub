#include "stdafx.h"
#include "globals.h"
#include "testutil.h"
#include <LibKN\Connector.h>
#include <LibKN\StrUtil.h>

#define VERBOSE

/**
 *
 */

class ConnectorTest2 : public CPPUNIT_NS::TestFixture
{

	CPPUNIT_TEST_SUITE(ConnectorTest2);
	CPPUNIT_TEST(testPubCloseReOpen);
	CPPUNIT_TEST(testPubSub);
	CPPUNIT_TEST(testRemoveCSHs);
	CPPUNIT_TEST(testOverrideOnStatus);
	CPPUNIT_TEST(testUnsubscribeReSubscribe);
	CPPUNIT_TEST(testPubSubReOpenSub);
	CPPUNIT_TEST_SUITE_END();

public:
	// CPPUNIT_NS::TestFixture
	//
	void setUp() {}
	void tearDown() {}

	void testPubCloseReOpen();
	void testPubSub();
	void testRemoveCSHs();
	void testOverrideOnStatus();
	void testUnsubscribeReSubscribe();
	void testPubSubReOpenSub();

};

int count_subListener_OnUpdate = 0;
int count_subConnSH_OnConnStatus = 0;
int count_subReqSH_OnStatus = 0;
int count_subReqSH_OnError = 0;
int count_subReqSH_OnSuccess = 0;
int count_pubConnSH_OnConnStatus = 0;
int count_pubReqSH_OnStatus = 0;
int count_pubReqSH_OnError = 0;
int count_pubReqSH_OnSuccess = 0;

CPPUNIT_TEST_SUITE_REGISTRATION(ConnectorTest2);

void RestCounters()
{
	count_subListener_OnUpdate = 0;
	count_subConnSH_OnConnStatus = 0;
	count_subReqSH_OnStatus = 0;
	count_subReqSH_OnError = 0;
	count_subReqSH_OnSuccess = 0;
	count_pubConnSH_OnConnStatus = 0;
	count_pubReqSH_OnStatus = 0;
	count_pubReqSH_OnError = 0;
	count_pubReqSH_OnSuccess = 0;
}

void DumpCounters()
{
	printf("count_subListener_OnUpdate  =%d\n",count_subListener_OnUpdate);
	printf("count_subConnSH_OnConnStatus=%d\n",count_subConnSH_OnConnStatus);
	printf("count_subReqSH_OnStatus     =%d\n",count_subReqSH_OnStatus);
	printf("count_subReqSH_OnError      =%d\n",count_subReqSH_OnError);
	printf("count_subReqSH_OnSuccess    =%d\n",count_subReqSH_OnSuccess);
	printf("count_pubConnSH_OnConnStatus=%d\n",count_pubConnSH_OnConnStatus);
	printf("count_pubReqSH_OnStatus     =%d\n",count_pubReqSH_OnStatus);
	printf("count_pubReqSH_OnError      =%d\n",count_pubReqSH_OnError);
	printf("count_pubReqSH_OnSuccess    =%d\n",count_pubReqSH_OnSuccess);
}

void DumpStr(const char* text)
{
	printf("%s\n", text);
}

void DumpStr(const char* text, int i)
{
	printf("%s%d\n", text, i);
}

void DumpStr(string text)
{
	printf("%s\n", text.c_str());
}

void DumpStr(wstring text)
{
	printf("%S\n", text.c_str());
}

void DumpMsg(const char* text, const Message& msg)
{
	printf("%s :\n", text);
	for (Message::Container::const_iterator it = 
			msg.GetContainer().begin(); 
			it != msg.GetContainer().end(); ++it)
	{
		printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
	}
}

class ConT2_SubListener : public IListener
{
public:
	void OnUpdate(const Message& msg)
	{
		//DumpMsg("<Sub>Listener: OnUpdate", msg);
		count_subListener_OnUpdate++;
	}
};

class ConT2_SubConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg)
	{
		//DumpMsg("<Sub>ConnectionStatusHandler: OnConnectionStatus", msg);
		count_subConnSH_OnConnStatus++;
	}
};

class ConT2_PubConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg)
	{
		//DumpMsg("<Pub>ConnectionStatusHandler: OnConnectionStatus", msg);
		count_pubConnSH_OnConnStatus++;
	}
};

class ConT2_SubRequestStatusHandler : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//DumpMsg("<Sub>Request Status Sucess: OnError", msg);
		count_subReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//DumpMsg("<Sub>RequestStatusHandler: OnSuccess", msg);
		count_subReqSH_OnSuccess++;
	}
};

class ConT2_PubRequestStatusHandler : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//DumpMsg("<Pub>RequestStatusHandler: OnError", msg);
		count_pubReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//DumpMsg("<Pub>RequestStatusHandler: OnSuccess", msg);
		count_pubReqSH_OnSuccess++;
	}
};

/**
 * This class tests of OnStatus truley overrides OnSucess() and OnError().
 */
class ConT2_SubRequestStatusHandler_OverrideOnStatus : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//DumpMsg("<Pub>RequestStatusHandler: OnError", msg);
		count_subReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//DumpMsg("<Pub>RequestStatusHandler: OnSuccess", msg);
		count_subReqSH_OnSuccess++;
	}
	void OnStatus(const Message& msg)
	{
		//DumpMsg("<Pub>RequestStatusHandler: OnStatus", msg);
		count_subReqSH_OnStatus++;
	}
};

/**
 * This class tests of OnStatus truley overrides OnSucess() and OnError().
 */
class ConT2_PubRequestStatusHandler_OverrideOnStatus : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//DumpMsg("<Pub>RequestStatusHandler: OnError", msg);
		count_pubReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//DumpMsg("<Pub>RequestStatusHandler: OnSuccess", msg);
		count_pubReqSH_OnSuccess++;
	}
	void OnStatus(const Message& msg)
	{
		//DumpMsg("<Pub>RequestStatusHandler: OnStatus", msg);
		count_pubReqSH_OnStatus++;
	}
};



/*
 * Test that Close() closes and Publish re-opens.
 */
void ConnectorTest2::testPubCloseReOpen()
{
	TU_INIT_TESTCASE("testPubCloseReOpen");
	RestCounters();

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/test/messages";


	Connector pubConn;
	ConT2_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;
	Message pubMsg;

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT2_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}
	
	CPPUNIT_ASSERT(pubConn.EnsureConnected());

	// Publish one message
	pubMsg.Set("do_method", "notify");
	pubMsg.Set(L"kn_to", topic);
	pubMsg.Set("kn_payload", "Hello-X");
	pubMsg.Set("nickname", "abc");
	pubMsg.Set("kn_response_format", "simple");

	CPPUNIT_ASSERT(pubConn.Publish(pubMsg, &pubReqSH));
	CPPUNIT_ASSERT(pubConn.IsConnected());
	CPPUNIT_ASSERT(pubConn.EnsureConnected());	


	CPPUNIT_ASSERT(pubConn.Close()); 
	CPPUNIT_ASSERT(!pubConn.IsConnected());

	CPPUNIT_ASSERT(pubConn.Publish(pubMsg, &pubReqSH));//Publish will re open.
	CPPUNIT_ASSERT(pubConn.IsConnected());

	CPPUNIT_ASSERT(pubConn.Close());
	CPPUNIT_ASSERT(!pubConn.IsConnected());
	
	CPPUNIT_ASSERT(pubConn.EnsureConnected());//EnsureConnected will re open.
	CPPUNIT_ASSERT(pubConn.IsConnected());

	//Close and Publish one more time to make sure really working
	CPPUNIT_ASSERT(pubConn.Close()); 
	CPPUNIT_ASSERT(!pubConn.IsConnected());

	CPPUNIT_ASSERT(pubConn.Publish(pubMsg, &pubReqSH));//Publish will re open.
	CPPUNIT_ASSERT(pubConn.IsConnected());

	//DumpCounters();
	CPPUNIT_ASSERT(count_subListener_OnUpdate  ==0); // No sub
	CPPUNIT_ASSERT(count_subConnSH_OnConnStatus==0); // "
	CPPUNIT_ASSERT(count_subReqSH_OnStatus     ==0); // "
	CPPUNIT_ASSERT(count_subReqSH_OnError      ==0); // "
	CPPUNIT_ASSERT(count_subReqSH_OnSuccess    ==0); // "
	CPPUNIT_ASSERT(count_pubConnSH_OnConnStatus==3); // 2 pub connections
	CPPUNIT_ASSERT(count_pubReqSH_OnStatus     ==0); // 
	CPPUNIT_ASSERT(count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnSuccess    ==3); // 2 pub connections
}


/*
<pub>notify: X
<Pub>ConnectionStatusHandler: OnConnectionStatus :...error_string =,status = 200 Watching topic.,status_code = 200
<Pub>RequestStatusHandler: OnStatus :             status = 200 OK,status_code = 200
<Sub>ConnectionStatusHandler: OnConnectionStatus :...error_string =,status = 200 Watching topic.,status_code = 200
<Sub>RequestStatusHandler: OnStatus :LIBKN_RID...,status = 200 OK,status_code = 200
<sub>rid = http://localhost:8000/kn/what/test/messages/kn_routes/00001211
<pub>notify: 0
<Pub>RequestStatusHandler: OnStatus :,status = 200 OK,status_code = 200
<pub>notify: 1
<Sub>Listener: OnUpdate :...kn_payload = Hello-0...
<Sub>Listener: OnUpdate :...kn_payload = Hello-1...
<Pub>RequestStatusHandler: OnStatus :status = 200 OK,status_code = 200
<pub>notify: 2
<Sub>Listener: OnUpdate :...kn_payload = Hello-2...
<Pub>RequestStatusHandler: OnStatus :status = 200 OK,status_code = 200
<pub>notify: 3
<Sub>Listener: OnUpdate :...kn_payload = Hello-3...
<Pub>RequestStatusHandler: OnStatus :status = 200 OK,status_code = 200
<pub>notify: 4
<Sub>Listener: OnUpdate :...kn_payload = Hello-4...
<Pub>RequestStatusHandler: OnStatus :status = 200 OK,status_code = 200
<sub>Sleep(10)
<sub>Unsubscribing.
<Sub>RequestStatusHandler: OnStatus :LIBKN_RID =...,status = 200 OK,status_code = 200

count_subListener_OnUpdate  =5 // 5 updates
count_subConnSH_OnConnStatus=1 // 1 sub connection
count_subReqSH_OnStatus     =0
count_subReqSH_OnError      =0
count_subReqSH_OnSuccess    =2 // connecting, unsubscribing
count_pubConnSH_OnConnStatus=1 // 1 pub connection
count_pubReqSH_OnStatus     =0
count_pubReqSH_OnError      =0
count_pubReqSH_OnSuccess    =6 // initial pub and 5 updates
 */
void ConnectorTest2::testPubSub()
{
	TU_INIT_TESTCASE("testPubSub");
	RestCounters();

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/test/messages";


	Connector pubConn;
	ConT2_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;
	Message pubMsg;

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT2_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}
	
	// Publish one message to make sure topic will exist.
	pubMsg.Set("do_method", "notify");
	pubMsg.Set(L"kn_to", topic);
	pubMsg.Set("kn_payload", "Hello-X");
	pubMsg.Set("nickname", "abc");
	pubMsg.Set("kn_response_format", "simple");

	//DumpStr("<pub>notify: X");
	pubConn.Publish(pubMsg, &pubReqSH);

	if (pubConn.HasItems())
	{
		CPPUNIT_FAIL("There's something in the queue (it should be empty).");
		return;
	}

	
	// Start the subscriber.
	Connector subConn;
	ConT2_SubListener subListener;
	ConT2_SubRequestStatusHandler subReqSH;
	ITransport::Parameters subITParams;


    // This is only necessary to get subConnSH_OnConnStatus
	ConT2_SubConnStatusHandler subConnSH;
	subConn.AddConnectionStatusHandler(&subConnSH);

	subITParams.m_ServerUrl = serverUrl;
   	if (!subConn.Open(subITParams)){
		CPPUNIT_FAIL("Open(subITParams) failed.");
		return;
	}
	
	wstring rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);

	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
	}
	else
	{
		//DumpStr(L"<sub>rid = " + rid);

		string hello = "Hello-";
		for(int i = 0; i < 5; i++){
			char buf[12];
			itoa(i,buf,10);
			//DumpStr("<pub>notify: ",i);
			pubMsg.Set("kn_payload", hello + buf);
			pubConn.Publish(pubMsg, &pubReqSH);

		}

		//DumpStr("<sub>Sleep(10)");
		Sleep(1000);

		//DumpStr("<sub>Unsubscribing.");
	
		if (!subConn.Unsubscribe(rid, &subReqSH))
		{
			CPPUNIT_FAIL("failed to unsubscribe.");
		}
	}

	if (pubConn.HasItems())
	{
		CPPUNIT_FAIL("There's something in the queue (it should be empty).");
		return;
	}

	pubConn.Close();
	subConn.Close();

	subConn.RemoveConnectionStatusHandler(&subConnSH);

	//DumpCounters();
	CPPUNIT_ASSERT(count_subListener_OnUpdate  ==5); // 5 updates - this test fails intermittantly.
	                                                 // Need to wait until 5th update witha time out.
	CPPUNIT_ASSERT(count_subConnSH_OnConnStatus==1); // 1st Subscribe() - this test fails intermittantly.
	                                                 // Probably need a log to catch what is going on.
	CPPUNIT_ASSERT(count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnSuccess    ==2); // Subscribe(),Unsubscribe()
	CPPUNIT_ASSERT(count_pubConnSH_OnConnStatus==1); // 1st Publish() - this test fails intermittantly.
	                                                 // Probably need a log to catch what is going on.
	CPPUNIT_ASSERT(count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnSuccess    ==6); // Publish() initial pub and 5 updates

}

/*
 * This test is same as testPubSub() but the IConnectionStatusHandlers
 * are removed. Except fot the counts everything else should be the same.
 */ 
void ConnectorTest2::testRemoveCSHs()
{
	TU_INIT_TESTCASE("testRemoveCSHs");
	RestCounters();

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/test/messages";


	Connector pubConn;
	ConT2_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;
	Message pubMsg;

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT2_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	// Now, remove it.
	pubConn.RemoveConnectionStatusHandler(&pubConnSH); // <<<TEST>>>

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}
	
	// Publish one message to make sure topic will exist.
	pubMsg.Set("do_method", "notify");
	pubMsg.Set(L"kn_to", topic);
	pubMsg.Set("kn_payload", "Hello-X");
	pubMsg.Set("nickname", "abc");
	pubMsg.Set("kn_response_format", "simple");

	//DumpStr("<pub>notify: X");
	pubConn.Publish(pubMsg, &pubReqSH);

	if (pubConn.HasItems())
	{
		CPPUNIT_FAIL("There's something in the queue (it should be empty).");
		return;
	}


	// Start the subscriber.
	Connector subConn;
	ConT2_SubListener subListener;
	ConT2_SubRequestStatusHandler subReqSH;
	ITransport::Parameters subITParams;


    // This is only necessary to get subConnSH_OnConnStatus
	ConT2_SubConnStatusHandler subConnSH;
	subConn.AddConnectionStatusHandler(&subConnSH);

	// Now, remove it.
	subConn.RemoveConnectionStatusHandler(&subConnSH); // <<<TEST>>>

	subITParams.m_ServerUrl = serverUrl;
   	if (!subConn.Open(subITParams)){
		CPPUNIT_FAIL("Open(subITParams) failed.");
		return;
	}
	
	wstring rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);

	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
	}
	else
	{
		//DumpStr(L"<sub>rid = " + rid);

		string hello = "Hello-";
		for(int i = 0; i < 5; i++){
			char buf[12];
			itoa(i,buf,10);
			//DumpStr("<pub>notify: ",i);
			pubMsg.Set("kn_payload", hello + buf);
			pubConn.Publish(pubMsg, &pubReqSH);

		}

		//DumpStr("<sub>Sleep(10)");
		Sleep(10);

		//DumpStr("<sub>Unsubscribing.");
	
		if (!subConn.Unsubscribe(rid, &subReqSH))
		{
			CPPUNIT_FAIL("failed to unsubscribe.");
		}
	}

	if (pubConn.HasItems())
	{
		CPPUNIT_FAIL("There's something in the queue (it should be empty).");
		return;
	}

	pubConn.Close();
	subConn.Close();

	subConn.RemoveConnectionStatusHandler(&subConnSH);

	//DumpCounters();
	CPPUNIT_ASSERT(count_subListener_OnUpdate  ==5); // 5 updates
	CPPUNIT_ASSERT(count_subConnSH_OnConnStatus==0); // 1st Subscribe() (NOT handled) // <<<TEST>>>
	CPPUNIT_ASSERT(count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnSuccess    ==2); // Subscribe(),Unsubscribe()
	CPPUNIT_ASSERT(count_pubConnSH_OnConnStatus==0); // 1st Publish() (NOT handled) // <<<TEST>>>
	CPPUNIT_ASSERT(count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnSuccess    ==6); // Publish() initial pub and 5 updates

}



/*
 * This test is same as testPubSub() but the IRequestStatusHandlers
 * override OnStatus(). Thus there should be counts for OnStatus() but not for
 * OnSuccess() or OnError().  This test only gets sucesses so it is not complete.
 * Except for the counts everything else should be the same.
 */ 
void ConnectorTest2::testOverrideOnStatus()
{
	TU_INIT_TESTCASE("testOverrideOnStatus");
	RestCounters();

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/test/messages";


	Connector pubConn;
	ConT2_PubRequestStatusHandler_OverrideOnStatus pubReqSH; // <<<TEST>>>
	ITransport::Parameters pubITParams;
	Message pubMsg;

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT2_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	// Now, remove it.
	pubConn.RemoveConnectionStatusHandler(&pubConnSH);

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}
	
	// Publish one message to make sure topic will exist.
	pubMsg.Set("do_method", "notify");
	pubMsg.Set(L"kn_to", topic);
	pubMsg.Set("kn_payload", "Hello-X");
	pubMsg.Set("nickname", "abc");
	pubMsg.Set("kn_response_format", "simple");

	//DumpStr("<pub>notify: X");
	pubConn.Publish(pubMsg, &pubReqSH);

	if (pubConn.HasItems())
	{
		CPPUNIT_FAIL("There's something in the queue (it should be empty).");
		return;
	}


	// Start the subscriber.
	Connector subConn;
	ConT2_SubListener subListener;
	ConT2_SubRequestStatusHandler_OverrideOnStatus subReqSH; // <<<TEST>>>
	ITransport::Parameters subITParams;


    // This is only necessary to get subConnSH_OnConnStatus
	ConT2_SubConnStatusHandler subConnSH;
	subConn.AddConnectionStatusHandler(&subConnSH);

	// Now, remove it.
	subConn.RemoveConnectionStatusHandler(&subConnSH);

	subITParams.m_ServerUrl = serverUrl;
   	if (!subConn.Open(subITParams)){
		CPPUNIT_FAIL("Open(subITParams) failed.");
		return;
	}
	
	wstring rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);

	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
	}
	else
	{
		//DumpStr(L"<sub>rid = " + rid);

		string hello = "Hello-";
		for(int i = 0; i < 5; i++){
			char buf[12];
			itoa(i,buf,10);
			//DumpStr("<pub>notify: ",i);
			pubMsg.Set("kn_payload", hello + buf);
			pubConn.Publish(pubMsg, &pubReqSH);

		}

		//DumpStr("<sub>Sleep(10)");
		Sleep(10);

		//DumpStr("<sub>Unsubscribing.");
	
		if (!subConn.Unsubscribe(rid, &subReqSH))
		{
			CPPUNIT_FAIL("failed to unsubscribe.");
		}
	}

	if (pubConn.HasItems())
	{
		CPPUNIT_FAIL("There's something in the queue (it should be empty).");
		return;
	}

	pubConn.Close();
	subConn.Close();

	subConn.RemoveConnectionStatusHandler(&subConnSH);

	//DumpCounters();
	CPPUNIT_ASSERT(count_subListener_OnUpdate  ==5); // 5 updates
	CPPUNIT_ASSERT(count_subConnSH_OnConnStatus==0); // 1st Subscribe() (NOT handled)
	CPPUNIT_ASSERT(count_subReqSH_OnStatus     ==2); // <<<TEST>>>
	CPPUNIT_ASSERT(count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnSuccess    ==0); // connecting, unsubscribing
	CPPUNIT_ASSERT(count_pubConnSH_OnConnStatus==0); // 1st Publish() (NOT handled)
	CPPUNIT_ASSERT(count_pubReqSH_OnStatus     ==6); // <<<TEST>>>
	CPPUNIT_ASSERT(count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnSuccess    ==0); // Publish() initial pub and 5 updates

}

/*
 * Test testUnsubscribe()
 */
void ConnectorTest2::testUnsubscribeReSubscribe()
{
	TU_INIT_TESTCASE("testUnsubscribeReSubscribe");
	RestCounters();

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/test/messages";


	Connector pubConn;
	ConT2_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;
	Message pubMsg;

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT2_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}
	
	// Publish one message to make sure topic will exist.
	pubMsg.Set("do_method", "notify");
	pubMsg.Set(L"kn_to", topic);
	pubMsg.Set("kn_payload", "Hello-X");
	pubMsg.Set("nickname", "abc");
	pubMsg.Set("kn_response_format", "simple");

	//DumpStr("<pub>notify: X");
	pubConn.Publish(pubMsg, &pubReqSH);

	if (pubConn.HasItems())
	{
		CPPUNIT_FAIL("There's something in the queue (it should be empty).");
		return;
	}

	
	// Start the subscriber.
	Connector subConn;
	ConT2_SubListener subListener;
	ConT2_SubRequestStatusHandler subReqSH;
	ITransport::Parameters subITParams;


    // This is only necessary to get subConnSH_OnConnStatus
	ConT2_SubConnStatusHandler subConnSH;
	subConn.AddConnectionStatusHandler(&subConnSH);

	subITParams.m_ServerUrl = serverUrl;
   	if (!subConn.Open(subITParams)){
		CPPUNIT_FAIL("Open(subITParams) failed.");
		return;
	}
	
	wstring rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);
	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
	}
	else
	{
		//DumpStr(L"<sub>rid = " + rid);

		string hello = "Hello-";
		for(int i = 0; i < 5; i++){
			char buf[12];
			itoa(i,buf,10);
			//DumpStr("<pub>notify: ",i);
			pubMsg.Set("kn_payload", hello + buf);
			pubConn.Publish(pubMsg, &pubReqSH);

		}

		//DumpStr("<sub>Sleep(10)");
		Sleep(10);

		//DumpStr("<sub>Unsubscribing.");
		if (!subConn.Unsubscribe(rid, &subReqSH))
		{
			CPPUNIT_FAIL("failed to unsubscribe.");
		}

		// These updates should be missed by the subscriber.
		for(i = 5; i < 10; i++){
			char buf[12];
			itoa(i,buf,10);
			//DumpStr("<pub>notify: ",i);
			pubMsg.Set("kn_payload", hello + buf);
			pubConn.Publish(pubMsg, &pubReqSH);

		}

	}

	//Now, make sure we can re-subscribe.
	rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);
	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to re-subscribe.");
	}
	else
	{
		//DumpStr(L"<sub>rid = " + rid);

		string hello = "Hello-";
		for(int i = 10; i < 15; i++){
			char buf[12];
			itoa(i,buf,10);
			//DumpStr("<pub>notify: ",i);
			pubMsg.Set("kn_payload", hello + buf);
			pubConn.Publish(pubMsg, &pubReqSH);

		}

		//DumpStr("<sub>Sleep(10)");
		Sleep(10);

		//DumpStr("<sub>Unsubscribing.");
		if (!subConn.Unsubscribe(rid, &subReqSH))
		{
			CPPUNIT_FAIL("failed to unsubscribe.");
		}

		// These updates should be missed by the subscriber.
		for(i = 15; i < 20; i++){
			char buf[12];
			itoa(i,buf,10);
			//DumpStr("<pub>notify: ",i);
			pubMsg.Set("kn_payload", hello + buf);
			pubConn.Publish(pubMsg, &pubReqSH);

		}

	}

	if (pubConn.HasItems())
	{
		CPPUNIT_FAIL("There's something in the queue (it should be empty).");
		return;
	}

	pubConn.Close();
	subConn.Close();

	subConn.RemoveConnectionStatusHandler(&subConnSH);

	//DumpCounters();
	CPPUNIT_ASSERT(count_subListener_OnUpdate  ==10); // 5 updates seen, 5 missed, 5 seen, 5 missed.
	CPPUNIT_ASSERT(count_subConnSH_OnConnStatus==1); // Subscribe()
	CPPUNIT_ASSERT(count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnSuccess    ==4); // Subscribe(),Unsubscribe(),Subscribe(),Unsubscribe()
	CPPUNIT_ASSERT(count_pubConnSH_OnConnStatus==1); // Publish() the first
	CPPUNIT_ASSERT(count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnSuccess    ==21); // Publish() initial pub and 20 updates

}

void ConnectorTest2::testPubSubReOpenSub()
{
	TU_INIT_TESTCASE("testPubSubReOpenSub");
	RestCounters();

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/test/messages";


	Connector pubConn;
	ConT2_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;
	Message pubMsg;

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT2_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}
	
	// Publish one message to make sure topic will exist.
	pubMsg.Set("do_method", "notify");
	pubMsg.Set(L"kn_to", topic);
	pubMsg.Set("kn_payload", "Hello-X");
	pubMsg.Set("nickname", "abc");
	pubMsg.Set("kn_response_format", "simple");

	//DumpStr("<pub>notify: X");
	pubConn.Publish(pubMsg, &pubReqSH);

	
	// Start the subscriber.
	Connector subConn;
	ConT2_SubListener subListener;
	ConT2_SubRequestStatusHandler subReqSH;
	ITransport::Parameters subITParams;


    // This is only necessary to get subConnSH_OnConnStatus
	ConT2_SubConnStatusHandler subConnSH;
	subConn.AddConnectionStatusHandler(&subConnSH);

	subITParams.m_ServerUrl = serverUrl;
   	if (!subConn.Open(subITParams)){
		CPPUNIT_FAIL("Open(subITParams) failed.");
		return;
	}
	
	wstring rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);

	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
	}
	else
	{
		//DumpStr(L"<sub>rid = " + rid);

		string hello = "Hello-";
		for(int i = 0; i < 5; i++){
			char buf[12];
			itoa(i,buf,10);
			//DumpStr("<pub>notify: ",i);
			pubMsg.Set("kn_payload", hello + buf);
			pubConn.Publish(pubMsg, &pubReqSH);

		}

		//DumpStr("<sub>Sleep(10)");
		Sleep(10);

		//DumpStr("<sub>Unsubscribing.");
	
		if (!subConn.Unsubscribe(rid, &subReqSH))
		{
			CPPUNIT_FAIL("failed to unsubscribe.");
		}
	}

	//DumpCounters();
	CPPUNIT_ASSERT(count_subListener_OnUpdate  ==5); // 5 updates
	CPPUNIT_ASSERT(count_subConnSH_OnConnStatus==1); // Subscribe()
	CPPUNIT_ASSERT(count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnSuccess    ==2); // Subscribe(),Unsubscribe()
	CPPUNIT_ASSERT(count_pubConnSH_OnConnStatus==1); // Publish() the first - this test fails intermittantly.
	                                                 // Probably need a log to catch what is going on.
	CPPUNIT_ASSERT(count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnSuccess    ==6); // Publish() 1 and 5 


	// Close subscriber.
	RestCounters();
	CPPUNIT_ASSERT(subConn.Close());
	CPPUNIT_ASSERT(!subConn.IsConnected());
	
	// Do some more publishing.
	// The subscriber should not see them.
	string hello = "Miss You-";
	for(int i = 0; i < 5; i++){
		char buf[12];
		itoa(i,buf,10);
		//DumpStr("<pub>notify: ",i);
		pubMsg.Set("kn_payload", hello + buf);
		pubConn.Publish(pubMsg, &pubReqSH);
	}
	//DumpStr("<sub>Sleep(10)");
	Sleep(10);

	//DumpCounters();
	CPPUNIT_ASSERT(count_subListener_OnUpdate  ==0); // Don't see updates
	CPPUNIT_ASSERT(count_subConnSH_OnConnStatus==0);  
	CPPUNIT_ASSERT(count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnSuccess    ==0);  
	CPPUNIT_ASSERT(count_pubConnSH_OnConnStatus==0);  
	CPPUNIT_ASSERT(count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnSuccess    ==5); // Publish() 5

	//EnsureConnected will re open.
	RestCounters();
	CPPUNIT_ASSERT(!subConn.IsConnected());
	CPPUNIT_ASSERT(subConn.EnsureConnected());
	CPPUNIT_ASSERT(subConn.IsConnected());

	// Do some more publishing.
	// The subscriber should not see them even though it is reconnected.
	for(i = 0; i < 5; i++){
		char buf[12];
		itoa(i,buf,10);
		//DumpStr("<pub>notify: ",i);
		pubMsg.Set("kn_payload", hello + buf);
		pubConn.Publish(pubMsg, &pubReqSH);
	}
	//DumpStr("<sub>Sleep(10)");
	Sleep(10);

	//DumpCounters();
	CPPUNIT_ASSERT(count_subListener_OnUpdate  ==0); // can't see 5 updates
	CPPUNIT_ASSERT(count_subConnSH_OnConnStatus==0); 
	CPPUNIT_ASSERT(count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnSuccess    ==0); 
	CPPUNIT_ASSERT(count_pubConnSH_OnConnStatus==0); 
	CPPUNIT_ASSERT(count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnSuccess    ==5); // Publish() 5



	//Subscribe one more time to make sure really working
	RestCounters();

	rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);
	CPPUNIT_ASSERT(subConn.IsConnected());

	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
	}
	else
	{
		//DumpStr(L"<sub>rid = " + rid);

		string hello = "Hello-";
		for(int i = 0; i < 5; i++){
			char buf[12];
			itoa(i,buf,10);
			//DumpStr("<pub>notify: ",i);
			pubMsg.Set("kn_payload", hello + buf);
			pubConn.Publish(pubMsg, &pubReqSH);

		}

		//DumpStr("<sub>Sleep(10)");
		Sleep(10);

		//DumpStr("<sub>Unsubscribing.");
	
		if (!subConn.Unsubscribe(rid, &subReqSH))
		{
			CPPUNIT_FAIL("failed to unsubscribe.");
		}
	}

	pubConn.Close();
	subConn.Close();

	subConn.RemoveConnectionStatusHandler(&subConnSH);

	//DumpCounters();
	CPPUNIT_ASSERT(count_subListener_OnUpdate  ==5); // saw 5 updates
	CPPUNIT_ASSERT(count_subConnSH_OnConnStatus==1); // Subscribe()
	CPPUNIT_ASSERT(count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnSuccess    ==2); // Subscribe(),Unsubscribe()
	CPPUNIT_ASSERT(count_pubConnSH_OnConnStatus==0); 
	CPPUNIT_ASSERT(count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnSuccess    ==5); // Publish() 5

}

