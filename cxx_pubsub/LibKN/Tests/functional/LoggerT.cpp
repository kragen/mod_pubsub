#include "stdafx.h"
#include "globals.h"
#include "testutil.h"
#include <LibKN\Logger.h>

class LoggerTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(LoggerTest);
	CPPUNIT_TEST(testLog);
	CPPUNIT_TEST(testTab);
	CPPUNIT_TEST_SUITE_END();

public:
	// CPPUNIT_NS::TestFixture
	//
	void setUp() {}
	void tearDown() {}

	void testLog();
	void testTab();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LoggerTest);

void LoggerTest::testLog()
{
	TU_INIT_TESTCASE("testLog");
	Logger::GetLogger().Log("TestUnit1", Logger::Mask::All, "B");
	Logger::GetLogger().Log("TestUnit1", Logger::Mask::None, "B");
	Logger::GetLogger().IncreaseTabLevel();
	Logger::GetLogger().Log("TestUnit1", Logger::Mask::Warning | Logger::Mask::Critical, "B");
	Logger::GetLogger().DecreaseTabLevel();
}

void LoggerTest::testTab()
{
	TU_INIT_TESTCASE("testTab");
	AutoMethod("LoggerTest", "testTab");

	Logger::GetLogger().Log("TestUnit2", Logger::Mask::All, "Enter");
	Logger::GetLogger().Log("TestUnit2", Logger::Mask::None, "C");

	{
		AutoTabIncr();
		Logger::GetLogger().Log("TestUnit2", Logger::Mask::Warning | Logger::Mask::Critical, "C");
	}

	Logger::GetLogger().Log("TestUnit2", Logger::Mask::All, "Leave");

}


