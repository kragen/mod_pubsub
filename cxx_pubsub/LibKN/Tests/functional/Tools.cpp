#include "stdafx.h"
#include <LibKN\Connector.h>
#include <LibKN\StrUtil.h>

#define VERBOSE

/**
 * + Positive test
 * - Negative test
 * ? In code but don not know if exercised
 *
 *
 * -----------------------------------------------------------------
 *                        Tested in Tests/functional/ConnectorTx:
 * Class                                      ConnT ConnT2 ConnT3
 * =========================================  ===== ====== ======
 * Connector:                                 
 * -----------------------------------------  ----- ------ ------
 *   Connector()                              +     +
 *   ~Connector()                             +     +
 *   bool IsConnected();                      + -   + -
 *   bool Open();                             + -   +
 *   const Parameters& GetParameters() const;
 *   bool Close();                            +     +
 *   bool EnsureConnected();                        +
 *   bool GetQueueing();
 *   void SetQueueing();
 *   bool SaveQueue();
 *   bool LoadQueue();
 *   bool Flush();
 *   bool Clear();
 *   bool HasItems();
 *   bool Publish();                                +
 *   wstring Subscribe();                           +
 *   bool Unsubscribe();                            2
 *   void AddConnectionStatusHandler();             0 p s
 *   void RemoveConnectionStatusHandler();          0 p s
 *   wstring GetRouteId() const;
 *   bool SubscribeRouteId();
 * =========================================  ===== ====== ======
 * ITransport::Parameters
 * -----------------------------------------  ----- ------ ------
 *   tstring  m_ServerUrl                     +     +
 *   tstring  m_Username 
 *   tstring  m_Password 
 *   bool  m_UseProxy 
 *   tstring  m_ProxyServer 
 *   tstring  m_ProxyUsername 
 *   tstring  m_ProxyPassword 
 *   tstring  m_ProxyExceptionList 
 *   tstring  m_CustomHeader 
 *   bool  m_ShowUI 
 *   HWND  m_ParentHwnd 
 * =========================================  ===== ====== ======
 * IConnectionStatusHandler - publish
 * -----------------------------------------  ----- ------ ------
 *   virtual void  OnConnectionStatus()             1
 * =========================================  ===== ====== ======
 * IConnectionStatusHandler - subscribe
 * -----------------------------------------  ----- ------ ------
 *   virtual void  OnConnectionStatus()             1
 * =========================================  ===== ====== ======
 * IListener - subscribe
 * -----------------------------------------  ----- ------ ------
 *   virtual void  Update()                         0 5 10
 * =========================================  ===== ====== ======
 * IRequestStatusHandler - publish
 * -----------------------------------------  ----- ------ ------
 *   virtual void  OnStatus()                      0 10 20
 *   virtual void  OnError()                  
 *   virtual void  OnSuccess()                +     6
 * =========================================  ===== ====== ======
 * IRequestStatusHandler - subscribe
 * -----------------------------------------  ----- ------ ------
 *   virtual void  OnStatus()                       0 2 4
 *   virtual void  OnError()                  
 *   virtual void  OnSuccess()                +     2
 * =========================================  ===== ====== ======
 */

class ConnectorTest3 : public CPPUNIT_NS::TestFixture
{

	CPPUNIT_TEST_SUITE(ConnectorTest3);
	CPPUNIT_TEST(testPubClose);
	CPPUNIT_TEST(testPubSub);
	CPPUNIT_TEST_SUITE_END();

public:
	// CPPUNIT_NS::TestFixture
	//
	void setUp() {}
	void tearDown() {}

	void testPubClose();
	void testPubSub();

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

CPPUNIT_TEST_SUITE_REGISTRATION(ConnectorTest3);

void ConT3_ResetCounters()
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

void ConT3_DumpCounters()
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

void ConT3_DumpStr(const char* text)
{
	printf("%s\n", text);
}

void ConT3_DumpStr(const char* text, int i)
{
	printf("%s%d\n", text, i);
}

void ConT3_DumpStr(string text)
{
	printf("%s\n", text.c_str());
}

void ConT3_DumpStr(wstring text)
{
	printf("%S\n", text.c_str());
}

void ConT3_DumpMsg(const char* text, const Message& msg)
{
	printf("%s :\n", text);
	for (Message::Container::const_iterator it = 
			msg.GetContainer().begin(); 
			it != msg.GetContainer().end(); ++it)
	{
		printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
	}
}

class ConT3_SubListener : public IListener
{
public:
	void OnUpdate(const Message& msg)
	{
		//ConT3_DumpMsg("<Sub>Listener: OnUpdate", msg);
		count_subListener_OnUpdate++;
	}
};

class ConT3_SubConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg)
	{
		//ConT3_DumpMsg("<Sub>ConnectionStatusHandler: OnConnectionStatus", msg);
		count_subConnSH_OnConnStatus++;
	}
};

class ConT3_PubConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg)
	{
		//ConT3_DumpMsg("<Pub>ConnectionStatusHandler: OnConnectionStatus", msg);
		count_pubConnSH_OnConnStatus++;
	}
};

