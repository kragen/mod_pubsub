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

// If you want to leave in calls to ConT4_Dump_, this must be defined to have output.
// At this time we are commenting out all calls to ConT4_Dump_.
#define VERBOSE

// Define this if you want to abridge large hex dumps. It restricts to first and 
// last 4 lines a large hex dump.
#define ABRIDGE_LARGE_DUMP

// Define this if you want hex dumped as well as strings. 
// Good for non-printing characters.
#define DUMP_HEX

/**
 * These test Publish() and Subscribe() with all possible character data 
 * for kn_payload values:
 * 1. All 8 bit characters: 00 to FF
 * 2. All 16 bit unicode non-surrogate characters: 0000 to D7FF and E000 to FFFF.
 * 3. Unicode surrogate characters are not test at this time.
 *
 * The test LibKN/Tests/functional/ConnectorT5.cpp has parallel tests for 
 * Publish() and Subscribe() user message fields.
 * The test LibKN/Tests/functional/MessageT2.cpp has parallel tests for Message().
 */
class ConnectorTest4 : public CPPUNIT_NS::TestFixture
{

	CPPUNIT_TEST_SUITE(ConnectorTest4);
	CPPUNIT_TEST(testPubSub_ValueKeyboard);
	CPPUNIT_TEST(testPubSub_Value8bit);
	CPPUNIT_TEST(testPubSub_Value16bit);
	//CPPUNIT_TEST(testPubSub_FieldKeyboard);
	//CPPUNIT_TEST(testPubSub_Field8bit);
	//CPPUNIT_TEST(testPubSub_Field16bit);
	//CPPUNIT_TEST(testBug942);
	CPPUNIT_TEST_SUITE_END();

public:

	// CPPUNIT_NS::TestFixture
	//
	void setUp() {}
	void tearDown() {}

	void testPubSub_ValueKeyboard();
	void testPubSub_Value8bit();
	void testPubSub_Value16bit();
	void testPubSub_FieldKeyboard();
	void testPubSub_Field8bit();
	void testPubSub_Field16bit();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConnectorTest4);

// Variables //////////////////////////////////////////////////////////////////


int conT4_count_subListener_OnUpdate = 0;
int conT4_count_subConnSH_OnConnStatus = 0;
int conT4_count_subReqSH_OnStatus = 0;
int conT4_count_subReqSH_OnError = 0;
int conT4_count_subReqSH_OnSuccess = 0;
int conT4_count_pubConnSH_OnConnStatus = 0;
int conT4_count_pubReqSH_OnStatus = 0;
int conT4_count_pubReqSH_OnError = 0;
int conT4_count_pubReqSH_OnSuccess = 0;

vector<string>   sPubPayloads;
vector<wstring> wsPubPayloads;
vector<wstring> wsSubPayloads;


// Tools //////////////////////////////////////////////////////////////////////

void ConT4_DumpInit()
{
#if defined(VERBOSE)
	printf("\n");
#endif
}

void ConT4_DumpStr(const char* text)
{
#if defined(VERBOSE)
	printf("%s\n", text);
#endif
}

void ConT4_DumpStr(const char* text, int i)
{
#if defined(VERBOSE)
	printf("%s%d\n", text, i);
#endif
}

void ConT4_DumpStr(string text)
{
#if defined(VERBOSE)
	printf("%s\n", text.c_str());
#endif
}

void ConT4_DumpStr(wstring text)
{
#if defined(VERBOSE)
	printf("%S\n", text.c_str());
#endif
}

static void ConT4_Dump_string(const string& msg, const string& s)
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

static void ConT4_Dump_wstring(const string& msg, const wstring& s)
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

void ConT4_DumpMsg(char * text, const Message& m)
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
		ConT4_Dump_wstring("\tField hex:", k);
		ConT4_Dump_wstring("\tValue hex:", v);
#endif
	}
	printf("\n");
#endif
}

