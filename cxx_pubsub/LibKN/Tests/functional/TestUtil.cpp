/**
 * Test Utilites
 */
#include "stdafx.h"
#include "Globals.h"
#include "TestUtil.h"
#include <LibKN\Connector.h>
#include <LibKN\StrUtil.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <vector>
using std::vector;
using std::cout;

string TU_TESTSUITE = "";

// -verbose
// True if ALL. If false, check the vector.
bool TU_VERBOSE_ALL = false;
vector<string> tu_verbose;

// -tests
// True if ALL. If false, check the vector.
bool TU_TESTALL = true;
vector<string> tu_tests;

// -servers
// If a list then set to tu_servers[0]. If no list then vector[0] is set to TU_SERVER.
string TU_SERVER = "";
vector<string> tu_servers;

// -servername
string TU_SERVER_NAME = "";

// -abridgeON/OFF
bool TU_DUMP_ABRIDGED_HEX = true;

// -msgHexON/OFF
bool TU_DUMP_MSG_HEX = false;

// -subPayloadON/OFF
bool TU_SUB_PAYLOAD = false;

bool TU_SUB_EVENTS_BRIEF = false;
bool TU_SUB_EVENTS_FULL = false;

bool TU_PUB_EVENTS_BRIEF = false;
bool TU_PUB_EVENTS_FULL = false;


vector<wstring> tu_pubPayloads;
vector<wstring> tu_subPayloads;

int tu_count_subListener_OnUpdate = 0;
int tu_count_subConnSH_OnConnStatus = 0;
int tu_count_subReqSH_OnError = 0;
int tu_count_subReqSH_OnSuccess = 0;
int tu_count_pubConnSH_OnConnStatus = 0;
int tu_count_pubReqSH_OnError = 0;
int tu_count_pubReqSH_OnSuccess = 0;





// forward declarations
void TU_dump(const char * text);




//////////////////////////////////////////////////////////////////////////////
/// Private functions ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void tu_displayHelp()
{
	cout << "----------- ------ -----------------------------------------------------------\n";
	cout << "Parameter   Values Description\n";
	cout << "----------- ------ -----------------------------------------------------------\n";
	cout << "-? -h              Display help (all other parameters are ignored).\n";
	cout << "-verbose           Runs all test suites with verbose output.\n";
	cout << "-verbose    all    Runs all test suites with verbose output.\n";
	cout << "-verbose    <list> A list of test suites to run with verbose output.\n";
	cout << "-tests             Runs all test suites.\n";
	cout << "-tests      all    Runs all test suites.\n";
	cout << "-tests      <name> A test suites to run. NOTE: All other test suites report\n";
	cout << "                     that they have passed even though they did not run.\n";
	cout << "-tests      <list> A list of test suites to run.\n";
	cout << "-servers    <name> A server URI for the tests to use.\n";
	cout << "                     The default is: DEFAULT_SERVER_URI in the Globals.h file.\n";
	cout << "                     Use TU_getServerURI() to get the value in a test.\n";
	cout << "-servers    <list> A list of server URIs.\n";
	cout << "                     Use TU_getServerURI() to get the first value in a test.\n";
	cout << "                     Use TU_getServerURI(i) to get the 0:x value in a test.\n";
	cout << "                     TU_getServerURI() is equivalent to TU_getServerURI(0).\n";
	cout << "-servername <name> The SCM name for the main server (-servers)\n";
	cout << "                     The default is:DEFAULT_SERVER_NAME in the Globals.h file.\n";
	cout << "                     Use TU_getServerName() to get the value in a test.\n";
	cout << "-msgHexON          Define this if you want dumps of messages to be in Hex.\n";
	cout << "-msgHexOFF           The default is OFF.\n";
	cout << "-abridgeON         Define this if you want to abridge large hex dumps.\n";
	cout << "-abridgeOFF          It restricts to first and last 4 lines of a hex dump.\n";
	cout << "                     The default is ON.\n";
	cout << "-subPayloadON      Report payload from subscriber event listener.\n";
	cout << "-subPayloadOFF        Default is OFF. Verbose must be ON.\n";
	cout << "-pubEventsOFF      Publish event reporting.\n";
	cout << "-pubEventsBRIEF      Default is OFF.\n";
	cout << "-pubEventsFULL       Verbose must be ON.\n";
	cout << "-subEventsOFF      Subscribe event reporting.\n";
	cout << "-subEventsBRIEF      Default is OFF.\n";
	cout << "-subEventsFULL       Verbose must be ON.\n";
	cout << "----------- ------ -----------------------------------------------------------\n";
}

