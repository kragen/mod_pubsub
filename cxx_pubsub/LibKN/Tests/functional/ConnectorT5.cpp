#include "stdafx.h"
#include "globals.h"
#include "testutil.h"
#include <iostream>
#include <cstdlib>
#include <LibKN\Connector.h>
#include <LibKN\Message.h>
#include <LibKN\StrUtil.h>
#include <vector>
using std::vector;
using std::cout;

// If you want to leave in calls to ConT5_Dump_, this must be defined to have output.
// At this time we are commenting out all calls to ConT5_Dump_.
#define VERBOSE

// Define this if you want to abridge large hex dumps. It restricts to first and 
// last 4 lines a large hex dump.
#define ABRIDGE_LARGE_DUMP

// Define this if you want hex dumped as well as strings. 
// Good for non-printing characters.
#define DUMP_HEX

/**
 * These test Publish() and Subscribe() with all possible character data 
 * for user defined message fields:
 * 1. ???All 8 bit characters: 00 to FF
 * 2. ???All 16 bit unicode non-surrogate characters: 0000 to D7FF and E000 to FFFF.
 * 3. Unicode surrogate characters are not test at this time.
 *
 * The test LibKN/Tests/functional/ConnectorT5.cpp has parallel tests for 
 * Publish() and Subscribe() kn_payload values.
 * The test LibKN/Tests/functional/MessageT2.cpp has parallel tests for Message().
 */
class ConnectorTest5 : public CPPUNIT_NS::TestFixture
{

	CPPUNIT_TEST_SUITE(ConnectorTest5);
	CPPUNIT_TEST(testPubSub_FieldKeyboard);
	CPPUNIT_TEST(testPubSub_wFieldKeyboard);
	CPPUNIT_TEST(testPubSub_Field8bit);
	CPPUNIT_TEST(testPubSub_Field16bit);
	CPPUNIT_TEST_SUITE_END();

public:

	// CPPUNIT_NS::TestFixture
	//
	void setUp() {}
	void tearDown() {}

	void testPubSub_FieldKeyboard();
	void testPubSub_wFieldKeyboard();
	void testPubSub_Field8bit();
	void testPubSub_Field16bit();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConnectorTest5);

// Variables //////////////////////////////////////////////////////////////////

const string  VALUE  =  "<<<VALUE>>>"; // Use a constant value to search for.
const wstring VALUEw = L"<<<VALUE>>>"; // Use a constant value to search for.

int conT5_count_subListener_OnUpdate = 0;
int conT5_count_subConnSH_OnConnStatus = 0;
int conT5_count_subReqSH_OnStatus = 0;
int conT5_count_subReqSH_OnError = 0;
int conT5_count_subReqSH_OnSuccess = 0;
int conT5_count_pubConnSH_OnConnStatus = 0;
int conT5_count_pubReqSH_OnStatus = 0;
int conT5_count_pubReqSH_OnError = 0;
int conT5_count_pubReqSH_OnSuccess = 0;

vector<string>   sPubFields;
vector<string>   sSubFields;
vector<wstring> wsPubFields;
vector<wstring> wsSubFields;


// Tools //////////////////////////////////////////////////////////////////////

void ConT5_DumpInit()
{
#if defined(VERBOSE)
	printf("\n");
#endif
}

void ConT5_DumpStr(const char* text)
{
#if defined(VERBOSE)
	printf("%s\n", text);
#endif
}

void ConT5_DumpStr(const char* text, int i)
{
#if defined(VERBOSE)
	printf("%s%d\n", text, i);
#endif
}

void ConT5_DumpStr(string text)
{
#if defined(VERBOSE)
	printf("%s\n", text.c_str());
#endif
}

void ConT5_DumpStr(wstring text)
{
#if defined(VERBOSE)
	printf("%S\n", text.c_str());
#endif
}

static void ConT5_Dump_string(const string& msg, const string& s)
{
#if defined(VERBOSE)
	printf("%s (%d)", msg.c_str(), s.length());

#if defined(ABRIDGE_LARGE_DUMP)
	for (int i = 0; i < min(s.length(),4*16); i++)
	{
		if (i % 16 == 0)
			printf("\n");
		else
			printf(" ");
		unsigned char c = s.at(i);
		printf("  %02X", c);
	}
	if(s.length() > 4*16)
		printf("\n. . . ABRIDGE_LARGE_DUMP is defined.");
	for (i = max(s.length()-(4*16),4*16); i < s.length(); i++)
	{
		if (i % 16 == 0)
			printf("\n");
		else
			printf(" ");
		unsigned char c = s.at(i);
		printf("  %02X", c);
	}
#else
	for (int i = 0; i < s.length(); i++)
	{
		if (i % 16 == 0)
			printf("\n");
		else
			printf(" ");
		unsigned char c = s.at(i);
		printf("  %02X", c);
	}
#endif
	printf("\n");
#endif
}

