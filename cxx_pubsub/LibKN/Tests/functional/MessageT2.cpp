#include "stdafx.h"
#include "globals.h"
#include "testutil.h"
#include <LibKN\Message.h>
#include <LibKN\StrUtil.h>

// If you want to leave in calls to Dump_, this must be defined to have output.
// At this time we are commenting out all calls to Dump_.
#define VERBOSE

// Define this if you want to abridge large hex dumps. It restricts to first and 
// last 4 lines a large hex dump.
#define ABRIDGE_LARGE_DUMP

// Define this if you want hex dumped as well as strings. 
// Good for non-printing characters.
#define DUMP_HEX


class MessageTest2 : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(MessageTest2);
	CPPUNIT_TEST(testTypes);
	CPPUNIT_TEST(testSetGet);
	CPPUNIT_TEST(testSetGetw);
	CPPUNIT_TEST(testSetGet_Overwrite);
	CPPUNIT_TEST(testSetGet_ValueKeyboard);
	CPPUNIT_TEST(testSetGet_FieldKeyboard);
	CPPUNIT_TEST(testSetGet_Value8bit);
	CPPUNIT_TEST(testSetGet_Field8bit);
	CPPUNIT_TEST(testSetGet_Value16bit);
	CPPUNIT_TEST(testSetGet_Field16bit);
	CPPUNIT_TEST(testBug942);
	CPPUNIT_TEST_SUITE_END();

public:
	// CPPUNIT_NS::TestFixture
	//
	void setUp() {}
	void tearDown() {}

	void testTypes();
	void testSetGet();
	void testSetGetw();
	void testSetGet_Overwrite();
	void testSetGet_ValueKeyboard();
	void testSetGet_FieldKeyboard();
	void testSetGet_Value8bit();
	void testSetGet_Field8bit();
	void testSetGet_Value16bit();
	void testSetGet_Field16bit();
	void testBug942();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MessageTest2);

// Tools //////////////////////////////////////////////////////////////////////

void InitMessage2(Message& m)
{
	m.Set("a", "a1");
	m.Set("b", "b1");
	m.Set("c", "C1");
	m.Set("d", "D1");
}

static void Dump_string(const string& msg, const string& s)
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

static void Dump_wstring(const string& msg, const wstring& s)
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

void Dump_Message(const Message& m)
{
#if defined(VERBOSE)
	printf("\nMessage:\n");
	for (Message::Container::const_iterator it = m.GetContainer().begin();
		it != m.GetContainer().end(); ++it)
	{
		wstring k = (*it).first;
		wstring v = (*it).second;

		printf("\t%S=%S\n", k.c_str(), v.c_str());
#if defined(DUMP_HEX)
		Dump_wstring("\tField hex:", k);
		Dump_wstring("\tValue hex:", v);
#endif
	}
	printf("\n");
#endif
}



// Tests //////////////////////////////////////////////////////////////////////

/**
 * For information only.
 */
void MessageTest2::testTypes()
{
	TU_INIT_TESTCASE("testTypes");

	TU_dump("Size of char   =",sizeof(char));
	TU_dump("Size of int    =",sizeof(int));
	TU_dump("Size of wchar_t=",sizeof(wchar_t));
}

/*
Make sure that when Set field is string, Get field works with either string or wstring.
*/
void MessageTest2::testSetGet()
{
	TU_INIT_TESTCASE("testSetGet");
	string  f1 =  "Field1";
	wstring F1 = L"Field1";
	string  v1 =  "Value1";
	wstring V1 = L"Value1";
	wstring V1_;
	bool found;

	Message m1;
	m1.Set(f1,v1);

	found = m1.Get(f1,V1_);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(V1 == V1_);

	found = m1.Get(F1,V1_);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(V1_ == V1);

	TU_dumpMsg("Msg 1:",m1);
}

/*
Make sure that when Set field is wstring, Get field works with either string or wstring.
*/
void MessageTest2::testSetGetw()
{
	TU_INIT_TESTCASE("testSetGetw");
	string  f1 =  "Field1";
	wstring F1 = L"Field1";
	string  v1 =  "Value1";
	wstring V1 = L"Value1";
	wstring V1_;
	bool found;

	Message m1;
	m1.Set(F1,V1);

	found = m1.Get(f1,V1_);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(V1 == V1_);

	found = m1.Get(F1,V1_);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(V1_ == V1);

	TU_dumpMsg("Msg 1:",m1);
}

