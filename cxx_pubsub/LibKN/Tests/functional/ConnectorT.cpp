#include "stdafx.h"
#include <LibKN\Connector.h>
#include <LibKN\StrUtil.h>

class ConnectorTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(ConnectorTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testPub);
	CPPUNIT_TEST(testConnect);
	CPPUNIT_TEST_SUITE_END();

public:
	// CPPUNIT_NS::TestFixture
	//
	void setUp() {}
	void tearDown() {}

	void testConstructor();
	void testPub();
	void testConnect();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConnectorTest);

void ConnectorTest::testConstructor()
{
	Connector c;
	CPPUNIT_ASSERT(!c.IsConnected());
}

void ConnectorTest::testConnect()
{
	Connector c;
	ITransport::Parameters p;

	// No server set, must fail
	//
	bool b = c.Open(p);
	CPPUNIT_ASSERT(!b);

	p.m_ServerUrl = "http://localhost:8000/kn";
	CPPUNIT_ASSERT(c.Open(p));
	CPPUNIT_ASSERT(c.IsConnected());
	CPPUNIT_ASSERT(c.Close());
	CPPUNIT_ASSERT(!c.IsConnected());
}

class MyHandler : public IRequestStatusHandler
{
public:
	void DumpMsg(const char* text, const Message& msg)
	{
		printf("%s ============\n", text);
		for (Message::Container::const_iterator it = msg.GetContainer().begin(); it != msg.GetContainer().end(); ++it)
		{
			printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
		}
		printf("\n");
	}

	void OnError(const Message& msg)
	{
		DumpMsg("OnError", msg);
	}
	void OnSuccess(const Message& msg)
	{
		DumpMsg("OnSuccess", msg);
	}
};


void ConnectorTest::testPub()
{
	Connector c;
	ITransport::Parameters p;
	MyHandler mh;
	p.m_ServerUrl = "http://localhost:8000/kn";

	if (c.Open(p))
	{
		Message m;
		m.Set("do_method", "notify");
		m.Set("kn_to", "/what/knchat/messages");
		m.Set("kn_payload", "Hello");
		m.Set("nickname", "abc");
		m.Set("kn_response_format", "simple");

		DWORD s = GetTickCount();
		const int n = 10;
		for (int i = 0; i < n; i++)
		{
			//printf("%d\n", i);
			c.Publish(m, &mh);
		}
		DWORD e = GetTickCount();

//		printf("\n%d updates in %d msec\n", n, e-s);

		c.Close();
	}
}