static void ConT5_Dump_wstring(const string& msg, const wstring& s)
{
#if defined(VERBOSE)
	printf("%s (%d)", msg.c_str(), s.length());

#if defined(ABRIDGE_LARGE_DUMP)
	for (int i = 0; i < min(s.length(),4*16); i++)
	{
		if (i % 16 == 0)
			printf("\n");
		else
			printf(" ");
		unsigned int c = s.at(i);
		printf("%04X", c);
	}
	if(s.length() > 4*16)
		printf("\n. . . ABRIDGE_LARGE_DUMP is defined.");
	for (i = max(s.length()-(4*16),4*16); i < s.length(); i++)
	{
		if (i % 16 == 0)
			printf("\n");
		else
			printf(" ");
		unsigned int c = s.at(i);
		printf("%04X", c);
	}
#else
	for (int i = 0; i < s.length(); i++)
	{
		if (i % 16 == 0)
			printf("\n");
		else
			printf(" ");
		unsigned int c = s.at(i);
		printf("%04X", c);
	}
#endif
	printf("\n");
#endif
}

void ConT5_DumpMsg(char * text, const Message& m)
{
#if defined(VERBOSE)
	printf("%s: Message:\n",text);
	for (Message::Container::const_iterator it = m.GetContainer().begin();
		it != m.GetContainer().end(); ++it)
	{
		wstring k = (*it).first;
		wstring v = (*it).second;

		printf("\t%S=%S\n", k.c_str(), v.c_str());
#if defined(DUMP_HEX)
		ConT5_Dump_wstring("\tField hex:", k);
		ConT5_Dump_wstring("\tValue hex:", v);
#endif
	}
	printf("\n");
#endif
}

void ConT5_ResetCounters()
{
	conT5_count_pubConnSH_OnConnStatus = 0;
	conT5_count_pubReqSH_OnStatus = 0;
	conT5_count_pubReqSH_OnError = 0;
	conT5_count_pubReqSH_OnSuccess = 0;
	conT5_count_subListener_OnUpdate = 0;
	conT5_count_subConnSH_OnConnStatus = 0;
	conT5_count_subReqSH_OnStatus = 0;
	conT5_count_subReqSH_OnError = 0;
	conT5_count_subReqSH_OnSuccess = 0;
}

void ConT5_DumpCounters()
{
#if defined(VERBOSE)
	printf("conT5_count_pubConnSH_OnConnStatus=%d\n",conT5_count_pubConnSH_OnConnStatus);
	printf("conT5_count_pubReqSH_OnStatus     =%d\n",conT5_count_pubReqSH_OnStatus);
	printf("conT5_count_pubReqSH_OnError      =%d\n",conT5_count_pubReqSH_OnError);
	printf("conT5_count_pubReqSH_OnSuccess    =%d\n",conT5_count_pubReqSH_OnSuccess);
	printf("conT5_count_subListener_OnUpdate  =%d\n",conT5_count_subListener_OnUpdate);
	printf("conT5_count_subConnSH_OnConnStatus=%d\n",conT5_count_subConnSH_OnConnStatus);
	printf("conT5_count_subReqSH_OnStatus     =%d\n",conT5_count_subReqSH_OnStatus);
	printf("conT5_count_subReqSH_OnError      =%d\n",conT5_count_subReqSH_OnError);
	printf("conT5_count_subReqSH_OnSuccess    =%d\n",conT5_count_subReqSH_OnSuccess);
#endif
}

void ConT5_DumpFields()
{
#if defined(VERBOSE)
	int i = 0;
	cout << "Pub Fields<string>: " << sPubFields.size() << "\n";
	for( i = 0; i < sPubFields.size(); i++)
	{
		ConT5_Dump_string("",sPubFields[i]);
	}
	cout << "Pub Fields<wstring>: " << wsPubFields.size() << "\n";
	for( i = 0; i < wsPubFields.size(); i++)
	{
		ConT5_Dump_wstring("",wsPubFields[i]);
	}
	cout << "Sub Fields<string>: " << sSubFields.size() << "\n";
	for( i = 0; i < sSubFields.size(); i++)
	{
		ConT5_Dump_string("",sSubFields[i]);
	}
	cout << "Sub Fields<wstring>: " << wsSubFields.size() << "\n";
	for( i = 0; i < wsSubFields.size(); i++)
	{
		ConT5_Dump_wstring("",wsSubFields[i]);
	}
#endif
}

void ConT5_wDumpFields()
{
#if defined(VERBOSE)
	int i = 0;
	cout << "Pub Fields<wstring>: " << wsPubFields.size() << "\n";
	for( i = 0; i < wsPubFields.size(); i++)
	{
		ConT5_Dump_wstring("",wsPubFields[i]);
	}
	cout << "Sub Fields<wstring>: " << wsSubFields.size() << "\n";
	for( i = 0; i < wsSubFields.size(); i++)
	{
		ConT5_Dump_wstring("",wsSubFields[i]);
	}
#endif
}

void ConT5_ResetFields()
{
	int n = sPubFields.size();
	for(int i = 0; i < n; i++)
	{
		sPubFields.pop_back();
	}
	n = wsPubFields.size();
	for( i = 0; i < n; i++)
	{
		wsPubFields.pop_back();
	}
	n = sSubFields.size();
	for( i = 0; i < n; i++)
	{
		sSubFields.pop_back();
	}
	n = wsSubFields.size();
	for( i = 0; i < n; i++)
	{
		wsSubFields.pop_back();
	}
}


// Classes ////////////////////////////////////////////////////////////////////