void ConT4_ResetCounters()
{
	conT4_count_pubConnSH_OnConnStatus = 0;
	conT4_count_pubReqSH_OnStatus = 0;
	conT4_count_pubReqSH_OnError = 0;
	conT4_count_pubReqSH_OnSuccess = 0;
	conT4_count_subListener_OnUpdate = 0;
	conT4_count_subConnSH_OnConnStatus = 0;
	conT4_count_subReqSH_OnStatus = 0;
	conT4_count_subReqSH_OnError = 0;
	conT4_count_subReqSH_OnSuccess = 0;
}

void ConT4_DumpCounters()
{
#if defined(VERBOSE)
	printf("conT4_count_pubConnSH_OnConnStatus=%d\n",conT4_count_pubConnSH_OnConnStatus);
	printf("conT4_count_pubReqSH_OnStatus     =%d\n",conT4_count_pubReqSH_OnStatus);
	printf("conT4_count_pubReqSH_OnError      =%d\n",conT4_count_pubReqSH_OnError);
	printf("conT4_count_pubReqSH_OnSuccess    =%d\n",conT4_count_pubReqSH_OnSuccess);
	printf("conT4_count_subListener_OnUpdate  =%d\n",conT4_count_subListener_OnUpdate);
	printf("conT4_count_subConnSH_OnConnStatus=%d\n",conT4_count_subConnSH_OnConnStatus);
	printf("conT4_count_subReqSH_OnStatus     =%d\n",conT4_count_subReqSH_OnStatus);
	printf("conT4_count_subReqSH_OnError      =%d\n",conT4_count_subReqSH_OnError);
	printf("conT4_count_subReqSH_OnSuccess    =%d\n",conT4_count_subReqSH_OnSuccess);
#endif
}

void ConT4_DumpPayloads()
{
#if defined(VERBOSE)
	cout << "Pub Payloads<string>: " << sPubPayloads.size() << "\n";
	for(int i = 0; i < sPubPayloads.size(); i++)
	{
		cout << "\t" << sPubPayloads[i] << "\n";
	}
	cout << "Pub Payloads<wstring>: " << wsPubPayloads.size() << "\n";
	for( i = 0; i < wsPubPayloads.size(); i++)
	{
		cout << "\t";
		ConT4_DumpStr(wsPubPayloads[i]);
	}
	cout << "Sub Payloads<wstring>: " << wsSubPayloads.size() << "\n";
	for( i = 0; i < wsSubPayloads.size(); i++)
	{
		cout << "\t";
		ConT4_DumpStr(wsSubPayloads[i]);
	}
#endif
}

void ConT4_ResetPayloads()
{
	int n = sPubPayloads.size();
	for(int i = 0; i < n; i++)
	{
		sPubPayloads.pop_back();
	}
	n = wsPubPayloads.size();
	for( i = 0; i < n; i++)
	{
		wsPubPayloads.pop_back();
	}
	n = wsSubPayloads.size();
	for( i = 0; i < n; i++)
	{
		wsSubPayloads.pop_back();
	}
}


// Classes ////////////////////////////////////////////////////////////////////

class ConT4_SubListener : public IListener
{
public:
	void OnUpdate(const Message& msg)
	{
		//ConT4_DumpMsg("<Sub>Listener: OnUpdate", msg);
		//printf("L");
		conT4_count_subListener_OnUpdate++;

		for (Message::Container::const_iterator it = msg.GetContainer().begin();
			it != msg.GetContainer().end(); ++it)
		{
			wstring k = (*it).first;
			wstring v = (*it).second;
			if(k == L"kn_payload")
			{
				wsSubPayloads.push_back(v);
			}
		}
	}
};

class ConT4_SubConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg)
	{
		//ConT4_DumpMsg("<Sub>ConnectionStatusHandler: OnConnectionStatus", msg);
		conT4_count_subConnSH_OnConnStatus++;
	}
};

class ConT4_PubConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg)
	{
		//ConT4_DumpMsg("<Pub>ConnectionStatusHandler: OnConnectionStatus", msg);
		conT4_count_pubConnSH_OnConnStatus++;
	}
};

