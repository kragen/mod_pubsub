#include "stdafx.h"
#include "globals.h"
#include "testutil.h"
#include <LibKN\StrUtil.h>

class Utf8Test : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(Utf8Test);
	CPPUNIT_TEST(testUtf8);
	CPPUNIT_TEST(testFromUtf8);
	//CPPUNIT_TEST(testTruncateFromUtf8);
	CPPUNIT_TEST(testSerialization);
	CPPUNIT_TEST_SUITE_END();

public:
	// CPPUNIT_NS::TestFixture
	//
	void setUp() {}
	void tearDown() {}

	void testUtf8();
	void testFromUtf8();
	//void testTruncateFromUtf8();
	void testSerialization();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Utf8Test);

/**
 * Test the boundaries of the Utf8 strings.
 */
void Utf8Test::testUtf8()
{
	TU_INIT_TESTCASE("testUtf8");

	// Construct a wide string with characters big enough to need multiple bytes.
	//                                                = Is UTF8 leading bits
	//                                                - Is the actual character
	const wchar_t wchars[] = 
	{
		L'\x41',	//  7 bits = 1 byte UTF8 =                     0100 0001 = 0x41
					//                                             =--- ----
		L'\x81',	//  8 bits = 2 byte UTF8 =           1100 0010 1000 0001 = 0xC2 81
					//                                   ===- ---- ==-- ----
		L'\x401', 	// 11 bits = 2 byte UTF8 =           1101 0000 1000 0001 = 0xD0 81 
					//                                   ===- ---- ==-- ----
		L'\x801',	// 12 bits = 3 byte UTF8 = 1110 0000 1010 0000 1000 0001 = 0xE0 A0 81
					//                         ==== ---- ==-- ---- ==-- ----
		L'\x8001',  // 16 bits = 3 byte UTF8 = 1110 1000 1000 0000 1000 0001 = 0xE8 80 81 
					//                         ==== ---- ==-- ---- ==-- ----
		L'\x44',    //  7 bits = 1 byte UTF8 =                     0100 0100 = 0x44
					//                                             =--- ---- 
		L'\x00'     //                       =                     0000 0000 = 0x00 
					//                                             =--- ----
	};

	wstring wstr;
	wstr.assign(wchars,wcslen(wchars));


	// Construct a UTF8 string that is equivalent to the above.
	const char chars[] = 
	{ 
		'\x41', '\xc2', '\x81', '\xd0', '\x81', '\xe0', '\xa0', 
		'\x81', '\xe8', '\x80', '\x81', '\x44', '\x00'
	};

	string str;
	str.assign(chars,strlen(chars));

	// Generate a new UTF8 string fromt the wide string
	string new_str = ConvertToUtf8(wstr);

	// Compare the two UTF8 strings.
	CPPUNIT_ASSERT(new_str == str);
}

/**
 * Test the boundaries of the Utf8 strings.
 */
void Utf8Test::testFromUtf8()
{
	TU_INIT_TESTCASE("testFromUtf8");

	// Construct a UTF8 string that has 1, 2 and 3 byte strings.
	const char chars[] = 
	{ 
		'\x41', '\xc2', '\x81', '\xd0', '\x81', '\xe0', '\xa0', 
		'\x81', '\xe8', '\x80', '\x81', '\x44', '\x00'
	};

	string str;
	str.assign(chars,strlen(chars));
	
	// Construct a wide string that is equivalent to the above.
	//                                                = Is UTF8 leading bits
	//                                                - Is the actual character

	const wchar_t wchars[] = 
	{
		L'\x41',	//  7 bits = 1 byte UTF8 =                     0100 0001 = 0x41
					//                                             =--- ----
		L'\x81',	//  8 bits = 2 byte UTF8 =           1100 0010 1000 0001 = 0xC2 81
					//                                   ===- ---- ==-- ----
		L'\x401', 	// 11 bits = 2 byte UTF8 =           1101 0000 1000 0001 = 0xD0 81 
					//                                   ===- ---- ==-- ----
		L'\x801',	// 12 bits = 3 byte UTF8 = 1110 0000 1010 0000 1000 0001 = 0xE0 A0 81
					//                         ==== ---- ==-- ---- ==-- ----
		L'\x8001',  // 16 bits = 3 byte UTF8 = 1110 1000 1000 0000 1000 0001 = 0xE8 80 81 
					//                         ==== ---- ==-- ---- ==-- ----
		L'\x44',    //  7 bits = 1 byte UTF8 =                     0100 0100 = 0x44
					//                                             =--- ---- 
		L'\x00'     //                       =                     0000 0000 = 0x00 
					//                                             =--- ----
	};

	wstring wstr;
	wstr.assign(wchars,wcslen(wchars));

	// Generate a new wide string from the UTF8 string.
	wstring new_wstr = ConvertFromUtf8(str);

	// Compare the two wide strings.
	CPPUNIT_ASSERT(new_wstr == wstr);

}