void tu_getFileName(char * tok, const char * str)
{
	char tmp[1024];
	strcpy(tmp,str);

	char *p;
	char *last;
	p = strtok(tmp,"\\/");
	do{
		last = p;
		p = strtok('\0',"\\/");
	}while(p);

	// remove .cpp
	int len = strlen(last);
	if(len > 4)
	{
		last[len-4] = '\0';
	}

	strcpy(tok,last);
}

bool tu_isVerbose()
{
	if(TU_VERBOSE_ALL) return true;
	
	bool result = false;
	for(int i = 0; i < tu_verbose.size(); i++)
	{
		if(TU_TESTSUITE == tu_verbose[i])
		{
			return true;
		}
	}
	return result;
}

bool tu_runTest()
{
	if(TU_TESTALL) return true;

	bool result = false;
	for(int i = 0; i < tu_tests.size(); i++)
	{
		if(TU_TESTSUITE == tu_tests[i])
		{
			return true;
		}
	}

	return result;
}

void tu_loadVector(vector<string> &v, const char * str)
{
	char tmp[1024];
	strcpy(tmp,str);

	char *p;
	p = strtok(tmp," ");

	if(!p) return;

	do{
		v.push_back(p);
		p = strtok('\0'," ");
	}while(p);
}

void tu_dumpParams()
{
	// This is called before a test case, so if any test case is verbose do it.
	if(!tu_isVerbose() && tu_verbose.size()==0) return;

	cout << "===============================================================================\n";
	int i = 0;
	cout << "TU_VERBOSE_ALL=" << TU_VERBOSE_ALL << "\n";
	for( i = 0; i < tu_verbose.size(); i++)
	{
		cout << "\t" << i << "=" << tu_verbose[i] << "\n";
	}
	cout << "TU_TESTALL=" << TU_TESTALL << "\n";
	for( i = 0; i < tu_tests.size(); i++)
	{
		cout << "\t" << i << "=" << tu_tests[i] << "\n";
	}
	cout << "TU_SERVER=" << TU_SERVER << "\n";
	for( i = 0; i < tu_servers.size(); i++)
	{
		cout << "\t" << i << "=" << tu_servers[i] << "\n";
	}
	cout << "TU_SERVER_NAME=" << TU_SERVER_NAME << "\n";
	cout << "TU_DUMP_ABRIDGED_HEX=" << TU_DUMP_MSG_HEX << "\n";
	cout << "TU_DUMP_MSG_HEX=" << TU_DUMP_MSG_HEX << "\n";
	cout << "===============================================================================\n";
}

void tu_dumpHex(const char* text, const string& s)
{
	if(!tu_isVerbose()) return;

	cout << text << " (" << s.length() << ")";

	if(TU_DUMP_ABRIDGED_HEX)
	{
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
	}else{
		for (int i = 0; i < s.length(); i++)
		{
			if (i % 16 == 0)
				printf("\n");
			else
				printf(" ");
			unsigned char c = s.at(i);
			printf("  %02X", c);
		}
	}
	printf("\n");
}

static void tu_dumpHex(const char* text, const wstring& s)
{
	if(!tu_isVerbose()) return;

	cout << text << " (" << s.length() << ")";

	if(TU_DUMP_ABRIDGED_HEX)
	{
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
	}else{
		for (int i = 0; i < s.length(); i++)
		{
			if (i % 16 == 0)
				printf("\n");
			else
				printf(" ");
			unsigned int c = s.at(i);
			printf("%04X", c);
		}
	}
	printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
// Server Tools ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 
//  tu_DisplayError()
// 
//  PURPOSE :     This is a helper function to display an error message 
//                if a function in _tmain() fails.
// 
//  PARAMETERS:   szAPI - the name of the function that failed
// 
//                dwError - the Win32 error code indicating why the
//                function failed
// 
//  RETURN VALUE: None
// 
//**********************************************************************

void tu_DisplayError( LPTSTR szAPI, DWORD dwError ) 
{
	if(!tu_isVerbose()) return;

   LPTSTR lpBuffer = NULL;

   FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
         FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         (LPTSTR) &lpBuffer, 0, NULL );

   _tprintf( TEXT("%s failed:\n"), szAPI );
   _tprintf( TEXT("    error code = %u\n"), dwError );
   _tprintf( TEXT("    message    = %s\n"), lpBuffer );

   LocalFree( lpBuffer );
}

