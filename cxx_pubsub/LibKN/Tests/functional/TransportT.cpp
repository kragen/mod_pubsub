// TransportT.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <LibKN\Transport.h>

class TransportTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(TransportTest);
//	CPPUNIT_TEST(testConstructor);
//	CPPUNIT_TEST(testConnection);
	CPPUNIT_TEST_SUITE_END();

public:
	// CPPUNIT_NS::TestFixture
	//
	void setUp();
	void tearDown();

	void testConstructor();
	void testConnection();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TransportTest);

void TransportTest::setUp()
{
}

void TransportTest::tearDown()
{
}

void TransportTest::testConstructor()
{
#if 0
	Transport t(0);
	CPPUNIT_ASSERT(!t.IsConnected());
#endif
}

void TransportTest::testConnection()
{
#if 0
	Transport t(0);
	CPPUNIT_ASSERT(!t.IsConnected());

	ITransport::Parameters p;
	CPPUNIT_ASSERT(p.m_UseProxy == false);

	p.m_ServerUrl = _T("http://www.ii.com:8000/kn");

	if (t.Open(p))
	{
		CPPUNIT_ASSERT(t.IsConnected());
		t.Close();
		CPPUNIT_ASSERT(!t.IsConnected());
	}
	else
	{
		CPPUNIT_ASSERT_MESSAGE("Open failed", false);
	}
#endif
}