/* This test is deleted because these are only internal routines. It will
be documented that the conversion will give inaccurate results if Utf8 strings
that are too long are used.

void Utf8Test::testTruncateFromUtf8()
{
	TU_INIT_TESTCASE("testTruncateFromUtf8");

	// Construct a UTF8 string that has 4 byte strings to big for a wide string character.
	char* chars = new char[17];
	chars[0]  = 0x48; // 
	chars[1]  = 0xf4; // 1111 0xxx where    xxx = 100
	chars[2]  = 0x80; // 10xx xxxx where xxxxxx = 000000
	chars[3]  = 0x81; // 10xx xxxx where xxxxxx = 000001
	chars[4]  = 0x89; // 10xx xxxx where xxxxxx = 001001 - should truncate to 0x49
	chars[5]  = 0;
	//chars[5]  = 0x50; 
	//chars[6]  = 0xf7; // 1111 0xxx where    xxx = 111
	//chars[7]  = 0xb0; // 10xx xxxx where xxxxxx = 110000
	//chars[8]  = 0x81; // 10xx xxxx where xxxxxx = 000001
	//chars[9]  = 0x91; // 10xx xxxx where xxxxxx = 010001 - should truncate to 0x51
	//chars[10] = 0x52; 
	//chars[11] = 0xf7; // 1111 0xxx where    xxx = 111
	//chars[12] = 0xb8; // 10xx xxxx where xxxxxx = 111000
	//chars[13] = 0x81; // 10xx xxxx where xxxxxx = 000001
	//chars[14] = 0x93; // 10xx xxxx where xxxxxx = 010011 - should truncate to 0x8053
	//chars[15] = 0x54; 
	//chars[16] = 0; 

	string str;
	str.assign(chars,strlen(chars));
	
	// Construct a wide string that is equivalent to the above
	// after the truncation occurs.
	wchar_t* wchars = new wchar_t[8];
	wchars[0]=0x48;
	wchars[1]=0x49;
	wchars[2]=0x50;
	wchars[3]=0x51;
	wchars[4]=0x52;
	wchars[5]=0x8053;
	wchars[6]=0x54;
	wchars[7]=0;     
	
	wstring wstr;
	wstr.assign(wchars,wcslen(wchars));

	// Generate a new wide string from the UTF8 string.
	wstring new_wstr = ConvertFromUtf8(str);


	// If you are having problems, print the two strings.
	const wchar_t* wch = wstr.c_str();
	const wchar_t* new_wch = new_wstr.c_str();
	const char* ch = (char*)wch;
	const char* new_ch = (char*)new_wch;

	int i;
	printf("\nwch =");
	for(i = 0; i < 13; i++){
		printf("%x ",ch[i]);
	}

	printf("\nnew_wch =");
	for(i = 0; i < 13; i++){
		printf("%x ",new_ch[i]);
	}



	// Compare the two wide strings.
	CPPUNIT_ASSERT(new_wstr == wstr);
}//*/



void Utf8Test::testSerialization()
{
	TU_INIT_TESTCASE("testSerialization");
	char tmp[258];

	ZeroMemory(tmp, 258);

	for (int i = 0; i < 256; i++)
	{
		tmp[i] = (unsigned char)i;
	}

	string v(tmp, 256);
	TU_dumpHex("v", v);

	wstring ws = ConvertToWide(v);


	string s = ConvertToNarrow(ws);
	TU_dumpHex("s", s);

	CPPUNIT_ASSERT(s.length() == v.length());

	const char* a = s.c_str();
	const char* b = v.c_str();

	int r = memcmp(a, b, s.length());

	CPPUNIT_ASSERT(r == 0);

	CPPUNIT_ASSERT(s == v);
}