class ConT5_SubListener : public IListener
{
public:
	void OnUpdate(const Message& msg)
	{
		//ConT5_DumpMsg("<Sub>Listener: OnUpdate", msg);
		//printf("L");
		conT5_count_subListener_OnUpdate++;

		for (Message::Container::const_iterator it = msg.GetContainer().begin();
			it != msg.GetContainer().end(); ++it)
		{
			wstring k = (*it).first;
			wstring v = (*it).second;
			if(v == VALUEw)
			{
				//printf("-");
				//ConT5_Dump_wstring("Field=",k);
				wsSubFields.push_back(k);
			}
		}
	}
};

class ConT5_SubConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg)
	{
		//ConT5_DumpMsg("<Sub>ConnectionStatusHandler: OnConnectionStatus", msg);
		conT5_count_subConnSH_OnConnStatus++;
	}
};

class ConT5_PubConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg)
	{
		//ConT5_DumpMsg("<Pub>ConnectionStatusHandler: OnConnectionStatus", msg);
		conT5_count_pubConnSH_OnConnStatus++;
	}
};

class ConT5_SubRequestStatusHandler : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//ConT5_DumpMsg("<Sub>Request Status Sucess: OnError", msg);
		conT5_count_subReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//ConT5_DumpMsg("<Sub>RequestStatusHandler: OnSuccess", msg);
		conT5_count_subReqSH_OnSuccess++;
	}
};

class ConT5_PubRequestStatusHandler : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//ConT5_DumpMsg("<Pub>RequestStatusHandler: OnError", msg);
		conT5_count_pubReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//ConT5_DumpMsg("<Pub>RequestStatusHandler: OnSuccess", msg);
		conT5_count_pubReqSH_OnSuccess++;
	}
};

// Tests //////////////////////////////////////////////////////////////////////


/**
 * Any keyboard character should be valid for user field names.
 * This test is redundant (testPubSub_Field8bit) but it can be useful for demonstration.
 * You must escape '\' and '"' in C++ code.
 *
 * This tests used string the next test will use wstring.
 */
void ConnectorTest5::testPubSub_FieldKeyboard()
{
	TU_INIT_TESTCASE("testPubSub_FieldKeyboard");
	ConT5_ResetCounters();
	ConT5_ResetFields();
	int i = 0;
	int sleep = 0;

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/Tests/functional/ConnectorT5/testPubSub_FieldKeyboard";

	// Determine the number of messages here to make VERY sure of results.
	int totalMsgs = 0;
	sPubFields.push_back("`1234567890-=~!@#$%^&*()_+[]\\{}|;':\",./<>?");totalMsgs++;
	sPubFields.push_back("'");totalMsgs++;
	sPubFields.push_back("~");totalMsgs++;
	sPubFields.push_back("?");totalMsgs++;
	sPubFields.push_back("|");totalMsgs++;
	sPubFields.push_back("\n");totalMsgs++;
	sPubFields.push_back("\t");totalMsgs++;
	sPubFields.push_back(" ");totalMsgs++;
	sPubFields.push_back("\\");totalMsgs++;
	sPubFields.push_back("/");totalMsgs++;
	//sPubFields.push_back("");totalMsgs++; // This is legal to publish and the event is seen
								// by the subscriber but the propperty value pair is missing. Bug968

	// 01 02 03 04 05 06 07 08 09 10


	// Start the subscriber.
	Connector subConn;
	ConT5_SubListener subListener;
	ConT5_SubRequestStatusHandler subReqSH;
	ITransport::Parameters subITParams;

    // This is only necessary to get subConnSH_OnConnStatus
	ConT5_SubConnStatusHandler subConnSH;
	subConn.AddConnectionStatusHandler(&subConnSH);

	subITParams.m_ServerUrl = serverUrl;
   	if (!subConn.Open(subITParams)){
		CPPUNIT_FAIL("Open(subITParams) failed.");
		return;
	}
	
	wstring rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);
	// ConT5_DumpStr(L"<sub>rid = " + rid);
	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
		return;
	}
	Sleep(100); // Avoid race conditions.



	// Start the publisher.
	Connector pubConn;
	ConT5_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT5_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}
	
	// Publish strings.
	for( i = 0; i < sPubFields.size(); i++)
	{
		Message pubMsg;

		pubMsg.Set("do_method", "notify");
		pubMsg.Set(L"kn_to", topic);
		pubMsg.Set("kn_response_format", "simple");
		//pubMsg.Set("kn_expires", "+300");//5 minutes
		pubMsg.Set(sPubFields[i], VALUE);
		pubConn.Publish(pubMsg, &pubReqSH);
	}



	Sleep(1000);//<<<<<<<<Subscriber still needs time to get pubs (especially if dumping output).



	// Done, disconnect.
	// ConT5_DumpStr("<sub>Unsubscribing.");
	if (!subConn.Unsubscribe(rid, &subReqSH))
	{
		CPPUNIT_FAIL("failed to unsubscribe.");
	}

	pubConn.Close();
	subConn.Close();

	subConn.RemoveConnectionStatusHandler(&subConnSH);



	// Check that pub == sub - Hardcode to make sure.
	char pubBuf[12];
	itoa(totalMsgs,pubBuf,10);
	char gotBuf[12];
	itoa(wsSubFields.size(),gotBuf,10);
	string msg = (string)"If this happens you could have a race condition or other problem.\n" 
			   + (string)"The results will be unpredictable.\n"
			   + (string)"Extending the Sleep() period may be all that is needed.\n"
			   + (string)"Attempted to publish " + pubBuf + " messages.\n"
			   + (string)"Sub got " + gotBuf + " messages.";
	CPPUNIT_ASSERT_MESSAGE(msg, wsSubFields.size() == totalMsgs);

	// Check that all pubs and subs occurred.
	// ConT5_DumpCounters();
	CPPUNIT_ASSERT(conT5_count_pubConnSH_OnConnStatus==1);// 1st Publish()
	CPPUNIT_ASSERT(conT5_count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT5_count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT5_count_pubReqSH_OnSuccess    ==totalMsgs);// Publish() updates
	CPPUNIT_ASSERT(conT5_count_subListener_OnUpdate  ==totalMsgs);// updates
	CPPUNIT_ASSERT(conT5_count_subConnSH_OnConnStatus==1);// 1st Subscribe()
	CPPUNIT_ASSERT(conT5_count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT5_count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT5_count_subReqSH_OnSuccess    ==2);// Subscribe(),Unsubscribe()




	// Check that all pubs == subs

	// Convert the string payloads and push them onto the wstring to compare.
	// ConT5_DumpFields();
	for( i = 0; i < sPubFields.size(); i++)
	{
		wstring tmp = ConvertToWide(sPubFields[i]);
		wsPubFields.push_back(tmp);
	}

	// ConT5_DumpFields();
	for( i = 0; i < sPubFields.size(); i++)
	{	
		// Construct msg in case of error.
		int sent = sPubFields[i].length();
		int got  = wsSubFields[i].length();
		char iBuf[12];
		itoa(i,iBuf,10);
		char sentBuf[12];
		itoa(sent,sentBuf,10);
		char gotBuf[12];
		itoa(got,gotBuf,10);
		string msg = (string)"Field[" + iBuf
				   + (string)"] Sent string length=0x" + sentBuf
				   + (string)" ,got length=0x" + gotBuf;

		//ConT5_DumpStr(">>>Pub Field size =",wsPubFields[i].length());
		//ConT5_DumpStr("   Sub Field size =",wsSubFields[i].length());
		CPPUNIT_ASSERT_MESSAGE(msg,wsPubFields[i] == wsSubFields[i]);
	}
}

