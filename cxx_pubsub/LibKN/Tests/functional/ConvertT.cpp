#include "stdafx.h"
#include "globals.h"
#include "testutil.h"
#include <LibKN\StrUtil.h>

class ConvertTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(ConvertTest);
	CPPUNIT_TEST(testWide);
	CPPUNIT_TEST(testNarrow);
	CPPUNIT_TEST(testString);
	CPPUNIT_TEST(testUtf8);
	CPPUNIT_TEST(testFromUtf8);
	CPPUNIT_TEST(testWide_0);
	CPPUNIT_TEST(testNarrow_0);
	CPPUNIT_TEST(testString_0);
	CPPUNIT_TEST(testUtf8_0);
	CPPUNIT_TEST(testFromUtf8_0);
	CPPUNIT_TEST_SUITE_END();

public:
	// CPPUNIT_NS::TestFixture
	//
	void setUp() {}
	void tearDown() {}

	void testWide();
	void testNarrow();
	void testString();
	void testUtf8();
	void testFromUtf8();

	void testWide_0();
	void testNarrow_0();
	void testString_0();
	void testUtf8_0();
	void testFromUtf8_0();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConvertTest);

void ConvertTest::testWide()
{
	TU_INIT_TESTCASE("testWide");
	wstring a = L"This is a string";
	string b = "This is a string";

	wstring c = ConvertToWide(b);
	CPPUNIT_ASSERT(a == c);
}

void ConvertTest::testNarrow()
{
	TU_INIT_TESTCASE("testNarrow");
	wstring a = L"This is a string";
	string b = "This is a string";

	string c = ConvertToNarrow(a);
	CPPUNIT_ASSERT(b == c);
}

void ConvertTest::testString()
{
	TU_INIT_TESTCASE("testString");
	wstring a = L"This is a string";
	string b = "This is a string";

#if defined(_UNICODE) || defined(UNICODE)
	tstring c = ConvertToTString(a);
	tstring d = ConvertToTString(b);
	CPPUNIT_ASSERT(a == c);
	CPPUNIT_ASSERT(a == d);
	TU_dump("UNICODE");
#else
	tstring c = ConvertToTString(a);
	tstring d = ConvertToTString(b);
	CPPUNIT_ASSERT(b == c);
	CPPUNIT_ASSERT(b == d);
	TU_dump("NOT UNICODE");
#endif
}

void ConvertTest::testUtf8()
{
	TU_INIT_TESTCASE("testUtf8");
	wstring a = L"This is a string";
	string b = "This is a string";

	string c = ConvertToUtf8(a);
	CPPUNIT_ASSERT(b == c);
}

void ConvertTest::testFromUtf8()
{
	TU_INIT_TESTCASE("testFromUtf8");
	wstring a = L"This is a string";
	string b = "This is a string";

	wstring c = ConvertFromUtf8(b);
	CPPUNIT_ASSERT(a == c);
}

// Zero lenght testing

void ConvertTest::testWide_0()
{
	TU_INIT_TESTCASE("testWide_0");
	wstring a = L"";
	string b = "";

	wstring c = ConvertToWide(b);
	CPPUNIT_ASSERT(a == c);
}

void ConvertTest::testNarrow_0()
{
	TU_INIT_TESTCASE("testNarrow_0");
	wstring a = L"";
	string b = "";

	string c = ConvertToNarrow(a);
	CPPUNIT_ASSERT(b == c);
}

void ConvertTest::testString_0()
{
	TU_INIT_TESTCASE("testString_0");
	wstring a = L"";
	string b = "";

#if defined(_UNICODE) || defined(UNICODE)
	tstring c = ConvertToTString(a);
	tstring d = ConvertToTString(b);
	CPPUNIT_ASSERT(a == c);
	CPPUNIT_ASSERT(a == d);
	TU_dump("UNICODE");
#else
	tstring c = ConvertToTString(a);
	tstring d = ConvertToTString(b);
	CPPUNIT_ASSERT(b == c);
	CPPUNIT_ASSERT(b == d);
	TU_dump("NOT UNICODE");
#endif
}

void ConvertTest::testUtf8_0()
{
	TU_INIT_TESTCASE("testUtf8_0");
	wstring a = L"";
	string b = "";

	string c = ConvertToUtf8(a);
	CPPUNIT_ASSERT(b == c);
}

void ConvertTest::testFromUtf8_0()
{
	TU_INIT_TESTCASE("testFromUtf8_0");
	wstring a = L"";
	string b = "";

	wstring c = ConvertFromUtf8(b);
	CPPUNIT_ASSERT(a == c);
}