void MessageTest2::testSetGet_Overwrite()
{
	TU_INIT_TESTCASE("testSetGet_Overwrite");
	string  f1 =  "Field1";
	wstring V1_;
	bool found;

	Message m1;
	m1.Set(f1,"Original value");
	found = m1.Get(f1,V1_);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(V1_ == L"Original value");

	m1.Set(f1,"New value");
	found = m1.Get(f1,V1_);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(V1_ == L"New value");

	m1.Set(f1,"\n");
	found = m1.Get(f1,V1_);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(V1_ == L"\n");

	m1.Set(f1,"");
	found = m1.Get(f1,V1_);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(V1_ == L"");

	TU_dumpMsg("Msg 1:",m1);
}

/**
 * Any keyboard character should be valid for values.
 * This test is redundant (testSetGet_Value8bit) but it can be useful for demonstration.
 * You must escape '\' and '"' in C++ code.
 */
void MessageTest2::testSetGet_ValueKeyboard()
{
	TU_INIT_TESTCASE("testSetGet_ValueKeyboard");
	wstring wstr;
	bool found;

	Message m1;

	m1.Set("Field01","`1234567890-=");	
	m1.Set("Field02","~!@#$%^&*()_+");	
	m1.Set("Field03","[]\\");	
	m1.Set("Field04","{}|");	
	m1.Set("Field05",";'");	
	m1.Set("Field06",":\"");	
	m1.Set("Field07",",./");	
	m1.Set("Field08","<>?");	
	
	found = m1.Get("Field01",wstr);	CPPUNIT_ASSERT(found);	CPPUNIT_ASSERT(wstr == L"`1234567890-=");
	found = m1.Get("Field02",wstr);	CPPUNIT_ASSERT(found);	CPPUNIT_ASSERT(wstr == L"~!@#$%^&*()_+");
	found = m1.Get("Field03",wstr);	CPPUNIT_ASSERT(found);	CPPUNIT_ASSERT(wstr == L"[]\\");
	found = m1.Get("Field04",wstr);	CPPUNIT_ASSERT(found);	CPPUNIT_ASSERT(wstr == L"{}|");
	found = m1.Get("Field05",wstr);	CPPUNIT_ASSERT(found);	CPPUNIT_ASSERT(wstr == L";'");
	found = m1.Get("Field06",wstr);	CPPUNIT_ASSERT(found);	CPPUNIT_ASSERT(wstr == L":\"");
	found = m1.Get("Field07",wstr);	CPPUNIT_ASSERT(found);	CPPUNIT_ASSERT(wstr == L",./");
	found = m1.Get("Field08",wstr);	CPPUNIT_ASSERT(found);	CPPUNIT_ASSERT(wstr == L"<>?");

	TU_dumpMsg("Msg 1:",m1);
}

/**
 * Any keyboard character should be valid for fields.
 * This test is redundant (testSetGet_Field8bit) but it can be useful for demonstration.
 * You must escape '\' and '"' in C++ code.
 */