//**********************************************************************
// 
//  tu_StopService_()
// 
//  PURPOSE :     This function attempts to stop a service. It allows
//                the caller to specify whether dependent services
//                should also be stopped. It also allows a timeout
//                value to be passed, to prevent a scenario in which a
//                service shutdown hangs, and in turn the application
//                stopping the service hangs.
// 
//  PARAMETERS:   hSCM - open handle to the service control manager
//                hService - open handle to the service to be stopped
//                fStopDependencies - flag indicating whether to stop
//                   dependent services
//                dwTimeout - maximum time (in milliseconds) to wait
//                   for the service and its dependencies to stop
// 
//  RETURN VALUE: If the operation is successful, ERROR_SUCCESS is 
//                returned. Otherwise, a Win32 error code is returned.
// 
//**********************************************************************

DWORD tu_StopService_( SC_HANDLE hSCM, SC_HANDLE hService, 
      BOOL fStopDependencies, DWORD dwTimeout ) {

   SERVICE_STATUS ss;
   DWORD dwStartTime = GetTickCount();

   // Make sure the service is not already stopped
   if ( !QueryServiceStatus( hService, &ss ) )
      return GetLastError();

   if ( ss.dwCurrentState == SERVICE_STOPPED ) 
      return ERROR_SUCCESS;

   // If a stop is pending, just wait for it
   while ( ss.dwCurrentState == SERVICE_STOP_PENDING ) {
	   TU_dump("Stop_pending(tu_StopService_)");

      Sleep( ss.dwWaitHint );
      if ( !QueryServiceStatus( hService, &ss ) )
         return GetLastError();

      if ( ss.dwCurrentState == SERVICE_STOPPED )
         return ERROR_SUCCESS;

      if ( GetTickCount() - dwStartTime > dwTimeout )
         return ERROR_TIMEOUT;
   }

   // If the service is running, dependencies must be stopped first
   if ( fStopDependencies ) {

      DWORD i;
      DWORD dwBytesNeeded;
      DWORD dwCount;

      LPENUM_SERVICE_STATUS   lpDependencies = NULL;
      ENUM_SERVICE_STATUS     ess;
      SC_HANDLE               hDepService;

      // Pass a zero-length buffer to get the required buffer size
      if ( EnumDependentServices( hService, SERVICE_ACTIVE, 
         lpDependencies, 0, &dwBytesNeeded, &dwCount ) ) {

         // If the Enum call succeeds, then there are no dependent
         // services so do nothing

      } else {
         TU_dump("Stopping dependencies.");

         if ( GetLastError() != ERROR_MORE_DATA )
            return GetLastError(); // Unexpected error

         // Allocate a buffer for the dependencies
         lpDependencies = (LPENUM_SERVICE_STATUS) HeapAlloc( 
               GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded );

         if ( !lpDependencies )
            return GetLastError();

         __try {

            // Enumerate the dependencies
            if ( !EnumDependentServices( hService, SERVICE_ACTIVE, 
                  lpDependencies, dwBytesNeeded, &dwBytesNeeded,
                  &dwCount ) )
               return GetLastError();

            for ( i = 0; i < dwCount; i++ ) {

               ess = *(lpDependencies + i);

               // Open the service
               hDepService = OpenService( hSCM, ess.lpServiceName, 
                     SERVICE_STOP | SERVICE_QUERY_STATUS );
               if ( !hDepService )
                  return GetLastError();

               __try {

                  // Send a stop code
                  if ( !ControlService( hDepService, SERVICE_CONTROL_STOP,
                        &ss ) )
                     return GetLastError();

                  // Wait for the service to stop
                  while ( ss.dwCurrentState != SERVICE_STOPPED ) {

                     Sleep( ss.dwWaitHint );
                     if ( !QueryServiceStatus( hDepService, &ss ) )
                        return GetLastError();

                     if ( ss.dwCurrentState == SERVICE_STOPPED )
                        break;

                     if ( GetTickCount() - dwStartTime > dwTimeout )
                        return ERROR_TIMEOUT;
                  }

               } __finally {

                  // Always release the service handle
                  CloseServiceHandle( hDepService );

               }

            }

         } __finally {

            // Always free the enumeration buffer
            HeapFree( GetProcessHeap(), 0, lpDependencies );

         }
      } 
   }

   // Send a stop code to the main service
   if ( !ControlService( hService, SERVICE_CONTROL_STOP, &ss ) )
      return GetLastError();

   // Wait for the service to stop
   while ( ss.dwCurrentState != SERVICE_STOPPED ) {
	   TU_dump("Wait for stopped(tu_StopService_)");

      Sleep( ss.dwWaitHint );
      if ( !QueryServiceStatus( hService, &ss ) )
         return GetLastError();

      if ( ss.dwCurrentState == SERVICE_STOPPED )
         break;

      if ( GetTickCount() - dwStartTime > dwTimeout )
         return ERROR_TIMEOUT;
   }

   // Return success
   return ERROR_SUCCESS;
}


