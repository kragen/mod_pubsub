#include "stdafx.h"
#include <LibKN\StrUtil.h>

class ConvertTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(ConvertTest);
	CPPUNIT_TEST(testWide);
	CPPUNIT_TEST(testNarrow);
	CPPUNIT_TEST_SUITE_END();

public:
	// CPPUNIT_NS::TestFixture
	//
	void setUp() {}
	void tearDown() {}

	void testWide();
	void testNarrow();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConvertTest);

void ConvertTest::testWide()
{
	wstring a = L"This is a string";
	string b = "This is a string";

	wstring c = ConvertToWide(b);
	CPPUNIT_ASSERT(a == c);
}

void ConvertTest::testNarrow()
{
	wstring a = L"This is a string";
	string b = "This is a string";

	string c = ConvertToNarrow(a);
	CPPUNIT_ASSERT(b == c);
}