/**
 * Any keyboard character should be valid for user field names.
 * This test is redundant (testPubSub_Field8bit) but it can be useful for demonstration.
 * You must escape '\' and '"' in C++ code.
 *
 * This tests uses wstring the previous test uses string.
 */
void ConnectorTest5::testPubSub_wFieldKeyboard()
{
	TU_INIT_TESTCASE("testPubSub_wFieldKeyboard");
	ConT5_ResetCounters();
	ConT5_ResetFields();
	int i = 0;
	int sleep = 0;

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/Tests/functional/ConnectorT5/testPubSub_wFieldKeyboard";

	// Determine the number of messages here to make VERY sure of results.
	int totalMsgs = 0;
	wsPubFields.push_back(L"`1234567890-=~!@#$%^&*()_+[]\\{}|;':\",./<>?");totalMsgs++;
	wsPubFields.push_back(L"'");totalMsgs++;
	wsPubFields.push_back(L"~");totalMsgs++;
	wsPubFields.push_back(L"?");totalMsgs++;
	wsPubFields.push_back(L"|");totalMsgs++;
	wsPubFields.push_back(L"\n");totalMsgs++;
	wsPubFields.push_back(L"\t");totalMsgs++;
	wsPubFields.push_back(L" ");totalMsgs++;
	wsPubFields.push_back(L"\\");totalMsgs++;
	wsPubFields.push_back(L"/");totalMsgs++;
	//wsPubFields.push_back("");totalMsgs++; // This is legal to publish and the event is seen
								// by the subscriber but the propperty value pair is missing. Bug968

	// 01 02 03 04 05 06 07 08 09 10


	// Start the subscriber.
	Connector subConn;
	ConT5_SubListener subListener;
	ConT5_SubRequestStatusHandler subReqSH;
	ITransport::Parameters subITParams;

    // This is only necessary to get subConnSH_OnConnStatus
	ConT5_SubConnStatusHandler subConnSH;
	subConn.AddConnectionStatusHandler(&subConnSH);

	subITParams.m_ServerUrl = serverUrl;
   	if (!subConn.Open(subITParams)){
		CPPUNIT_FAIL("Open(subITParams) failed.");
		return;
	}
	
	wstring rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);
	// ConT5_DumpStr(L"<sub>rid = " + rid);
	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
		return;
	}
	Sleep(100); // Avoid race conditions.



	// Start the publisher.
	Connector pubConn;
	ConT5_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT5_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}
	
	// Publish strings.
	for( i = 0; i < wsPubFields.size(); i++)
	{
		Message pubMsg;

		pubMsg.Set("do_method", "notify");
		pubMsg.Set(L"kn_to", topic);
		pubMsg.Set("kn_response_format", "simple");
		//pubMsg.Set("kn_expires", "+300");//5 minutes
		pubMsg.Set(wsPubFields[i], VALUEw);
		pubConn.Publish(pubMsg, &pubReqSH);
	}



	Sleep(1000);//<<<<<<<<Subscriber still needs time to get pubs (especially if dumping output).



	// Done, disconnect.
	// ConT5_DumpStr("<sub>Unsubscribing.");
	if (!subConn.Unsubscribe(rid, &subReqSH))
	{
		CPPUNIT_FAIL("failed to unsubscribe.");
	}

	pubConn.Close();
	subConn.Close();

	subConn.RemoveConnectionStatusHandler(&subConnSH);



	// Check that pub == sub - Hardcode to make sure.
	char pubBuf[12];
	itoa(totalMsgs,pubBuf,10);
	char gotBuf[12];
	itoa(wsSubFields.size(),gotBuf,10);
	string msg = (string)"If this happens you could have a race condition or other problem.\n" 
			   + (string)"The results will be unpredictable.\n"
			   + (string)"Extending the Sleep() period may be all that is needed.\n"
			   + (string)"Attempted to publish " + pubBuf + " messages.\n"
			   + (string)"Sub got " + gotBuf + " messages.";
	CPPUNIT_ASSERT_MESSAGE(msg, wsSubFields.size() == totalMsgs);

	// Check that all pubs and subs occurred.
	// ConT5_DumpCounters();
	CPPUNIT_ASSERT(conT5_count_pubConnSH_OnConnStatus==1);// 1st Publish()
	CPPUNIT_ASSERT(conT5_count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT5_count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT5_count_pubReqSH_OnSuccess    ==totalMsgs);// Publish() updates
	CPPUNIT_ASSERT(conT5_count_subListener_OnUpdate  ==totalMsgs);// updates
	CPPUNIT_ASSERT(conT5_count_subConnSH_OnConnStatus==1);// 1st Subscribe()
	CPPUNIT_ASSERT(conT5_count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT5_count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT5_count_subReqSH_OnSuccess    ==2);// Subscribe(),Unsubscribe()

	// Check that all pubs == subs
	// ConT5_DumpFields();
	for( i = 0; i < wsPubFields.size(); i++)
	{	
		// Construct msg in case of error.
		int sent = wsPubFields[i].length();
		int got  = wsSubFields[i].length();
		char iBuf[12];
		itoa(i,iBuf,10);
		char sentBuf[12];
		itoa(sent,sentBuf,10);
		char gotBuf[12];
		itoa(got,gotBuf,10);
		string msg = (string)"Field[" + iBuf
				   + (string)"] Sent string length=0x" + sentBuf
				   + (string)" ,got length=0x" + gotBuf;

		//ConT5_DumpStr(">>>Pub Field size =",wsPubFields[i].length());
		//ConT5_DumpStr("   Sub Field size =",wsSubFields[i].length());
		CPPUNIT_ASSERT_MESSAGE(msg,wsPubFields[i] == wsSubFields[i]);
	}
}