DWORD tu_StartService_( SC_HANDLE hSCM, SC_HANDLE hService, DWORD dwTimeout ) 
{

	SERVICE_STATUS ss;
	DWORD dwStartTime = GetTickCount();
	DWORD     dwError;

	// Make sure the service is not already stopped
	if ( !QueryServiceStatus( hService, &ss ) )
		return GetLastError();

	if ( ss.dwCurrentState == SERVICE_RUNNING ) 
		return ERROR_SUCCESS;

	// If a stop is pending, just wait for it
	while ( ss.dwCurrentState == SERVICE_STOP_PENDING ) {
	   TU_dump("Stop_pending(StartService_)\n");

		Sleep( ss.dwWaitHint );
		if ( !QueryServiceStatus( hService, &ss ) )
			return GetLastError();

		if ( ss.dwCurrentState == SERVICE_STOPPED )
			return ERROR_SUCCESS;

		if ( GetTickCount() - dwStartTime > dwTimeout )
			return ERROR_TIMEOUT;
	}


    // Start the service.
	dwError = StartService( hService, 0, NULL );

	// For some reason an error is returned here but things may still work.
	//         StartService() failed:
	//         error code = 1
	//         message    = Incorrect function.
    //if ( dwError == ERROR_SUCCESS )
    //   tu_DumpStr("Service started(wait for running).");
    //else
    //   tu_DisplayError( TEXT("StartService()"), dwError );


	// Wait for the service to start
	while ( ss.dwCurrentState != SERVICE_RUNNING ) {
	   TU_dump("Wait for running(StartService_)");

		Sleep( ss.dwWaitHint );
		if ( !QueryServiceStatus( hService, &ss ) )
			return GetLastError();

		if ( ss.dwCurrentState == SERVICE_RUNNING )
			break;

		if ( GetTickCount() - dwStartTime > dwTimeout )
			return ERROR_TIMEOUT;
	}

	// Return success
	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
/// Public functions /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/*
 * This is public but the caller should use: TU_INIT_TESTSUITE(i,j).
 */
string tu_initTestSuite(int argc_, char* argv_[], const char * file)
{
	string error = "";

	char fBuf[512];
	tu_getFileName(fBuf,file);
	TU_TESTSUITE = fBuf;

	// set defaults
	TU_VERBOSE_ALL = false;
	TU_TESTALL = true;
	TU_SERVER = DEFAULT_SERVER_URI;
	TU_SERVER_NAME = DEFAULT_SERVER_NAME;
	TU_DUMP_ABRIDGED_HEX = true;
	TU_DUMP_MSG_HEX = false;
	TU_SUB_EVENTS_BRIEF = false;
	TU_SUB_EVENTS_FULL = false;
	TU_PUB_EVENTS_BRIEF = false;
	TU_PUB_EVENTS_FULL = false;

	for(int i = 1; i < argc_; i++)
	{
		string param = argv_[i];

		if(param == "-help" || param == "-h" || param == "-?" || param == "-HELP" 
			|| param == "-Help" || param == "-H")
		{
			tu_displayHelp();
			exit(1);
		}
		else if(param == "-verbose")
		{
			TU_VERBOSE_ALL = true;
			if(i+1 < argc_)
			{
				string arg = argv_[i+1];
				if(argv_[i+1][0] == '-')
				{
					TU_VERBOSE_ALL = true;
				}else if(arg == "ALL" || arg == "All" || arg == "all")
				{
					TU_VERBOSE_ALL = true;
					i++; //advance
				}else
				{
					TU_VERBOSE_ALL = false;
					tu_loadVector(tu_verbose,argv_[i+1]);
					i++; //advance
				}
			}
		}
		else if(param == "-servers")
		{
			TU_SERVER = DEFAULT_SERVER_URI;
			if(i+1 < argc_)
			{
				// at least one more param
				string arg = argv_[i+1];
				if(argv_[i+1][0] == '-')
				{
					// next param, no values for this param.
					return "Parameter -servers requires a value or list.";
				}else
				{
					// the values for this param
					tu_loadVector(tu_servers,argv_[i+1]);
					TU_SERVER = tu_servers[0];
					i++; //advance
				}
			}else{
				// no more params.
				return "Parameter -servers requires a value or list.";
			}
		}
		else if(param == "-tests")
		{
			TU_TESTALL = true;
			if(i+1 < argc_)
			{
				string arg = argv_[i+1];
				if(argv_[i+1][0] == '-')
				{
					return "Parameter -tests requires a value or list.";
				}else if(arg == "ALL" || arg == "All" || arg == "all")
				{
					i++; //advance
				}else
				{
					TU_TESTALL = false;
					tu_loadVector(tu_tests,argv_[i+1]);
					i++; //advance
				}
			}else{
				return "Parameter -tests requires a value or list.";
			}
		}
		else if(param == "-servername")
		{
			TU_SERVER_NAME = DEFAULT_SERVER_NAME;
			if(i+1 < argc_)
			{
				// at lest one more param
				string arg = argv_[i+1];
				if(argv_[i+1][0] == '-')
				{
					// next param, no values for this param.
					return "Parameter -servername requires a value.";
				}else
				{
					// the value for this param
					TU_SERVER_NAME = arg;
					i++; //advance
				}
			}else{
				// no more params.
				return "Parameter -servername requires a value.";
			}
		}
		else if(param == "-abridgeON")
		{
			TU_DUMP_ABRIDGED_HEX = true;
		}
		else if(param == "-abridgeOFF")
		{
			TU_DUMP_ABRIDGED_HEX = false;
		}
		else if(param == "-msgHexON")
		{
			TU_DUMP_MSG_HEX = true;
		}
		else if(param == "-msgHexOFF")
		{
			TU_DUMP_MSG_HEX = false;
		}
		else if(param == "-subPayloadON")
		{
			TU_SUB_PAYLOAD = true;
		}
		else if(param == "-subPayloadOFF")
		{
			TU_SUB_PAYLOAD = false;
		}
		else if(param == "-subEventsOFF")
		{
			TU_SUB_EVENTS_BRIEF = false;
			TU_SUB_EVENTS_FULL = false;
		}
		else if(param == "-subEventsBRIEF")
		{
			TU_SUB_EVENTS_BRIEF = true;
			TU_SUB_EVENTS_FULL = false;
		}
		else if(param == "-subEventsFULL")
		{
			TU_SUB_EVENTS_BRIEF = false;
			TU_SUB_EVENTS_FULL = true;
		}
		else if(param == "-pubEventsOFF")
		{
			TU_PUB_EVENTS_BRIEF = false;
			TU_PUB_EVENTS_FULL = false;
		}
		else if(param == "-pubEventsBRIEF")
		{
			TU_PUB_EVENTS_BRIEF = true;
			TU_PUB_EVENTS_FULL = false;
		}
		else if(param == "-pubEventsFULL")
		{
			TU_PUB_EVENTS_BRIEF = false;
			TU_PUB_EVENTS_FULL = true;
		}
		else
		{
			return (string)"Unknown parameter: " + param + (string)"\n";
		}

	}

	// Set any params with <list> that did not get set.
	if(tu_servers.size() == 0)
	{
		TU_SERVER = DEFAULT_SERVER_URI;
		tu_servers.push_back(DEFAULT_SERVER_URI);
	}

	tu_dumpParams();

	return error;
}

/*
 * This is public but the caller should use: TU_INIT_TESTCASE(i).
 */
bool tu_initTestCase(const char * testCase, const char * file)
{
	char fBuf[512];
	tu_getFileName(fBuf,file);
	TU_TESTSUITE = fBuf;

	if(!tu_runTest()) return false;
	if(!tu_isVerbose()) return true;

	cout << "\n";
	cout << "===============================================================================\n";
	cout << "=== Start test: " << TU_TESTSUITE << "::" << testCase << "\n";
	cout << "===============================================================================\n";
	return true;
}


void TU_dump(const char * text)
{
	if(!tu_isVerbose()) return;

	cout << "<" << TU_TESTSUITE << "> " << text << "\n";
}


void TU_dump(string text)
{
	if(!tu_isVerbose()) return;

	cout << "<" << TU_TESTSUITE << "> " << text.c_str() << "\n";
}

void TU_dump(wstring wstr)
{
	if(!tu_isVerbose()) return;

	cout << "<" << TU_TESTSUITE << "> ";
	printf("%S\n",wstr.c_str());
}

void TU_dump(const char* text, int i)
{
	if(!tu_isVerbose()) return;

	char iBuf[12];
	itoa(i,iBuf,10);

	cout << "<" << TU_TESTSUITE << "> " << text << iBuf << "\n";
}

void TU_dump(const char* text, string str)
{
	if(!tu_isVerbose()) return;

	cout << "<" << TU_TESTSUITE << "> " << text << " : " << str.c_str() << "\n";
}

void TU_dump(const char* text, wstring wstr)
{
	if(!tu_isVerbose()) return;

	cout << "<" << TU_TESTSUITE << "> " << text << " : ";
	printf("%S\n",wstr.c_str());
}

void TU_dumpHex(const char* text, const string& str)
{
	if(!tu_isVerbose()) return;

	cout << "<" << TU_TESTSUITE << "> ";
	tu_dumpHex(text,str);
}

void TU_dumpHex(const char* text, const wstring& wstr)
{
	if(!tu_isVerbose()) return;

	cout << "<" << TU_TESTSUITE << "> ";
	tu_dumpHex(text,wstr);
}

void TU_dumpMsg(const char* text, const Message& msg)
{
	if(!tu_isVerbose()) return;

	cout << "<" << TU_TESTSUITE << "> " << text << " : \n";

	for (Message::Container::const_iterator it = 
			msg.GetContainer().begin(); 
			it != msg.GetContainer().end(); ++it)
	{
		printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
		if(TU_DUMP_MSG_HEX)
		{
			tu_dumpHex("value=",(*it).second);
		}
	}
}

void TU_dumpMsgKey(const char* text, const Message& msg, wstring key)
{
	if(!tu_isVerbose()) return;

	cout << "<" << TU_TESTSUITE << "> " << text << " : ";

	//printf("<%s> %s : ",TU_TESTSUITE , text);
	for (Message::Container::const_iterator it = 
			msg.GetContainer().begin(); 
			it != msg.GetContainer().end(); ++it)
	{
		if((*it).first == key)
		{
			printf("%S = %S ", (*it).first.c_str(), (*it).second.c_str());
		}
	}
	printf("\n");
}


void TU_dumpPubPayloads()
{
	if(!tu_isVerbose()) return;

	cout << "<" << TU_TESTSUITE << "> Pub Payloads<wstring>: " << tu_pubPayloads.size() << "\n";
	for(int i = 0; i < tu_pubPayloads.size(); i++)
	{
		printf("\t%d:%S\n", i, tu_pubPayloads[i].c_str());
	}
}

void TU_dumpSubPayloads()
{
	if(!tu_isVerbose()) return;

	cout << "<" << TU_TESTSUITE << "> Sub Payloads<wstring>: " << tu_subPayloads.size() << "\n";
	for(int i = 0; i < tu_subPayloads.size(); i++)
	{
		printf("\t%d:%S\n", i, tu_subPayloads[i].c_str());
	}
}

void TU_resetPubPayloads()
{
	if(!tu_isVerbose()) return;

	int n = tu_pubPayloads.size();
	for(int i = 0; i < n; i++)
	{
		tu_pubPayloads.pop_back();
	}
}

void TU_resetSubPayloads()
{
	if(!tu_isVerbose()) return;

	int n = tu_subPayloads.size();
	for(int i = 0; i < n; i++)
	{
		tu_subPayloads.pop_back();
	}
}

void TU_addPubPayload(wstring pl)
{
	tu_pubPayloads.push_back(pl);
}

void TU_addSubPayload(wstring pl)
{
	tu_subPayloads.push_back(pl);
}

wstring TU_getPubPayload(int index)
{
	if(index < tu_pubPayloads.size())
	{
		return tu_pubPayloads[index];
	}else{
		return NULL;
	}
}

wstring TU_getSubPayload(int index)
{
	if(index < tu_subPayloads.size())
	{
		return tu_subPayloads[index];
	}else{
		return NULL;
	}
}




void TU_resetCounters()
{
	tu_count_pubConnSH_OnConnStatus = 0;
	tu_count_pubReqSH_OnError = 0;
	tu_count_pubReqSH_OnSuccess = 0;
	tu_count_subListener_OnUpdate = 0;
	tu_count_subConnSH_OnConnStatus = 0;
	tu_count_subReqSH_OnError = 0;
	tu_count_subReqSH_OnSuccess = 0;
}

void TU_dumpCounters()
{
	if(!tu_isVerbose()) return;

	cout << "<" << TU_TESTSUITE << "> Event Counters:\n";
	printf("\tpub ConnSH   OnConnStatus =%d\n",tu_count_pubConnSH_OnConnStatus);
	printf("\tpub ReqSH    OnError      =%d\n",tu_count_pubReqSH_OnError);
	printf("\tpub ReqSH    OnSuccess    =%d\n",tu_count_pubReqSH_OnSuccess);
	printf("\tsub Listener OnUpdate     =%d\n",tu_count_subListener_OnUpdate);
	printf("\tsub ConnSH   OnConnStatus =%d\n",tu_count_subConnSH_OnConnStatus);
	printf("\tsub ReqSH    OnError      =%d\n",tu_count_subReqSH_OnError);
	printf("\tsub ReqSH    OnSuccess    =%d\n",tu_count_subReqSH_OnSuccess);
}

int TU_getCount_pubConnSH_OnConnStatus()
{
	return tu_count_pubConnSH_OnConnStatus;
}

int TU_getCount_pubReqSH_OnError()
{
	return tu_count_pubReqSH_OnError;
}

int TU_getCount_pubReqSH_OnSuccess()
{
	return tu_count_pubReqSH_OnSuccess;
}

int TU_getCount_subListener_OnUpdate()
{
	return tu_count_subListener_OnUpdate;
}

int TU_getCount_subConnSH_OnConnStatus()
{
	return tu_count_subConnSH_OnConnStatus;
}

int TU_getCount_subReqSH_OnError()
{
	return tu_count_subReqSH_OnError;
}

int TU_getCount_subReqSH_OnSuccess()
{
	return tu_count_subReqSH_OnSuccess;
}


string TU_getServerURI()
{
	return TU_SERVER;
}

string TU_getServerURI(int index)
{
	if(index < tu_servers.size())
	{
		return tu_servers[index];
	}
	else
	{
		return "";
	}
}

wstring TU_getwServerURI()
{
	return ConvertToWide(TU_SERVER);
}

wstring TU_getwServerURI(int index)
{
	if(index < tu_servers.size())
	{
		return ConvertToWide(tu_servers[index]);
	}
	else
	{
		return L"";
	}
}

string TU_getServerName()
{
	return TU_SERVER_NAME;
}

wstring TU_getwServerName()
{
	return ConvertToWide(TU_SERVER_NAME);
}


void TU_StopKnowNowServer()
{

	SC_HANDLE hSCM;
	SC_HANDLE hService;
	DWORD     dwError;

	TU_dump("Stoping Service.");

   __try {

      // Open the SCM database
      hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT );
      if ( !hSCM ) {
         tu_DisplayError( TEXT("OpenSCManager()"), GetLastError() );
         __leave;
      }

      // Open the specified service
      hService = OpenService( hSCM
		  , TU_SERVER_NAME.c_str()
		  , SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS );
      if ( !hService ) {
         tu_DisplayError( TEXT("OpenService()"), GetLastError() );
         __leave;
      }

      // Try to stop the service, specifying a 30 second timeout
      dwError = tu_StopService_( hSCM, hService, TRUE, 30000 ) ;
      if ( dwError == ERROR_SUCCESS )
	  {
         TU_dump("Service stopped.");
	  }
      else
         tu_DisplayError( TEXT("StopService()"), dwError );

   } __finally {

      if ( hService )
         CloseServiceHandle( hService );

      if ( hSCM )
         CloseServiceHandle( hSCM );
   }

}