void MessageTest2::testSetGet_FieldKeyboard()
{
	TU_INIT_TESTCASE("testSetGet_FieldKeyboard");
	wstring wstr;
	bool found;

	Message m1;


	string field01 = "`1234567890-=";	
	string field02 = "~!@#$%^&*()_+";	
	string field03 = "[]\\";	
	string field04 = "{}|";	
	string field05 = ";'";	
	string field06 = ":\"";	
	string field07 = ",./";	
	string field08 = "<>?";	
	string field09 = "";	
	
	wstring field01w = L"`1234567890-=-L";	
	wstring field02w = L"~!@#$%^&*()_+-L";	
	wstring field03w = L"[]\\-L";	
	wstring field04w = L"{}|-L";	
	wstring field05w = L";'-L";	
	wstring field06w = L":\"-L";	
	wstring field07w = L",./-L";	
	wstring field08w = L"<>?-L";	
	wstring field09w = L"";	
	
	string value01 = "value-str-01";	
	string value02 = "value-str-02";	
	string value03 = "value-str-03";	
	string value04 = "value-str-04";	
	string value05 = "value-str-05";	
	string value06 = "value-str-06";	
	string value07 = "value-str-07";	
	string value08 = "value-str-08";	
	string value09 = "value-str-09";	
	
	wstring value01w = L"value-str-01";	
	wstring value02w = L"value-str-02";	
	wstring value03w = L"value-str-03";	
	wstring value04w = L"value-str-04";	
	wstring value05w = L"value-str-05";	
	wstring value06w = L"value-str-06";	
	wstring value07w = L"value-str-07";	
	wstring value08w = L"value-str-08";	
	wstring value09w = L"value-str-09";	
	
	
	m1.Set(field01,value01);	
	m1.Set(field02,value02);	
	m1.Set(field03,value03);	
	m1.Set(field04,value04);	
	m1.Set(field05,value05);	
	m1.Set(field06,value06);	
	m1.Set(field07,value07);	
	m1.Set(field08,value08);	
	m1.Set(field09,value09);	
	m1.Set(field01w,value01w);	
	m1.Set(field02w,value02w);	
	m1.Set(field03w,value03w);	
	m1.Set(field04w,value04w);	
	m1.Set(field05w,value05w);	
	m1.Set(field06w,value06w);	
	m1.Set(field07w,value07w);	
	m1.Set(field08w,value08w);	
	
	TU_dumpMsg("Msg 1:",m1);

	found = m1.Get(field01,wstr);
	CPPUNIT_ASSERT(found);	
	CPPUNIT_ASSERT(wstr == value01w);

	found = m1.Get(field02,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value02w);

	found = m1.Get(field03,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value03w);

	found = m1.Get(field04,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value04w);

	found = m1.Get(field05,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value05w);

	found = m1.Get(field06,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value06w);

	found = m1.Get(field07,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value07w);

	found = m1.Get(field08,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value08w);

	found = m1.Get(field09,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value09w);

	found = m1.Get(field01w,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value01w);

	found = m1.Get(field02w,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value02w);

	found = m1.Get(field03w,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value03w);

	found = m1.Get(field04w,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value04w);

	found = m1.Get(field05w,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value05w);

	found = m1.Get(field06w,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value06w);

	found = m1.Get(field07w,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value07w);

	found = m1.Get(field08w,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value08w);

	// DO this last because it will overwrite.
	m1.Set(field09w,value09w);
	
	TU_dumpMsg("Msg 1:",m1);

	found = m1.Get(field09w,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == value09w);

}

/**
 * Test all 8 bit char values for values. All should be valid.
 * '/' is a special character (folder delimiter for topics).
 */
void MessageTest2::testSetGet_Value8bit()
{
	TU_INIT_TESTCASE("testSetGet_Value8bit");
	char tmp[258];
	ZeroMemory(tmp, 258);
	for (int i = 0; i < 256; i++){
		tmp[i] = (unsigned char)i;
	}
	string  val(tmp, 256);

	wchar_t tmpw[258];
	ZeroMemory(tmpw, 258);
	for (i = 0; i < 256; i++){
		tmpw[i] = (unsigned) i;
	}
	wstring valw(tmpw, 256);

	wstring wstr;
	bool found;
	Message m1;

	m1.Set( "Field01",val);	
	m1.Set(L"Field02",valw);	

	TU_dumpMsg("Msg 1:",m1);
	
	found = m1.Get("Field01" ,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == valw);

	found = m1.Get("Field02" ,wstr);
	CPPUNIT_ASSERT(found);
	CPPUNIT_ASSERT(wstr == valw);
}

/**
 * Test all 8 bit char values for fields. All should be valid.
 * '/' is a special character (folder delimiter for topics).
 */
void MessageTest2::testSetGet_Field8bit()
{
	TU_INIT_TESTCASE("testSetGet_Field8bit");
	char tmp[259];
	ZeroMemory(tmp, 258);
	for (int i = 0; i < 256; i++){
		tmp[i] = (unsigned char)i;
	}
	tmp[256] = '1';        // Field 1.
	string  field01(tmp, 257);

	wchar_t tmpw[259];
	ZeroMemory(tmpw, 258);
	for (i = 0; i < 256; i++){
		tmpw[i] = (unsigned) i;
	}
	char* tmpx = (char*)tmpw;
	tmpx[256*2]     = (unsigned char)0x32;// Field 2.
	tmpx[(256*2)+1] = (unsigned char)0x00;       
	wstring field02w(tmpw, 257);

	wstring wstr;
	bool found;
	Message m1;

	m1.Set(field01,  "value-str-01");	
	m1.Set(field02w,L"value-str-02");
	
	TU_dumpMsg("Msg 1:",m1);
	
	found = m1.Get(field01 ,wstr);	CPPUNIT_ASSERT(found);	CPPUNIT_ASSERT(wstr == L"value-str-01");
	found = m1.Get(field02w,wstr);	CPPUNIT_ASSERT(found);	CPPUNIT_ASSERT(wstr == L"value-str-02");
}