/**
 * Test all 8 bit char values for property names. All should be valid.
 * There is no need to test wstring because the 16bit test will.
 */
void ConnectorTest5::testPubSub_Field8bit()
{
	TU_INIT_TESTCASE("testPubSub_Field8bit");
	ConT5_ResetCounters();
	ConT5_ResetFields();
	int i = 0;
	int sleep = 0;
///*
	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/Tests/functional/ConnectorT5/testPubSub_Value8bit";

	// Determine the number of messages here to make VERY sure of results.
	int totalMsgs = 0;

	char tmp[258];
	ZeroMemory(tmp, 258);
	for ( i = 0; i < 256; i++){
		tmp[i] = (unsigned char)i;
	}
	string  val(tmp, 256);
	sPubFields.push_back(val); totalMsgs++;



	// Start the subscriber.
	Connector subConn;
	ConT5_SubListener subListener;
	ConT5_SubRequestStatusHandler subReqSH;
	ITransport::Parameters subITParams;

    // This is only necessary to get subConnSH_OnConnStatus
	ConT5_SubConnStatusHandler subConnSH;
	subConn.AddConnectionStatusHandler(&subConnSH);

	subITParams.m_ServerUrl = serverUrl;
   	if (!subConn.Open(subITParams)){
		CPPUNIT_FAIL("Open(subITParams) failed.");
		return;
	}
	
	wstring rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);
	// ConT5_DumpStr(L"<sub>rid = " + rid);
	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
		return;
	}
	Sleep(100);	// Avoid race conditions.

///*

	// Start the publisher.
	Connector pubConn;
	ConT5_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT5_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}
	
	
	// Publish strings.
	for( i = 0; i < sPubFields.size(); i++)
	{
		Message pubMsg;

		pubMsg.Set("do_method", "notify");
		pubMsg.Set(L"kn_to", topic);
		pubMsg.Set("kn_response_format", "simple");
		pubMsg.Set("kn_expires", "+300");//5 minutes
		pubMsg.Set(sPubFields[i],VALUE);
		pubConn.Publish(pubMsg, &pubReqSH);
	}


///*
	// Avoid race conditions.
	Sleep(1000);//<<<<<<<<Subscriber still needs time to get pubs (especially if dumping output).




	// Done, disconnect.
	// ConT5_DumpStr("<sub>Unsubscribing.");
	if (!subConn.Unsubscribe(rid, &subReqSH))
	{
		CPPUNIT_FAIL("failed to unsubscribe.");
	}

	pubConn.Close();
	subConn.Close();

	subConn.RemoveConnectionStatusHandler(&subConnSH);