class ConT4_SubRequestStatusHandler : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//ConT4_DumpMsg("<Sub>Request Status Sucess: OnError", msg);
		conT4_count_subReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//ConT4_DumpMsg("<Sub>RequestStatusHandler: OnSuccess", msg);
		conT4_count_subReqSH_OnSuccess++;
	}
};

class ConT4_PubRequestStatusHandler : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//ConT4_DumpMsg("<Pub>RequestStatusHandler: OnError", msg);
		conT4_count_pubReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//ConT4_DumpMsg("<Pub>RequestStatusHandler: OnSuccess", msg);
		conT4_count_pubReqSH_OnSuccess++;
	}
};

// Tests /////////////////////////////////////////////////////////////////////


/**
 * Any keyboard character should be valid for values.
 * This test is redundant (testPubSub_Value8bit) but it can be useful for demonstration.
 * You must escape '\' and '"' in C++ code.
 */
void ConnectorTest4::testPubSub_ValueKeyboard()
{
	TU_INIT_TESTCASE("testPubSub_ValueKeyboard");
	ConT4_ResetCounters();
	ConT4_ResetPayloads();
	int i = 0;
	int sleep = 0;

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/Tests/functional/ConnectorT4/testPubSub_ValueKeyboard";

	wsPubPayloads.push_back(L"`1234567890-=~!@#$%^&*()_+[]\\{}|;':\",./<>?");
	wsPubPayloads.push_back(L" ");
	wsPubPayloads.push_back(L"");
	sPubPayloads.push_back("`1234567890-=~!@#$%^&*()_+[]\\{}|;':\",./<>?");
	sPubPayloads.push_back(" ");
	sPubPayloads.push_back("");



	// Start the subscriber.
	Connector subConn;
	ConT4_SubListener subListener;
	ConT4_SubRequestStatusHandler subReqSH;
	ITransport::Parameters subITParams;

    // This is only necessary to get subConnSH_OnConnStatus
	ConT4_SubConnStatusHandler subConnSH;
	subConn.AddConnectionStatusHandler(&subConnSH);

	subITParams.m_ServerUrl = serverUrl;
   	if (!subConn.Open(subITParams)){
		CPPUNIT_FAIL("Open(subITParams) failed.");
		return;
	}
	
	wstring rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);
	// ConT4_DumpStr(L"<sub>rid = " + rid);
	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
		return;
	}
	Sleep(100); // Avoid race conditions.



	// Start the publisher.
	Connector pubConn;
	ConT4_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;
	Message pubMsg;

	pubMsg.Set("do_method", "notify");
	pubMsg.Set(L"kn_to", topic);
	pubMsg.Set("kn_response_format", "simple");

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT4_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}
	
	// Publish wstrings.
	for( i = 0; i < wsPubPayloads.size(); i++)
	{
		pubMsg.Set(L"kn_payload", wsPubPayloads[i]);
		pubConn.Publish(pubMsg, &pubReqSH);
	}
	
	// Publish strings.
	for( i = 0; i < sPubPayloads.size(); i++)
	{
		pubMsg.Set("kn_payload", sPubPayloads[i]);
		pubConn.Publish(pubMsg, &pubReqSH);
	}



	// Avoid race conditions.
	/*
	sleep = 20; 
	while (pubConn.HasItems())
	{
		Sleep(100);	
		printf("p");
		if( sleep-- == 0 ) break;
	}
	if (pubConn.HasItems())
	{
		CPPUNIT_FAIL("There's something in the publish queue (it should be empty).");
		return;
	}
	sleep = 20; 
	while (subConn.HasItems())
	{
		Sleep(100);	
		printf("s");
		if( sleep-- == 0 ) break;
	}
	if (subConn.HasItems())
	{
		CPPUNIT_FAIL("There's something in the subscribe queue (it should be empty).");
		return;
	}
	//*/
	Sleep(1000);//<<<<<<<<Subscriber still needs time to get pubs (especially if dumping output).



	// Done, disconnect.
	// ConT4_DumpStr("<sub>Unsubscribing.");
	if (!subConn.Unsubscribe(rid, &subReqSH))
	{
		CPPUNIT_FAIL("failed to unsubscribe.");
	}

	pubConn.Close();
	subConn.Close();

	subConn.RemoveConnectionStatusHandler(&subConnSH);



	// Check that pub == sub - Hardcode to make sure.
	CPPUNIT_ASSERT_MESSAGE("If this happens you could have a race condition."
		,wsSubPayloads.size() == 6);

	// Check that all pubs and subs occurred.
	// ConT4_DumpCounters();
	CPPUNIT_ASSERT(conT4_count_pubConnSH_OnConnStatus==1);// 1st Publish()
	CPPUNIT_ASSERT(conT4_count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT4_count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT4_count_pubReqSH_OnSuccess    ==6);// Publish() updates
	CPPUNIT_ASSERT(conT4_count_subListener_OnUpdate  ==6);// updates
	CPPUNIT_ASSERT(conT4_count_subConnSH_OnConnStatus==1);// 1st Subscribe()
	CPPUNIT_ASSERT(conT4_count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT4_count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT4_count_subReqSH_OnSuccess    ==2);// Subscribe(),Unsubscribe()

	// Convert the string payloads and push them onto the wstring to compare.
	// ConT4_DumpPayloads();
	for( i = 0; i < sPubPayloads.size(); i++)
	{
		wstring tmp = ConvertToWide(sPubPayloads[i]);
		wsPubPayloads.push_back(tmp);
	}

	// Check that all pubs == subs
	// ConT4_DumpPayloads();
	for( i = 0; i < wsPubPayloads.size(); i++)
	{
		CPPUNIT_ASSERT(wsPubPayloads[i] == wsSubPayloads[i]);
	}
}