/**
 * Test all 8 bit char values for values. All should be valid.
 */
void MessageTest2::testSetGet_Value16bit()
{
	TU_INIT_TESTCASE("testSetGet_Value16bit");
	wchar_t tmpw[65536+2];
	ZeroMemory(tmpw, 65536+2);
	for (int i = 0; i < 65536; i++){
		tmpw[i] = (unsigned) i;
	}
	wstring valw(tmpw, 65536);

	wstring wstr;
	bool found;
	Message m1;

	m1.Set(L"Field-str-01",valw);	

	TU_dumpMsg("Msg 1:",m1);
	
	found = m1.Get("Field-str-01" ,wstr);	
	CPPUNIT_ASSERT(found);	
	CPPUNIT_ASSERT(wstr == valw);
}

/**
 * Test all 16 bit char values for fields. All should be valid.
 */
void MessageTest2::testSetGet_Field16bit()
{
	TU_INIT_TESTCASE("testSetGet_Field16bit");
	wchar_t tmpw[65536+2];
	ZeroMemory(tmpw, 65536+2);
	for (int i = 0; i < 65536; i++){
		tmpw[i] = (unsigned) i;
	}
	wstring field01w(tmpw, 65536);

	wstring wstr;
	bool found;
	Message m1;

	m1.Set(field01w,L"value-str-01");	
	
	TU_dumpMsg("Msg 1:",m1);
	
	found = m1.Get(field01w,wstr);	
	CPPUNIT_ASSERT(found);	
	CPPUNIT_ASSERT(wstr == L"value-str-01");
}


/**
 * This turned out to be a problem in the compiler.
 * L'\x80 is interpreted as a multibyte character.
 */
void MessageTest2::testBug942()
{
	TU_INIT_TESTCASE("testBug942");
	wstring wstr;
	bool found;

	Message m1;

	const char     chars1[] = { '\x3e', '\x3e', '\x80', '\x3c', '\x3c', '\x00'};
	const wchar_t wchars2[] = {L'\x3e',L'\x3e',L'\x80',L'\x3c',L'\x3c',L'\x00'};

	// Correct the compiler problem.
	char* tmp = (char*)wchars2;
	tmp[4] = (unsigned char)0x80;
	tmp[5] = (unsigned char)0x00;

	const char     chars3[] = { '\x3e', '\x3e', '\xff', '\x3c', '\x3c', '\x00'};
	const wchar_t wchars4[] = {L'\x3e',L'\x3e',L'\xff',L'\x3c',L'\x3c',L'\x00'};

	string  v1;	v1.assign( chars1,strlen( chars1));
	wstring V2;	V2.assign(wchars2,wcslen(wchars2));
	string  v3;	v3.assign( chars3,strlen( chars3));
	wstring V4;	V4.assign(wchars4,wcslen(wchars4));

	m1.Set( "Field01",v1);	
	m1.Set(L"Field02",V2);	
	m1.Set( "Field03",v3);	
	m1.Set(L"Field04",V4);	
	TU_dumpMsg("Msg 1:",m1);

	TU_dumpHex("Field01=",v1);
	TU_dumpHex("Field02=",V2);
	TU_dumpHex("Field03=",v3);
	TU_dumpHex("Field04=",V4);
	
	found = m1.Get("Field01",wstr);	
	CPPUNIT_ASSERT(found);	
	CPPUNIT_ASSERT(wstr == V2);

	found = m1.Get("Field02",wstr);	
	CPPUNIT_ASSERT(found);	
	CPPUNIT_ASSERT(wstr == V2);
	found = m1.Get("Field03",wstr);	CPPUNIT_ASSERT(found);	CPPUNIT_ASSERT(wstr == V4);
	found = m1.Get("Field04",wstr);	CPPUNIT_ASSERT(found);	CPPUNIT_ASSERT(wstr == V4);

}




