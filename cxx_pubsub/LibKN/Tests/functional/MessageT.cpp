#include "stdafx.h"
#include "globals.h"
#include "testutil.h"
#include <LibKN\Message.h>
#include <LibKN\StrUtil.h>

class MessageTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(MessageTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testCopyConstructor);
	CPPUNIT_TEST(testOperatorEqual);
	CPPUNIT_TEST(testSet);
	CPPUNIT_TEST(testConversion);
	CPPUNIT_TEST(testRemove);
	CPPUNIT_TEST(testEmpty);
	CPPUNIT_TEST(testSerialization);
	CPPUNIT_TEST(testSerialization2);
	CPPUNIT_TEST(testSerialization3);
	CPPUNIT_TEST_SUITE_END();

public:
	// CPPUNIT_NS::TestFixture
	//
	void setUp() {}
	void tearDown() {}

	void testConstructor();
	void testCopyConstructor();
	void testOperatorEqual();
	void testSet();
	void testConversion();
	void testRemove();
	void testEmpty();
	void testSerialization();
	void testSerialization2();
	void testSerialization3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MessageTest);

void MessageTest::testConstructor()
{
	TU_INIT_TESTCASE("testConstructor");
	Message m;
	CPPUNIT_ASSERT(m.IsEmpty());
}

static void InitMessage(Message& m)
{
	m.Set("a", "a1");
	m.Set("b", "b1");
	m.Set("c", "C1");
	m.Set("d", "D1");
}

/*static void DumpMessage(const Message& m)
{
	for (Message::Container::const_iterator it = m.GetContainer().begin();
		it != m.GetContainer().end(); ++it)
	{
		wstring k = (*it).first;
		wstring v = (*it).second;

		if (k != L"kn_payload")
		{
			printf("\t%S=%S\n", k.c_str(), v.c_str());
		}
		else
		{
			printf("\tkn_payload (%d)", v.length());

			for (int i = 0; i < v.length(); i++)
			{
				if (i % 8 == 0)
					printf("\n\t ");
				wchar_t c = v.at(i);
				printf("%04X ", c);
			}
			printf("\n");
		}
	}
}//*/

void MessageTest::testCopyConstructor()
{
	TU_INIT_TESTCASE("testCopyConstructor");
	Message m1;
	CPPUNIT_ASSERT(m1.IsEmpty());
	InitMessage(m1);
	TU_dumpMsg("Message 1",m1);

	Message m2(m1);
	CPPUNIT_ASSERT(!m2.IsEmpty());
	TU_dumpMsg("Message 2",m2);

	CPPUNIT_ASSERT(m1 == m2);
}

void MessageTest::testOperatorEqual()
{
	TU_INIT_TESTCASE("testOperatorEqual");
	Message m1;
	CPPUNIT_ASSERT(m1.IsEmpty());
	InitMessage(m1);
	TU_dumpMsg("Message 1",m1);

	Message m2;
	CPPUNIT_ASSERT(m2.IsEmpty());
	m2 = m1;
	CPPUNIT_ASSERT(!m2.IsEmpty());
	TU_dumpMsg("Message 2",m2);

	CPPUNIT_ASSERT(m1 == m2);
}

void MessageTest::testSet()
{
	TU_INIT_TESTCASE("testSet");
	string f = "Field1";
	string v = "Value1";

	wstring F = L"Field1";
	wstring V = L"Value1";

	Message m1;
	m1.Set(L"Hello", L"f");
	CPPUNIT_ASSERT(!m1.IsEmpty());

	Message m2;
	m2.Set("Hello", "f");
	CPPUNIT_ASSERT(m1 == m2);
}

void MessageTest::testConversion()
{
	TU_INIT_TESTCASE("testConversion");
	string h = "Hello";
	wstring H = L"Hello";

	wstring t = ConvertToWide(h);
	CPPUNIT_ASSERT(t == H);
	string t2 = ConvertToNarrow(t);
	CPPUNIT_ASSERT(t2 == h);
}

void MessageTest::testRemove()
{
	TU_INIT_TESTCASE("testRemove");
	Message m1;
	InitMessage(m1);
	m1.Remove("a");
	m1.Remove("d");

	Message m2;
	m2.Set("b", "b1");
	m2.Set("c", "C1");

	CPPUNIT_ASSERT(m1 == m2);
}

void MessageTest::testEmpty()
{
	TU_INIT_TESTCASE("testEmpty");
	Message m1;
	CPPUNIT_ASSERT(m1.IsEmpty());

	InitMessage(m1);
	CPPUNIT_ASSERT(!m1.IsEmpty());

	m1.Empty();
	CPPUNIT_ASSERT(m1.IsEmpty());
}

void MessageTest::testSerialization()
{
	TU_INIT_TESTCASE("testSerialization");
	Message m;
	Message m2;
	string s;
	bool b;

	m.Set("a", "a1");
	m.Set("b", "b1");
	m.Set("c", "C1");
	m.Set("d", "D1");

	s = m.GetAsSimpleFormat();
	b = m2.InitFromSimple(s);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(m == m2);

	m.Empty();
	m2.Empty();
	CPPUNIT_ASSERT(m.IsEmpty());
	CPPUNIT_ASSERT(m2.IsEmpty());

	m.Set("a", "a1");
	m.Set("b", "b1");
	m.Set("c", "C1");
	m.Set("d", "D1");
	m.Set("kn_payload", "Hello World");

	s = m.GetAsSimpleFormat();
	b = m2.InitFromSimple(s);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(m == m2);


	TU_dumpMsg("Message 1",m);
	TU_dumpMsg("Message 2",m2);


	m.Empty();
	m2.Empty();
	CPPUNIT_ASSERT(m.IsEmpty());
	CPPUNIT_ASSERT(m2.IsEmpty());

	m.Set("kn_payload", "Hello World");

	s = m.GetAsSimpleFormat();
	b = m2.InitFromSimple(s);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(m == m2);
}
	
/*
 * Test a message with a new-line.
 */
void MessageTest::testSerialization2()
{
	TU_INIT_TESTCASE("testSerialization2");
	Message m;
	Message m2;
	string s;
	bool b;

	m.Set("a", "a1");
	m.Set("b", "");
	m.Set("c", "%");
	m.Set("d", "D1\nD2");

	s = m.GetAsSimpleFormat();
	b = m2.InitFromSimple(s);
	CPPUNIT_ASSERT(b);

	TU_dumpMsg("Message 1",m);
	TU_dumpMsg("Message 2",m2);


	CPPUNIT_ASSERT(m == m2);
}

/*
 * This test looks incomplete. 
 */
void MessageTest::testSerialization3()
{
	TU_INIT_TESTCASE("testSerialization3");
	Message m;
	Message m2;
	char tmp[258];
	string s;
	bool b;

	ZeroMemory(tmp, 258);

	for (int i = 0; i < 256; i++)
	{
		tmp[i] = (unsigned char)i;
	}

	string v(tmp, 256);
	wstring v2;

	m.Set("kn_payload", v);
	if (m.Get("kn_payload", v2))
	{
		string v3 = ConvertToNarrow(v2);
		CPPUNIT_ASSERT(v3 == v);
	}

	s = m.GetAsSimpleFormat();
	b = m2.InitFromSimple(s);
	CPPUNIT_ASSERT(b);

	if (m2.Get("kn_payload", v2))
	{
		string v3 = ConvertToNarrow(v2);
		CPPUNIT_ASSERT(v3 == v);
	}

	TU_dumpMsg("Message 1",m);
	TU_dumpMsg("Message 2",m2);


	CPPUNIT_ASSERT(m == m2);
}
	