/**
 * To get a list of services go to:
 * Contol Panel - Administration - Services
 * KnowNow Notification Server
 * KnowNow Server
 */
void TU_StartKnowNowServer()
{

	SC_HANDLE hSCM;
	SC_HANDLE hService;
	DWORD     dwError;

	TU_dump("Starting Service.");

   __try {

      // Open the SCM database
      hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT );
      if ( !hSCM ) {
         tu_DisplayError( TEXT("OpenSCManager()"), GetLastError() );
         __leave;
      }

      // Open the specified service
      hService = OpenService( hSCM
		  , TU_SERVER_NAME.c_str()
		  , SERVICE_ALL_ACCESS );
      if ( !hService ) {
         tu_DisplayError( TEXT("OpenService()"), GetLastError() );
         __leave;
      }

      // Try to stop the service, specifying a 30 second timeout
      dwError = tu_StartService_( hSCM, hService, 30000 ) ;
      if ( dwError == ERROR_SUCCESS )
	  {
         TU_dump("Service started.");
	  }
      else
         tu_DisplayError( TEXT("StartService()"), dwError );

   } __finally {

      if ( hService )
         CloseServiceHandle( hService );

      if ( hSCM )
         CloseServiceHandle( hSCM );
   }

}


///////////////////////////////////////////////////////////////////////////////
// Classes ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TU_SubListener::OnUpdate(const Message& msg)
{
	if(TU_SUB_EVENTS_FULL)
		TU_dumpMsg("Sub Listener: OnUpdate", msg);
	else if(TU_SUB_PAYLOAD)
		TU_dumpMsgKey("Sub Listener: OnUpdate", msg, L"kn_payload");
	else if(TU_SUB_EVENTS_BRIEF)
		TU_dump("Sub Listener: OnUpdate");

	tu_count_subListener_OnUpdate++;
	for (Message::Container::const_iterator it = msg.GetContainer().begin();
		it != msg.GetContainer().end(); ++it)
	{
		wstring k = (*it).first;
		wstring v = (*it).second;
		if(k == L"kn_payload")
		{
			TU_addSubPayload(v);
		}
	}
};