class ConT3_SubRequestStatusHandler : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//ConT3_DumpMsg("<Sub>Request Status Sucess: OnError", msg);
		count_subReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//ConT3_DumpMsg("<Sub>RequestStatusHandler: OnSuccess", msg);
		count_subReqSH_OnSuccess++;
	}
};

class ConT3_PubRequestStatusHandler : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//ConT3_DumpMsg("<Pub>RequestStatusHandler: OnError", msg);
		count_pubReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//ConT3_DumpMsg("<Pub>RequestStatusHandler: OnSuccess", msg);
		count_pubReqSH_OnSuccess++;
	}
};

/**
 * This class tests of OnStatus truley overrides OnSucess() and OnError().
 */
class ConT3_SubRequestStatusHandler_OverrideOnStatus : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//ConT3_DumpMsg("<Pub>RequestStatusHandler: OnError", msg);
		count_subReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//ConT3_DumpMsg("<Pub>RequestStatusHandler: OnSuccess", msg);
		count_subReqSH_OnSuccess++;
	}
	void OnStatus(const Message& msg)
	{
		//ConT3_DumpMsg("<Pub>RequestStatusHandler: OnStatus", msg);
		count_subReqSH_OnStatus++;
	}
};

/**
 * This class tests of OnStatus truley overrides OnSucess() and OnError().
 */
class ConT3_PubRequestStatusHandler_OverrideOnStatus : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//ConT3_DumpMsg("<Pub>RequestStatusHandler: OnError", msg);
		count_pubReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//ConT3_DumpMsg("<Pub>RequestStatusHandler: OnSuccess", msg);
		count_pubReqSH_OnSuccess++;
	}
	void OnStatus(const Message& msg)
	{
		//ConT3_DumpMsg("<Pub>RequestStatusHandler: OnStatus", msg);
		count_pubReqSH_OnStatus++;
	}
};



/*
 */
void ConnectorTest3::testPubClose()
{
	ConT3_ResetCounters();

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/test/messages";


	Connector pubConn;
	ConT3_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;
	Message pubMsg;

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT3_PubConnStatusHandler pubConnSH;
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


	CPPUNIT_ASSERT(pubConn.Close()); //If I do this it still can publish later.
	
	// These should all fail.
	CPPUNIT_ASSERT(!pubConn.IsConnected());
	CPPUNIT_ASSERT(!pubConn.EnsureConnected());//BUG

	//ConT3_DumpCounters();
	CPPUNIT_ASSERT(count_subListener_OnUpdate  ==0); // No sub
	CPPUNIT_ASSERT(count_subConnSH_OnConnStatus==0); // "
	CPPUNIT_ASSERT(count_subReqSH_OnStatus     ==0); // "
	CPPUNIT_ASSERT(count_subReqSH_OnError      ==0); // "
	CPPUNIT_ASSERT(count_subReqSH_OnSuccess    ==0); // "
	CPPUNIT_ASSERT(count_pubConnSH_OnConnStatus==1); // 1 pub connection, the second should fail
	CPPUNIT_ASSERT(count_pubReqSH_OnStatus     ==0); // 
	CPPUNIT_ASSERT(count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnSuccess    ==1); // 1 pub connection, the second should fail
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
void ConnectorTest3::testPubSub()
{
	ConT3_ResetCounters();

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/test/messages";


	Connector pubConn;
	ConT3_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;
	Message pubMsg;

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT3_PubConnStatusHandler pubConnSH;
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

	//ConT3_DumpStr("<pub>notify: X");
	pubConn.Publish(pubMsg, &pubReqSH);

	if (pubConn.HasItems())
	{
		CPPUNIT_FAIL("There's something in the queue (it should be empty).");
		return;
	}

	
	// Start the subscriber.
	Connector subConn;
	ConT3_SubListener subListener;
	ConT3_SubRequestStatusHandler subReqSH;
	ITransport::Parameters subITParams;


    // This is only necessary to get subConnSH_OnConnStatus
	ConT3_SubConnStatusHandler subConnSH;
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
		//ConT3_DumpStr(L"<sub>rid = " + rid);

		string hello = "Hello-";
		for(int i = 0; i < 5; i++){
			char buf[12];
			itoa(i,buf,10);
			//ConT3_DumpStr("<pub>notify: ",i);
			pubMsg.Set("kn_payload", hello + buf);
			pubConn.Publish(pubMsg, &pubReqSH);

		}

		//ConT3_DumpStr("<sub>Sleep(10)");
		Sleep(10);

		//ConT3_DumpStr("<sub>Unsubscribing.");
	
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

	//ConT3_DumpCounters();
	CPPUNIT_ASSERT(count_subListener_OnUpdate  ==5); // 5 updates
	CPPUNIT_ASSERT(count_subConnSH_OnConnStatus==1); // 1 sub connection
	CPPUNIT_ASSERT(count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_subReqSH_OnSuccess    ==2); // connecting, unsubscribing
	CPPUNIT_ASSERT(count_pubConnSH_OnConnStatus==1); // 1 pub connection
	CPPUNIT_ASSERT(count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(count_pubReqSH_OnSuccess    ==6); // initial pub and 5 updates

}




