#include "stdafx.h"
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
};

CPPUNIT_TEST_SUITE_REGISTRATION(MessageTest);

void MessageTest::testConstructor()
{
	Message m;
	CPPUNIT_ASSERT(m.HasItemsChanged());
	CPPUNIT_ASSERT(m.IsEmpty());
}

void InitMessage(Message& m)
{
	m.Set("a", "a1");
	m.Set("b", "b1");
	m.Set("c", "C1");
	m.Set("d", "D1");
}

void DumpMessage(const Message& m)
{
	for (Message::Container::const_iterator it = m.GetContainer().begin();
		it != m.GetContainer().end(); ++it)
	{
		wstring k = (*it).first;
		wstring v = (*it).second;

		printf("\t%S=%S\n", k.c_str(), v.c_str());
	}
}

void MessageTest::testCopyConstructor()
{
	Message m1;
	CPPUNIT_ASSERT(m1.IsEmpty());
	InitMessage(m1);
	CPPUNIT_ASSERT(m1.HasItemsChanged());
//	DumpMessage(m1);
	Message m2(m1);
	CPPUNIT_ASSERT(!m2.IsEmpty());
	CPPUNIT_ASSERT(m2.HasItemsChanged());
//	DumpMessage(m2);

	CPPUNIT_ASSERT(m1 == m2);
}

void MessageTest::testOperatorEqual()
{
	Message m1;
	CPPUNIT_ASSERT(m1.IsEmpty());
	InitMessage(m1);
//	DumpMessage(m1);
	CPPUNIT_ASSERT(m1.HasItemsChanged());

	Message m2;
	CPPUNIT_ASSERT(m2.IsEmpty());
	m2 = m1;
	CPPUNIT_ASSERT(!m2.IsEmpty());
//	DumpMessage(m2);
	CPPUNIT_ASSERT(m2.HasItemsChanged());

	CPPUNIT_ASSERT(m1 == m2);
}

void MessageTest::testSet()
{
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
	string h = "Hello";
	wstring H = L"Hello";

	wstring t = ConvertToWide(h);
	CPPUNIT_ASSERT(t == H);
	string t2 = ConvertToNarrow(t);
	CPPUNIT_ASSERT(t2 == h);
}

void MessageTest::testRemove()
{
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
	Message m1;
	CPPUNIT_ASSERT(m1.IsEmpty());
	InitMessage(m1);
	CPPUNIT_ASSERT(!m1.IsEmpty());
	m1.Empty();
	CPPUNIT_ASSERT(m1.IsEmpty());
}

void MessageTest::testSerialization()
{
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


#if 0
	printf("\n<m\n");
	DumpMessage(m);
	printf("m>\n");
#endif
#if 0
	printf("\n<m2\n");
	DumpMessage(m2);
	printf("m2>\n");
#endif

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
	