/**
 * Test all 8 bit char values for values. All should be valid.
 */
void ConnectorTest4::testPubSub_Value8bit()
{
	TU_INIT_TESTCASE("testPubSub_Value8bit");
	ConT4_ResetCounters();
	ConT4_ResetPayloads();
	int i = 0;
	int sleep = 0;

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/Tests/functional/ConnectorT4/testPubSub_Value8bit";

	wchar_t tmpw[258];
	ZeroMemory(tmpw, 258);
	for (i = 0; i < 256; i++){
		tmpw[i] = (unsigned) i;
	}
	wstring valw(tmpw, 256);

	char tmp[258];
	ZeroMemory(tmp, 258);
	for ( i = 0; i < 256; i++){
		tmp[i] = (unsigned char)i;
	}
	string  val(tmp, 256);

	wsPubPayloads.push_back(valw);
	sPubPayloads.push_back(val);



	// Start the subscriber.
	Connector subConn;
	ConT4_SubListener subListener;
	ConT4_SubRequestStatusHandler subReqSH;
	ITransport::Parameters subITParams;

    // This is only necessary to get subConnSH_OnConnStatus
	ConT4_SubConnStatusHandler subConnSH;
	subConn.AddConnectionStatusHandler(&subConnSH);

	subITParams.m_ServerUrl = serverUrl;
   	if (!subConn.Open(subITParams)){
		CPPUNIT_FAIL("Open(subITParams) failed.");
		return;
	}
	
	wstring rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);
	// ConT4_DumpStr(L"<sub>rid = " + rid);
	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
		return;
	}
	Sleep(100);	// Avoid race conditions.



	// Start the publisher.
	Connector pubConn;
	ConT4_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;
	Message pubMsg;

	pubMsg.Set("do_method", "notify");
	pubMsg.Set(L"kn_to", topic);
	pubMsg.Set("kn_response_format", "simple");

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT4_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}
	
	// Publish wstrings.
	for( i = 0; i < wsPubPayloads.size(); i++)
	{
		pubMsg.Set(L"kn_payload", wsPubPayloads[i]);
		pubConn.Publish(pubMsg, &pubReqSH);
	}
	
	// Publish strings.
	for( i = 0; i < sPubPayloads.size(); i++)
	{
		pubMsg.Set("kn_payload", sPubPayloads[i]);
		pubConn.Publish(pubMsg, &pubReqSH);
	}



	// Avoid race conditions.
	Sleep(1000);//<<<<<<<<Subscriber still needs time to get pubs (especially if dumping output).




	// Done, disconnect.
	// ConT4_DumpStr("<sub>Unsubscribing.");
	if (!subConn.Unsubscribe(rid, &subReqSH))
	{
		CPPUNIT_FAIL("failed to unsubscribe.");
	}

	pubConn.Close();
	subConn.Close();

	subConn.RemoveConnectionStatusHandler(&subConnSH);



	// Check that pub == sub - Hardcode to make sure.
	CPPUNIT_ASSERT_MESSAGE("If this happens you could have a race condition."
		,wsSubPayloads.size() == 2);

	// Check that all pubs and subs occurred.
	// ConT4_DumpCounters();
	CPPUNIT_ASSERT(conT4_count_pubConnSH_OnConnStatus==1);// 1st Publish()
	CPPUNIT_ASSERT(conT4_count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT4_count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT4_count_pubReqSH_OnSuccess    ==2);// Publish() updates
	CPPUNIT_ASSERT(conT4_count_subListener_OnUpdate  ==2);// updates
	CPPUNIT_ASSERT(conT4_count_subConnSH_OnConnStatus==1);// 1st Subscribe()
	CPPUNIT_ASSERT(conT4_count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT4_count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT4_count_subReqSH_OnSuccess    ==2);// Subscribe(),Unsubscribe()

	// Convert the string payloads and push them onto the wstring to compare.
	// ConT4_DumpPayloads();
	for( i = 0; i < sPubPayloads.size(); i++)
	{
		wstring tmp = ConvertToWide(sPubPayloads[i]);
		wsPubPayloads.push_back(tmp);
	}

	// Check that all pubs == subs
	// ConT4_DumpPayloads();
	for( i = 0; i < wsPubPayloads.size(); i++)
	{
		CPPUNIT_ASSERT(wsPubPayloads[i] == wsSubPayloads[i]);
	}
}