///*

	// Check that pub == sub - Hardcode to make sure.
	char pubBuf[12];
	itoa(totalMsgs,pubBuf,10);
	char gotBuf[12];
	itoa(wsSubFields.size(),gotBuf,10);
	string msg = (string)"If this happens you could have a race condition or other problem.\n" 
			   + (string)"The results will be unpredictable.\n"
			   + (string)"Extending the Sleep() period may be all that is needed.\n"
			   + (string)"Attempted to publish " + pubBuf + " messages.\n"
			   + (string)"Sub got " + gotBuf + " messages.";
	CPPUNIT_ASSERT_MESSAGE(msg, wsSubFields.size() == totalMsgs);

	// Check that all pubs and subs occurred.
	// ConT5_DumpCounters();
	CPPUNIT_ASSERT(conT5_count_pubConnSH_OnConnStatus==1);// 1st Publish()
	CPPUNIT_ASSERT(conT5_count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT5_count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT5_count_pubReqSH_OnSuccess    ==totalMsgs);// Publish() updates
	CPPUNIT_ASSERT(conT5_count_subListener_OnUpdate  ==totalMsgs);// updates
	CPPUNIT_ASSERT(conT5_count_subConnSH_OnConnStatus==1);// 1st Subscribe()
	CPPUNIT_ASSERT(conT5_count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT5_count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT5_count_subReqSH_OnSuccess    ==2);// Subscribe(),Unsubscribe()

	// Check that all pubs == subs
///*
	// Convert the string payloads and push them onto the wstring to compare.
	// ConT5_DumpFields();
	for( i = 0; i < sPubFields.size(); i++)
	{
		wstring tmp = ConvertToWide(sPubFields[i]);
		wsPubFields.push_back(tmp);
	}
///*
	// ConT5_DumpFields();
	for( i = 0; i < sPubFields.size(); i++)
	{	
		// Construct msg in case of error.
		int sent = sPubFields[i].length();
		int got  = wsSubFields[i].length();
		char iBuf[12];
		itoa(i,iBuf,10);
		char sentBuf[12];
		itoa(sent,sentBuf,10);
		char gotBuf[12];
		itoa(got,gotBuf,10);
		string msg = (string)"Field[" + iBuf
				   + (string)"] Sent string length=0x" + sentBuf
				   + (string)" ,got length=0x" + gotBuf;

		//ConT5_DumpStr(">>>Pub Field size =",wsPubFields[i].length());
		//ConT5_DumpStr("   Sub Field size =",wsSubFields[i].length());
		CPPUNIT_ASSERT_MESSAGE(msg,wsPubFields[i] == wsSubFields[i]);
	}
	
}//*/

/**
 * Test all 16 bit char values for property names except Unicode surrogate values U+D800 - U+DFFF
 * which can only be used with the appropriate syntax sequences.
 * All other characters should be valid in any order.
 * 
 * NOTE: Publishing messages with large property names causes severe performance problems.
 * Therefore we send 4k of wstring in 16 mssages. Performance is much better than doing it all
 * in 2 messages. This is noted as a bug for documentation: bug969.
 */