void TU_SubConnStatusHandler::OnConnectionStatus(const Message& msg)
{
	if(TU_SUB_EVENTS_FULL)
		TU_dumpMsg("Sub ConnectionStatusHandler: OnConnectionStatus", msg);
	else if(TU_SUB_EVENTS_BRIEF)
		TU_dump("Sub ConnectionStatusHandler: OnConnectionStatus");

	tu_count_subConnSH_OnConnStatus++;
};

void TU_PubConnStatusHandler::OnConnectionStatus(const Message& msg)
{
	if(TU_PUB_EVENTS_FULL)
		TU_dumpMsg("Pub ConnectionStatusHandler: OnConnectionStatus", msg);
	else if(TU_PUB_EVENTS_BRIEF)
		TU_dump("Pub ConnectionStatusHandler: OnConnectionStatus");

	tu_count_pubConnSH_OnConnStatus++;
};

void TU_SubRequestStatusHandler::OnError(const Message& msg)
{
	if(TU_SUB_EVENTS_FULL)
		TU_dumpMsg("Sub RequestStatusHandler: OnError", msg);
	else if(TU_SUB_EVENTS_BRIEF)
		TU_dump("Sub RequestStatusHandler: OnError");

	tu_count_subReqSH_OnError++;
};

void TU_SubRequestStatusHandler::OnSuccess(const Message& msg)
{
	if(TU_SUB_EVENTS_FULL)
		TU_dumpMsg("Sub RequestStatusHandler: OnSuccess", msg);
	else if(TU_SUB_EVENTS_BRIEF)
		TU_dump("Sub RequestStatusHandler: OnSuccess");

	tu_count_subReqSH_OnSuccess++;
};

void TU_PubRequestStatusHandler::OnError(const Message& msg)
{
	if(TU_PUB_EVENTS_FULL)
		TU_dumpMsg("Pub RequestStatusHandler: OnError", msg);
	else if(TU_PUB_EVENTS_BRIEF)
		TU_dump("Pub RequestStatusHandler: OnError");

	tu_count_pubReqSH_OnError++;
};

void TU_PubRequestStatusHandler::OnSuccess(const Message& msg)
{
	if(TU_PUB_EVENTS_FULL)
		TU_dumpMsg("Pub RequestStatusHandler: OnSuccess", msg);
	else if(TU_PUB_EVENTS_BRIEF)
		TU_dump("Pub RequestStatusHandler: OnSuccess");

	tu_count_pubReqSH_OnSuccess++;
};

