/**
 * Test utilities.
 * <P>
 * All functions that begin with uppercase TU_ can be used in test suites, functions
 * that begin with lower case tu_ are meant to by used internally to support the utilities.
 * <P>
 * To convert a set of testsuites make the following changes to the file that executes
 * the testsuites:
 * <PRE>
 *   #include "stdafx.h"
 *   #include "Globals.h"    //TestUtil.
 *   #include "TestUtil.h"   //TestUtil.
 *   #include <stdio.h>
 *   #include <vector>
 *   using std::vector;
 *   using std::cout;
 *   
 *   int main(int argc, char* argv[])
 *   {
 *       // Process the arguments                        //TestUtil.
 *       string error = TU_INIT_TESTSUITE(argc, argv);   //TestUtil.
 *       if(error.size() != 0)                           //TestUtil.
 *       {                                               //TestUtil.
 *           cout << error;                              //TestUtil.
 *           exit(1);                                    //TestUtil.
 *       }                                               //TestUtil.
 *   
 *       // Get the top level suite from the registry
 *       CPPUNIT_NS::Test* suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
 *   	
 *       // Adds the test to the list of test to run
 *       CPPUNIT_NS::TextUi::TestRunner runner;
 *       runner.addTest(suite);
 *   	
 *       // Change the default outputter to a compiler error format outputter
 *       runner.setOutputter(new CPPUNIT_NS::CompilerOutputter(&runner.result(), std::cerr));
 *   	
 *       // Run the test.
 *       bool wasSucessful = runner.run();
 *   	
 *       // Return error code 1 if the one of test failed.
 *       return wasSucessful ? 0 : 1;
 *   }
 * </PRE>
 *
 * To minimally convert to these tools all that is needed add the following to each
 * CPPUNIT_NS::TestFixture file:
 * <PRE>
 *   #include "globals.h"
 *   #include "testutil.h"
 * </PRE>
 *
 * And add TU_INIT_TESTCASE() to the first line to every testcase of the TestFixture.
 * <BR>Example:
 * <PRE>
 * void TestSuite1::testCase1()
 * {
 *     TU_INIT_TESTCASE("testCase1");
 *     . . .
 * </PRE>
 *
 * Now you can use the -tests parameter (see help in TestUtil.cpp or execute the new
 * test runner executable with the param -help) to run a single test or a list of tests.
 * <P>
 * To make use of the -verbose parameter us TU_dump functions in your tests rather than
 * cout or printf().
 * <P>
 * You can also use or add other parameters to control runtime values such as server URIs
 * and user names.
 * <P>
 * So far all the tests have been converted partially to use -tests. The following table lists
 * work done so far:
 * <PRE>
 *  Test name           -tests   complete
 *  ------------------  -------  --------
 *  ConnectorT          Yes      Yes
 *  ConnectorT2         Yes      No
 *  ConnectorT3         Yes      No
 *  ConnectorT4         Yes      No
 *  ConnectorT5         Yes      No
 *  ConvertT            Yes      Yes
 *  LoggerT             Yes      Yes
 *  MessageT            Yes      Yes
 *  MessageT2           Yes      Yes
 *  QueueT              Yes      Yes
 *  TransportT          Yes      Yes
 *  Utf8Test            Yes      Yes
 * </PRE>
 */
#if !defined(TESTUTIL_H)
#define TESTUTIL_H


#include "globals.h"
#include "stdafx.h"
#include <LibKN\Connector.h>
#include <LibKN\StrUtil.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <vector>
using std::vector;
using std::cout;


#define TU_INIT_TESTSUITE(i,j)	tu_initTestSuite(i,j,__FILE__)
#define TU_INIT_TESTCASE(i)		if(!tu_initTestCase(i,__FILE__)) return


/** 
 * Use: TU_INIT_TESTSUITE(i,j)
 * <P>
 * Should only be called by the .exe file.
 */
extern string tu_initTestSuite(int argc_, char* argv_[], const char * file);