void ConnectorTest5::testPubSub_Field16bit()
{
	TU_INIT_TESTCASE("testPubSub_Field16bit");
	ConT5_ResetCounters();
	ConT5_ResetFields();
	int i = 0;
	int sleep = 0;

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/Tests/functional/ConnectorT5/testPubSub_Value16bit";


	// Determine the number of messages here to make VERY sure of results.
	int totalMsgs = 0;
	
	int start00 = 0x0000;
	const int size00 = 0x1000;		
	wchar_t tmpw00[size00+2];			
	ZeroMemory(tmpw00, size00+2);
	for ( i = 0; i < size00; i++){
		tmpw00[i] = (unsigned) (i+start00);
	}
	wstring valw00(tmpw00, size00);	
	wsPubFields.push_back(valw00); totalMsgs++;	

	int start01 = 0x1000;
	const int size01 = 0x1000;		
	wchar_t tmpw01[size01+2];			
	ZeroMemory(tmpw01, size01+2);
	for ( i = 0; i < size01; i++){
		tmpw01[i] = (unsigned) (i+start01);
	}
	wstring valw01(tmpw01, size01);	
	wsPubFields.push_back(valw01); totalMsgs++;	

	int start02 = 0x2000;
	const int size02 = 0x1000;		
	wchar_t tmpw02[size02+2];			
	ZeroMemory(tmpw02, size02+2);
	for ( i = 0; i < size02; i++){
		tmpw02[i] = (unsigned) (i+start02);
	}
	wstring valw02(tmpw02, size02);	
	wsPubFields.push_back(valw02); totalMsgs++;	

	int start03 = 0x3000;
	const int size03 = 0x1000;		
	wchar_t tmpw03[size03+2];			
	ZeroMemory(tmpw03, size03+2);
	for ( i = 0; i < size03; i++){
		tmpw03[i] = (unsigned) (i+start03);
	}
	wstring valw03(tmpw03, size03);	
	wsPubFields.push_back(valw03); totalMsgs++;	

	int start04 = 0x4000;
	const int size04 = 0x1000;		
	wchar_t tmpw04[size04+2];			
	ZeroMemory(tmpw04, size04+2);
	for ( i = 0; i < size04; i++){
		tmpw04[i] = (unsigned) (i+start04);
	}
	wstring valw04(tmpw04, size04);	
	wsPubFields.push_back(valw04); totalMsgs++;	

	int start05 = 0x5000;
	const int size05 = 0x1000;		
	wchar_t tmpw05[size05+2];			
	ZeroMemory(tmpw05, size05+2);
	for ( i = 0; i < size05; i++){
		tmpw05[i] = (unsigned) (i+start05);
	}
	wstring valw05(tmpw05, size05);	
	wsPubFields.push_back(valw05); totalMsgs++;	

	int start06 = 0x6000;
	const int size06 = 0x1000;		
	wchar_t tmpw06[size06+2];			
	ZeroMemory(tmpw06, size06+2);
	for ( i = 0; i < size06; i++){
		tmpw06[i] = (unsigned) (i+start06);
	}
	wstring valw06(tmpw06, size06);	
	wsPubFields.push_back(valw06); totalMsgs++;	

	int start07 = 0x7000;
	const int size07 = 0x1000;		
	wchar_t tmpw07[size07+2];			
	ZeroMemory(tmpw07, size07+2);
	for ( i = 0; i < size07; i++){
		tmpw07[i] = (unsigned) (i+start07);
	}
	wstring valw07(tmpw07, size07);	
	wsPubFields.push_back(valw07); totalMsgs++;	

	int start08 = 0x8000;
	const int size08 = 0x1000;		
	wchar_t tmpw08[size08+2];			
	ZeroMemory(tmpw08, size08+2);
	for ( i = 0; i < size08; i++){
		tmpw08[i] = (unsigned) (i+start08);
	}
	wstring valw08(tmpw08, size08);	
	wsPubFields.push_back(valw08); totalMsgs++;	

	int start09 = 0x9000;
	const int size09 = 0x1000;		
	wchar_t tmpw09[size09+2];			
	ZeroMemory(tmpw09, size09+2);
	for ( i = 0; i < size09; i++){
		tmpw09[i] = (unsigned) (i+start09);
	}
	wstring valw09(tmpw09, size09);	
	wsPubFields.push_back(valw09); totalMsgs++;	

	int start0A = 0xA000;
	const int size0A = 0x1000;		
	wchar_t tmpw0A[size0A+2];			
	ZeroMemory(tmpw0A, size0A+2);
	for ( i = 0; i < size0A; i++){
		tmpw0A[i] = (unsigned) (i+start0A);
	}
	wstring valw0A(tmpw0A, size0A);	
	wsPubFields.push_back(valw0A); totalMsgs++;	

	int start0B = 0xB000;
	const int size0B = 0x1000;		
	wchar_t tmpw0B[size0B+2];			
	ZeroMemory(tmpw0B, size0B+2);
	for ( i = 0; i < size0B; i++){
		tmpw0B[i] = (unsigned) (i+start0B);
	}
	wstring valw0B(tmpw0B, size0B);	
	wsPubFields.push_back(valw0B); totalMsgs++;	

	int start0C = 0xC000;
	const int size0C = 0x1000;		
	wchar_t tmpw0C[size0C+2];			
	ZeroMemory(tmpw0C, size0C+2);
	for ( i = 0; i < size0C; i++){
		tmpw0C[i] = (unsigned) (i+start0C);
	}
	wstring valw0C(tmpw0C, size0C);	
	wsPubFields.push_back(valw0C); totalMsgs++;	

	int start0D = 0xD000;
	const int size0D = 0x0800;	// <<<<<<<< Stop before D800
	wchar_t tmpw0D[size0D+2];			
	ZeroMemory(tmpw0D, size0D+2);
	for ( i = 0; i < size0D; i++){
		tmpw0D[i] = (unsigned) (i+start0D);
	}
	wstring valw0D(tmpw0D, size0D);	
	wsPubFields.push_back(valw0D); totalMsgs++;	

	// 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
//										// skip D800 to DFFF

	int start0E = 0xE000;
	const int size0E = 0x1000;		
	wchar_t tmpw0E[size0E+2];			
	ZeroMemory(tmpw0E, size0E+2);
	for ( i = 0; i < size0E; i++){
		tmpw0E[i] = (unsigned) (i+start0E);
	}
	wstring valw0E(tmpw0E, size0E);	
	wsPubFields.push_back(valw0E); totalMsgs++;	

	int start0F = 0xF000;
	const int size0F = 0x1000;		
	wchar_t tmpw0F[size0F+2];			
	ZeroMemory(tmpw0F, size0F+2);
	for ( i = 0; i < size0F; i++){
		tmpw0F[i] = (unsigned) (i+start0F);
	}
	wstring valw0F(tmpw0F, size0F);	
	wsPubFields.push_back(valw0F); totalMsgs++;	

	// 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F


//  This works but causes performance problems. See bug969.
//
//	const int size01 = 0xd800;			// 0 to D800
//	wchar_t tmpw01[size01+2];			// 0 to 1101 0111 1111 1111
//	ZeroMemory(tmpw01, size01+2);
//	for ( i = 0; i < size01; i++){
//		tmpw01[i] = (unsigned) i;
//	}
//	wstring valw01(tmpw01, size01);	
//	wsPubFields.push_back(valw01); totalMsgs++;	
//
//										// skip D800 to DFFF
//
//	const int size02 = 0xffff-0xe000+1;	// E000 to FFFF
//	wchar_t tmpw02[size02+2];			// 1110 0000 0000 0000 to 1111 1111 1111 1111
//	ZeroMemory(tmpw02, size02+2);
//	for ( i = 0; i < size02; i++){
//		tmpw02[i] = (unsigned) i + 0xe000;
//	}
//	wstring valw02(tmpw02, size02);	
//	wsPubFields.push_back(valw02); totalMsgs++;

	// 01 02 03 04 05 06 07 08 09 10


	// Start the subscriber.
	Connector subConn;
	ConT5_SubListener subListener;
	ConT5_SubRequestStatusHandler subReqSH;
	ITransport::Parameters subITParams;

    // This is only necessary to get subConnSH_OnConnStatus
	ConT5_SubConnStatusHandler subConnSH;
	subConn.AddConnectionStatusHandler(&subConnSH);

	subITParams.m_ServerUrl = serverUrl;
   	if (!subConn.Open(subITParams)){
		CPPUNIT_FAIL("Open(subITParams) failed.");
		return;
	}
	
	wstring rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);
	// ConT5_DumpStr(L"<sub>rid = " + rid);
	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
		return;
	}
	Sleep(100);	// Avoid race conditions.



	// Start the publisher.
	Connector pubConn;
	ConT5_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT5_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}
	
	// Publish wstrings.
	for( i = 0; i < wsPubFields.size(); i++)
	{
		//ConT5_DumpStr("Publish:",i);
		Message pubMsg;

		pubMsg.Set("do_method", "notify");
		pubMsg.Set(L"kn_to", topic);
		pubMsg.Set("kn_response_format", "simple");
		pubMsg.Set("kn_expires", "+300");//5 minutes
		pubMsg.Set(wsPubFields[i],VALUEw);
		pubConn.Publish(pubMsg, &pubReqSH);
	}
	


	// Avoid race conditions.
	Sleep(1000);//<<<<<<<<Subscriber still needs time to get pubs (especially if dumping output).
	Sleep(2000);//<<<<<<<<Subscriber still needs time to get pubs (especially if dumping output).
	Sleep(2000);//<<<<<<<<Subscriber still needs time to get pubs (especially if dumping output).




	// Done, disconnect.
	// ConT5_DumpStr("<sub>Unsubscribing.");
	if (!subConn.Unsubscribe(rid, &subReqSH))
	{
		CPPUNIT_FAIL("failed to unsubscribe.");
	}

	pubConn.Close();
	subConn.Close();

	subConn.RemoveConnectionStatusHandler(&subConnSH);



	// Check that pub == sub - Hardcode to make sure.
	char pubBuf[12];
	itoa(totalMsgs,pubBuf,10);
	char gotBuf[12];
	itoa(wsSubFields.size(),gotBuf,10);
	string msg = (string)"If this happens you could have a race condition or other problem.\n" 
			   + (string)"The results will be unpredictable.\n"
			   + (string)"Extending the Sleep() period may be all that is needed.\n"
			   + (string)"Attempted to publish " + pubBuf + " messages.\n"
			   + (string)"Sub got " + gotBuf + " messages.";
	CPPUNIT_ASSERT_MESSAGE(msg, wsSubFields.size() == totalMsgs);

	// Check that all pubs and subs occurred.
	// ConT5_DumpCounters();
	CPPUNIT_ASSERT(conT5_count_pubConnSH_OnConnStatus==1);// 1st Publish()
	CPPUNIT_ASSERT(conT5_count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT5_count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT5_count_pubReqSH_OnSuccess    ==totalMsgs);// Publish() updates
	CPPUNIT_ASSERT(conT5_count_subListener_OnUpdate  ==totalMsgs);// updates
	CPPUNIT_ASSERT(conT5_count_subConnSH_OnConnStatus==1);// 1st Subscribe()
	CPPUNIT_ASSERT(conT5_count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT5_count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT5_count_subReqSH_OnSuccess    ==2);// Subscribe(),Unsubscribe()

	// Check that all pubs == subs
	// ConT5_wDumpFields();
	for( i = 0; i < wsPubFields.size(); i++)
	{	
		// Construct msg in case of error.
		int sent = wsPubFields[i].length();
		int got  = wsSubFields[i].length();
		char iBuf[12];
		itoa(i,iBuf,10);
		char sentBuf[12];
		itoa(sent,sentBuf,16);
		char gotBuf[12];
		itoa(got,gotBuf,16);
		string msg = (string)"Payload[" + iBuf
				   + (string)"] Sent wstring length=0x" + sentBuf
				   + (string)" ,got length=0x" + gotBuf;

		//ConT5_DumpStr(">>>Pub Payload size =",wsPubFields[i].length());
		//ConT5_DumpStr("   Sub Payload size =",wsSubFields[i].length());
		CPPUNIT_ASSERT_MESSAGE(msg,wsPubFields[i] == wsSubFields[i]);
	}
}//*/