/**
 * Test all 16 bit char values for values except Unicode surrogate values U+D800 - U+DFFF
 * which can only be used with the appropriate syntax sequences.
 * All other characters should be valid in any order.
 */
void ConnectorTest4::testPubSub_Value16bit()
{
	TU_INIT_TESTCASE("testPubSub_Value16bit");
	ConT4_ResetCounters();
	ConT4_ResetPayloads();
	int i = 0;
	int sleep = 0;

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/Tests/functional/ConnectorT4/testPubSub_Value16bit";


	const int size01 = 0xd800;			// 0 to D800
	wchar_t tmpw01[size01+2];			// 0 to 1101 0111 1111 1111
	ZeroMemory(tmpw01, size01+2);
	for ( i = 0; i < size01; i++){
		tmpw01[i] = (unsigned) i;
	}
	wstring valw01(tmpw01, size01);	
	wsPubPayloads.push_back(valw01);	

										// skip D800 to DFFF

	const int size02 = 0xffff-0xe000+1;	// E000 to FFFF
	wchar_t tmpw02[size02+2];			// 1110 0000 0000 0000 to 1111 1111 1111 1111
	ZeroMemory(tmpw02, size02+2);
	for ( i = 0; i < size02; i++){
		tmpw02[i] = (unsigned) i + 0xe000;
	}
	wstring valw02(tmpw02, size02);	
	wsPubPayloads.push_back(valw02);	

	// 01 02 03 04 05 06 07 08 09 10
	// Hardcode the number of messages sent to make very sure of results.
	int totalMsgs = 2;

	// Start the subscriber.
	Connector subConn;
	ConT4_SubListener subListener;
	ConT4_SubRequestStatusHandler subReqSH;
	ITransport::Parameters subITParams;

    // This is only necessary to get subConnSH_OnConnStatus
	ConT4_SubConnStatusHandler subConnSH;
	subConn.AddConnectionStatusHandler(&subConnSH);

	subITParams.m_ServerUrl = serverUrl;
   	if (!subConn.Open(subITParams)){
		CPPUNIT_FAIL("Open(subITParams) failed.");
		return;
	}
	
	wstring rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);
	// ConT4_DumpStr(L"<sub>rid = " + rid);
	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
		return;
	}
	Sleep(100);	// Avoid race conditions.



	// Start the publisher.
	Connector pubConn;
	ConT4_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;
	Message pubMsg;

	pubMsg.Set("do_method", "notify");
	pubMsg.Set(L"kn_to", topic);
	pubMsg.Set("kn_response_format", "simple");
	pubMsg.Set("kn_expires", "+1200");//20 minutes
	pubMsg.Set("testname", "functional/ConnectorT4::testPubSub_Value16bit()");
	// A large value to make sure problem is payload size
	// not the total msg size. If it is the msg size this will force a
	// different msg to fail.
	//pubMsg.Set(L"testbuf", valw01); // Listener does not trigger.
	//pubMsg.Set(L"testbuf", L"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");

    // This is only necessary to get pubConnSH_OnConnStatus
	ConT4_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}
	
	// Publish wstrings.
	for( i = 0; i < wsPubPayloads.size(); i++)
	{
		pubMsg.Set(L"kn_payload", wsPubPayloads[i]);
		pubConn.Publish(pubMsg, &pubReqSH);
	}
	


	// Avoid race conditions.
	Sleep(1000);//<<<<<<<<Subscriber still needs time to get pubs (especially if dumping output).
	//Sleep(2000);//<<<<<<<<Subscriber still needs time to get pubs (especially if dumping output).




	// Done, disconnect.
	// ConT4_DumpStr("<sub>Unsubscribing.");
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
	itoa(wsSubPayloads.size(),gotBuf,10);
	string msg = (string)"If this happens you could have a race condition or other problem.\n" 
			   + (string)"The results will be unpredictable.\n"
			   + (string)"Attempted to publish " + pubBuf + " messages.\n"
			   + (string)"Sub got " + gotBuf + " messages.";
	CPPUNIT_ASSERT_MESSAGE(msg, wsSubPayloads.size() == totalMsgs);

	// Check that all pubs and subs occurred.
	// ConT4_DumpCounters();
	CPPUNIT_ASSERT(conT4_count_pubConnSH_OnConnStatus==1);// 1st Publish()
	CPPUNIT_ASSERT(conT4_count_pubReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT4_count_pubReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT4_count_pubReqSH_OnSuccess    ==totalMsgs);// Publish() updates
	CPPUNIT_ASSERT(conT4_count_subListener_OnUpdate  ==totalMsgs);// updates
	CPPUNIT_ASSERT(conT4_count_subConnSH_OnConnStatus==1);// 1st Subscribe()
	CPPUNIT_ASSERT(conT4_count_subReqSH_OnStatus     ==0);
	CPPUNIT_ASSERT(conT4_count_subReqSH_OnError      ==0);
	CPPUNIT_ASSERT(conT4_count_subReqSH_OnSuccess    ==2);// Subscribe(),Unsubscribe()

	// Check that all pubs == subs
	// ConT4_DumpPayloads();
	for( i = 0; i < wsPubPayloads.size(); i++)
	{	
		// Construct msg in case of error.
		int sent = wsPubPayloads[i].length();
		int got  = wsSubPayloads[i].length();
		char iBuf[12];
		itoa(i,iBuf,10);
		char sentBuf[12];
		itoa(sent,sentBuf,16);
		char gotBuf[12];
		itoa(got,gotBuf,16);
		string msg = (string)"Payload[" + iBuf
				   + (string)"] Sent wstring length=0x" + sentBuf
				   + (string)" ,got length=0x" + gotBuf;

		//ConT4_DumpStr(">>>Pub Payload size =",wsPubPayloads[i].length());
		//ConT4_DumpStr("   Sub Payload size =",wsSubPayloads[i].length());
		CPPUNIT_ASSERT_MESSAGE(msg,wsPubPayloads[i] == wsSubPayloads[i]);
	}
}