/** 
 * Use: TU_INIT_TESTCASE(i)
 * <P>
 * Initializes a test case. Should be called at the begining of each test case.
 * <P>
 * Example:<BR>
 * TU_INIT_TESTCASE("testPubSub");
 */
extern bool tu_initTestCase(const char * testCase, const char * file); 

/**
 * Returns the server URI input by the parameter -server or the 
 * DEFAULT_SERVER_URI from the Globals.h file.
 * <P>
 * If there is a list if server URIs then the first one is returned.
 */
extern string TU_getServerURI();

/**
 * Returns the nth server URI input by the parameter -server or an empty string.
 */
extern string TU_getServerURI(int index);

/**
 * Returns the server URI input by the parameter -server or an empty string.
 * <P>
 * If there is a list if server URIs then the first one is returned.
 */
extern wstring TU_getwServerURI();

/**
 * Returns the server URI input by the parameter -server or the 
 * DEFAULT_SERVER_URI fro the Globals.h file.
 * <P>
 * If there is a list if server URIs then the first one is returned.
 */
extern wstring TU_getwServerURI(int index);

/**
 * Returns the server name input by the parameter -serverName.
 * Or the DEFAULT_SERVER_NAME from the Globals.h file.
 */
extern string TU_getServerName();

/**
 * Returns the server name input by the parameter -serverName.
 * Or the DEFAULT_SERVER_NAME from the Globals.h file.
 */
extern wstring TU_getwServerName();

/**
 * Stops the server. Useful for Queue testing.
 * 
 * <PRE>
 * To get a list of services go to:
 * Contol Panel - Administration - Services
 * </PRE>
 */
extern void TU_StopKnowNowServer();

/**
 * Starts the server. Useful for Queue testing.
 * 
 * <PRE>
 * To get a list of services go to:
 * Contol Panel - Administration - Services
 * </PRE>
 */
extern void TU_StartKnowNowServer();


/** 
 * Outputs a char string if -verbose is on.
 */
extern void TU_dump(const char * text); 

/** 
 * Outputs string if -verbose is on.
 */
extern void TU_dump(string text); 

/** 
 * Outputs wstring if -verbose is on.
 */
extern void TU_dump(wstring text);

/** 
 * Outputs char string and a decimal integer if -verbose is on.
 */
extern void TU_dump(const char* text, int i);
extern void TU_dump(const char* text, string str);
extern void TU_dump(const char* text, wstring wstr);

extern void TU_dumpHex(const char* text, const string& str);
extern void TU_dumpHex(const char* text, const wstring& wstr);

extern void TU_dumpMsg(const char* text, const Message& msg);
extern void TU_dumpMsgKey(const char* text, const Message& msg, wstring key);

extern void TU_dumpPubPayloads();
extern void TU_dumpSubPayloads();
extern void TU_resetPubPayloads();
extern void TU_resetSubPayloads();
extern void TU_addPubPayload(wstring pl);
extern void TU_addSubPayload(wstring pl);
extern wstring TU_getPubPayload(int index);
extern wstring TU_getSubPayload(int index);

extern void TU_resetCounters();
extern void TU_dumpCounters();
extern int TU_getCount_pubConnSH_OnConnStatus();
extern int TU_getCount_pubReqSH_OnError();
extern int TU_getCount_pubReqSH_OnSuccess();
extern int TU_getCount_subListener_OnUpdate();
extern int TU_getCount_subConnSH_OnConnStatus();
extern int TU_getCount_subReqSH_OnError();
extern int TU_getCount_subReqSH_OnSuccess();


class TU_SubListener : public IListener
{
public:
	void OnUpdate(const Message& msg);
};

class TU_SubConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg);
};

class TU_PubConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg);
};

class TU_SubRequestStatusHandler : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg);
	void OnSuccess(const Message& msg);
};

class TU_PubRequestStatusHandler : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg);
	void OnSuccess(const Message& msg);
};



#endif // !defined(TESTUTIL_H)

