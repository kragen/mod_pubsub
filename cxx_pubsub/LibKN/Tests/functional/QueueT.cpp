#include "stdafx.h"
#include <LibKN\Connector.h>
#include <LibKN\StrUtil.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <vector>
using std::vector;
using std::cout;



#define VERBOSE

/**
 *
 */

class QueueTest : public CPPUNIT_NS::TestFixture
{

	CPPUNIT_TEST_SUITE(QueueTest);
//	CPPUNIT_TEST(testStopStartServer);
	CPPUNIT_TEST(testPubSub);
//	CPPUNIT_TEST(testStartServer);
	CPPUNIT_TEST_SUITE_END();

public:
	// CPPUNIT_NS::TestFixture
	//
	void setUp() {}
	void tearDown() {}

	void testStopStartServer();
	void testPubSub();
	void testStartServer();

};

CPPUNIT_TEST_SUITE_REGISTRATION(QueueTest);

///////////////////////////////////////////////////////////////////////////////
// Variables //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int queT_count_subListener_OnUpdate = 0;
int queT_count_subConnSH_OnConnStatus = 0;
int queT_count_subReqSH_OnStatus = 0;
int queT_count_subReqSH_OnError = 0;
int queT_count_subReqSH_OnSuccess = 0;
int queT_count_pubConnSH_OnConnStatus = 0;
int queT_count_pubReqSH_OnStatus = 0;
int queT_count_pubReqSH_OnError = 0;
int queT_count_pubReqSH_OnSuccess = 0;

vector<wstring> pubPayloads;
vector<wstring> subPayloads;


///////////////////////////////////////////////////////////////////////////////
// Tools //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void QueT_ResetCounters()
{
	queT_count_pubConnSH_OnConnStatus = 0;
	queT_count_pubReqSH_OnStatus = 0;
	queT_count_pubReqSH_OnError = 0;
	queT_count_pubReqSH_OnSuccess = 0;
	queT_count_subListener_OnUpdate = 0;
	queT_count_subConnSH_OnConnStatus = 0;
	queT_count_subReqSH_OnStatus = 0;
	queT_count_subReqSH_OnError = 0;
	queT_count_subReqSH_OnSuccess = 0;
}

void QueT_DumpCounters()
{
	printf("queT_count_pubConnSH_OnConnStatus=%d\n",queT_count_pubConnSH_OnConnStatus);
	printf("queT_count_pubReqSH_OnStatus     =%d\n",queT_count_pubReqSH_OnStatus);
	printf("queT_count_pubReqSH_OnError      =%d\n",queT_count_pubReqSH_OnError);
	printf("queT_count_pubReqSH_OnSuccess    =%d\n",queT_count_pubReqSH_OnSuccess);
	printf("queT_count_subListener_OnUpdate  =%d\n",queT_count_subListener_OnUpdate);
	printf("queT_count_subConnSH_OnConnStatus=%d\n",queT_count_subConnSH_OnConnStatus);
	printf("queT_count_subReqSH_OnStatus     =%d\n",queT_count_subReqSH_OnStatus);
	printf("queT_count_subReqSH_OnError      =%d\n",queT_count_subReqSH_OnError);
	printf("queT_count_subReqSH_OnSuccess    =%d\n",queT_count_subReqSH_OnSuccess);
}

void QueT_DumpInit()
{
	printf("\n");
}

void QueT_DumpStr(const char* text)
{
	printf("%s\n", text);
}

void QueT_DumpStr(const char* text, int i)
{
	printf("%s%d\n", text, i);
}

void QueT_DumpStr(string text)
{
	printf("%s\n", text.c_str());
}

void QueT_DumpStr(wstring text)
{
	printf("%S\n", text.c_str());
}

void QueT_DumpMsg(const char* text, const Message& msg)
{
	printf("%s :\n", text);
	for (Message::Container::const_iterator it = 
			msg.GetContainer().begin(); 
			it != msg.GetContainer().end(); ++it)
	{
		printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
	}
}

void QueT_DumpMsgKey(const char* text, const Message& msg, wstring key)
{
	printf("%s : ", text);
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

void QueT_DumpPubPayloads()
{
	cout << "Pub Payloads<wstring>: " << pubPayloads.size() << "\n";
	for(int i = 0; i < pubPayloads.size(); i++)
	{
		printf("\t%d:%S\n", i, pubPayloads[i].c_str());
	}
}

void QueT_DumpSubPayloads()
{
	cout << "Sub Payloads<wstring>: " << subPayloads.size() << "\n";
	for(int i = 0; i < subPayloads.size(); i++)
	{
		printf("\t%d:%S\n", i, subPayloads[i].c_str());
	}
}

void QueT_ResetPubPayloads()
{
	int n = pubPayloads.size();
	for(int i = 0; i < n; i++)
	{
		pubPayloads.pop_back();
	}
}

void QueT_ResetSubPayloads()
{
	int n = subPayloads.size();
	for(int i = 0; i < n; i++)
	{
		subPayloads.pop_back();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Classes ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class QueT_SubListener : public IListener
{
public:
	void OnUpdate(const Message& msg)
	{
		//QueT_DumpMsg("<Sub>Listener: OnUpdate", msg);
		QueT_DumpMsgKey("<Sub>Listener: OnUpdate", msg, L"kn_payload");

		queT_count_subListener_OnUpdate++;
		for (Message::Container::const_iterator it = msg.GetContainer().begin();
			it != msg.GetContainer().end(); ++it)
		{
			wstring k = (*it).first;
			wstring v = (*it).second;
			if(k == L"kn_payload")
			{
				//QueT_DumpStr(L"Field="+k);
				subPayloads.push_back(v);
			}
		}
	}
};

class QueT_SubConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg)
	{
		//QueT_DumpMsg("<Sub>ConnectionStatusHandler: OnConnectionStatus", msg);
		queT_count_subConnSH_OnConnStatus++;
	}
};

class QueT_PubConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg)
	{
		//QueT_DumpMsg("<Pub>ConnectionStatusHandler: OnConnectionStatus", msg);
		queT_count_pubConnSH_OnConnStatus++;
	}
};

class QueT_SubRequestStatusHandler : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//QueT_DumpMsg("<Sub>Request Status Sucess: OnError", msg);
		queT_count_subReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//QueT_DumpMsg("<Sub>RequestStatusHandler: OnSuccess", msg);
		queT_count_subReqSH_OnSuccess++;
	}
};

class QueT_PubRequestStatusHandler : public IRequestStatusHandler
{
public:
	void OnError(const Message& msg)
	{
		//QueT_DumpMsg("<Pub>RequestStatusHandler: OnError", msg);
		queT_count_pubReqSH_OnError++;
	}
	void OnSuccess(const Message& msg)
	{
		//QueT_DumpMsg("<Pub>RequestStatusHandler: OnSuccess", msg);
		queT_count_pubReqSH_OnSuccess++;
	}
};


///////////////////////////////////////////////////////////////////////////////
// Server Tools ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//**********************************************************************
// 
//  QueT_DisplayError()
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

void QueT_DisplayError( LPTSTR szAPI, DWORD dwError ) {

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
//  QueT_StopService_()
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

DWORD QueT_StopService_( SC_HANDLE hSCM, SC_HANDLE hService, 
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
	   QueT_DumpStr("Stop_pending(QueT_StopService_)");

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
         QueT_DumpStr("Stopping dependencies.");

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
	   printf("Wait for stopped(QueT_StopService_)\n");

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


DWORD QueT_StartService_( SC_HANDLE hSCM, SC_HANDLE hService, DWORD dwTimeout ) 
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
	   QueT_DumpStr("Stop_pending(StartService_)\n");

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
    //   QueT_DumpStr("Service started(wait for running).");
    //else
    //   QueT_DisplayError( TEXT("StartService()"), dwError );


	// Wait for the service to start
	while ( ss.dwCurrentState != SERVICE_RUNNING ) {
	   QueT_DumpStr("Wait for running(StartService_)");

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






void QueT_StopKnowNowServer()
{

	SC_HANDLE hSCM;
	SC_HANDLE hService;
	DWORD     dwError;

	QueT_DumpStr("Stoping Service.");

   __try {

      // Open the SCM database
      hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT );
      if ( !hSCM ) {
         QueT_DisplayError( TEXT("OpenSCManager()"), GetLastError() );
         __leave;
      }

      // Open the specified service
	  char *SERVICE_NAME = "KnowNow Notification Server";
      hService = OpenService( hSCM
		  , SERVICE_NAME
		  , SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS );
      if ( !hService ) {
         QueT_DisplayError( TEXT("OpenService()"), GetLastError() );
         __leave;
      }

      // Try to stop the service, specifying a 30 second timeout
      dwError = QueT_StopService_( hSCM, hService, TRUE, 30000 ) ;
      if ( dwError == ERROR_SUCCESS )
	  {
         QueT_DumpStr("Service stopped.");
	  }
      else
         QueT_DisplayError( TEXT("StopService()"), dwError );

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
void QueT_StartKnowNowServer()
{

	SC_HANDLE hSCM;
	SC_HANDLE hService;
	DWORD     dwError;

	QueT_DumpStr("Starting Service.");

   __try {

      // Open the SCM database
      hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT );
      if ( !hSCM ) {
         QueT_DisplayError( TEXT("OpenSCManager()"), GetLastError() );
         __leave;
      }

      // Open the specified service
	  char *SERVICE_NAME = "KnowNow Notification Server";
      hService = OpenService( hSCM
		  , SERVICE_NAME
		  , SERVICE_ALL_ACCESS );
      if ( !hService ) {
         QueT_DisplayError( TEXT("OpenService()"), GetLastError() );
         __leave;
      }

      // Try to stop the service, specifying a 30 second timeout
      dwError = QueT_StartService_( hSCM, hService, 30000 ) ;
      if ( dwError == ERROR_SUCCESS )
	  {
         QueT_DumpStr("Service started.");
	  }
      else
         QueT_DisplayError( TEXT("StartService()"), dwError );

   } __finally {

      if ( hService )
         CloseServiceHandle( hService );

      if ( hSCM )
         CloseServiceHandle( hSCM );
   }

}



///////////////////////////////////////////////////////////////////////////////
// Tests //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


/**
 *
 */
void QueueTest::testStopStartServer()
{
	QueT_DumpInit();

	// Make sure started.
	QueT_StartKnowNowServer();

	QueT_StopKnowNowServer();

	QueT_StartKnowNowServer();
}


/**
 * Tests bug967
 */
void QueueTest::testPubSub()
{

	QueT_DumpInit();
	QueT_ResetCounters();
	QueT_ResetPubPayloads();
	QueT_ResetSubPayloads();


	// Make sure started.
	QueT_StartKnowNowServer();

	tstring serverUrl = "http://localhost:8000/kn";
	wstring topic = L"/what/Tests/functional/QueueT/testPubSub";


	// Start the subscriber.
	Connector subConn;
	QueT_SubListener subListener;
	QueT_SubRequestStatusHandler subReqSH;
	ITransport::Parameters subITParams;
	wstring rid = L"";

    // This is only necessary to get subConnSH_OnConnStatus
	QueT_SubConnStatusHandler subConnSH;
	subConn.AddConnectionStatusHandler(&subConnSH);

	subITParams.m_ServerUrl = serverUrl;
   	if (!subConn.Open(subITParams)){
		CPPUNIT_FAIL("Open(subITParams) failed.");
		return;
	}
	
	rid = subConn.Subscribe(topic, &subListener, Message(), &subReqSH);
	QueT_DumpStr(L"<sub>rid = " + rid);
	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
		return;
	}
	Sleep(100); // Avoid race conditions.



	// Start the publisher.
	Connector pubConn;
	QueT_PubRequestStatusHandler pubReqSH;
	ITransport::Parameters pubITParams;

    // This is only necessary to get pubConnSH_OnConnStatus
	QueT_PubConnStatusHandler pubConnSH;
	pubConn.AddConnectionStatusHandler(&pubConnSH);

	pubITParams.m_ServerUrl = serverUrl;
   	if (!pubConn.Open(pubITParams)){
		CPPUNIT_FAIL("Open(pubITParams) failed.");
		return;
	}

	// Clear the server queue, we don't want item from any previous activeties.
	CPPUNIT_ASSERT_MESSAGE("Queue could not be Cleared.",pubConn.Clear());

	if(pubConn.HasItems()) QueT_DumpStr("Queue should be empty, server queue has been cleared.");
//	CPPUNIT_ASSERT_MESSAGE("ERROR: Queue should be empty, server has been clesred.",pubConn.HasItems());

	// Turns the queueing on 
	pubConn.SetQueueing(true);
	CPPUNIT_ASSERT_MESSAGE("ERROR: Queueing must be on for this test(publish).",pubConn.GetQueueing());




	int i = 0;
	// Publish 5 messages.
	for( ; i < 5; i++)
	{
		Message pubMsg;

		char iBuf[12];
		itoa(i,iBuf,10);
		string payload = (string)"PAYLOAD_" + iBuf;

		pubMsg.Set("do_method", "notify");
		pubMsg.Set(L"kn_to", topic);
		pubMsg.Set("kn_response_format", "simple");
		pubMsg.Set("kn_expires", "+300");//5 minutes
		pubMsg.Set("kn_payload", payload);
		QueT_DumpStr("<pub>:",i);
		pubConn.Publish(pubMsg, &pubReqSH);
	}

	if(pubConn.HasItems()) QueT_DumpStr("ERROR: Queue should be empty (after pubs), server has been up.");
//	CPPUNIT_ASSERT_MESSAGE("Queue should be empty (after pubs), server has been up.",pubConn.HasItems());

	// Sleep a tad and then stop the server.
	Sleep(500);
	QueT_StopKnowNowServer();

	// Check that all pubs and subs occurred.
	 QueT_DumpCounters();
//	//CPPUNIT_ASSERT(queT_count_pubConnSH_OnConnStatus==?);// 1st Publish(), ??? depends on server up/down
//	CPPUNIT_ASSERT(queT_count_pubReqSH_OnStatus     ==0);
//	CPPUNIT_ASSERT(queT_count_pubReqSH_OnError      ==0);
//	CPPUNIT_ASSERT(queT_count_pubReqSH_OnSuccess    ==5);// Publish() 5 updates
//	CPPUNIT_ASSERT(queT_count_subListener_OnUpdate  ==5);// 5 updates
//	//CPPUNIT_ASSERT(queT_count_subConnSH_OnConnStatus==?);// 1st Subscribe(), ??? depends on server up/down
//	CPPUNIT_ASSERT(queT_count_subReqSH_OnStatus     ==0);
//	CPPUNIT_ASSERT(queT_count_subReqSH_OnError      ==0);
//	CPPUNIT_ASSERT(queT_count_subReqSH_OnSuccess    ==1);// Subscribe()


	// Publish 5 more messages with the server down, they should be Queued.
	for( ; i < 10; i++)
	{
		Message pubMsg;

		char iBuf[12];
		itoa(i,iBuf,10);
		string payload = (string)"PAYLOAD_" + iBuf;

		pubMsg.Set("do_method", "notify");
		pubMsg.Set(L"kn_to", topic);
		pubMsg.Set("kn_response_format", "simple");
		pubMsg.Set("kn_expires", "+300");//5 minutes
		pubMsg.Set("kn_payload", payload);
		QueT_DumpStr("<pub>:",i);
		pubConn.Publish(pubMsg, &pubReqSH);
	}

	if(!pubConn.HasItems()) QueT_DumpStr("ERROR: Queue should have messages, server is down.");
//	CPPUNIT_ASSERT_MESSAGE("Queue should have messages, server is down.",!pubConn.HasItems());


	const wstring queueFile = L"queue.txt";
	// delete any existing queue file
	_wunlink(queueFile.c_str());

	// Write the queue for a temporary visual test or debugging
	QueT_DumpStr(L"<pub>:SaveQueue(" + queueFile + L")");
	bool b = pubConn.SaveQueue(queueFile);
	CPPUNIT_ASSERT_MESSAGE("Could not write queue to file.", b);

	// Restart the server.
	// The publisher should still be valid.
	// The subscriber must be restarted.
	Sleep(500);
	QueT_StartKnowNowServer();



#if 0
	// Restart the subscriber.
	// Start the subscriber.
	Connector subConn2;
	QueT_SubListener subListener2;
	QueT_SubRequestStatusHandler subReqSH2;
	ITransport::Parameters subITParams2;

    // This is only necessary to get subConnSH_OnConnStatus
	QueT_SubConnStatusHandler subConnSH2;
	subConn2.AddConnectionStatusHandler(&subConnSH2);

	subITParams2.m_ServerUrl = serverUrl;
   	if (!subConn2.Open(subITParams2)){
		CPPUNIT_FAIL("Open(subITParams2) failed.");
		return;
	}
	
	rid = subConn2.Subscribe(topic, &subListener2, Message(), &subReqSH2);
	 QueT_DumpStr(L"<sub>rid = " + rid);
	if (rid.length() == 0)
	{
		CPPUNIT_FAIL("Failed to subscribe.");
		return;
	}
	Sleep(100); // Avoid race conditions.

#endif

	b = pubConn.LoadQueue(queueFile);

	// Flush the server queue, we don't want the test to have to wait.
	QueT_DumpStr("<pub>Flush()");

	b = pubConn.Flush();
	CPPUNIT_ASSERT_MESSAGE("Queue could not be flushed.", b);
	if(pubConn.HasItems()) QueT_DumpStr("ERROR: Queue should not have messages,  server was flushed.");
	//CPPUNIT_ASSERT_MESSAGE("Queue should be empty, server was flushed.",pubConn.HasItems());
	Sleep(5000); // Avoid race conditions, This one is really necessary.

	// Check that all pubs and subs occurred.
	 QueT_DumpCounters();
//	//CPPUNIT_ASSERT(queT_count_pubConnSH_OnConnStatus==?);// 1st Publish(), ??? depends on server up/down
//	CPPUNIT_ASSERT(queT_count_pubReqSH_OnStatus     ==0);
//	CPPUNIT_ASSERT(queT_count_pubReqSH_OnError      ==5);// Publish() 5 updates with server down.
//	CPPUNIT_ASSERT(queT_count_pubReqSH_OnSuccess    ==5);// Publish() 5 updates
//	CPPUNIT_ASSERT(queT_count_subListener_OnUpdate  ==10);// updates
//	//CPPUNIT_ASSERT(queT_count_subConnSH_OnConnStatus==?);// 1st Subscribe(), ??? depends on server up/down
//	CPPUNIT_ASSERT(queT_count_subReqSH_OnStatus     ==0);
//	CPPUNIT_ASSERT(queT_count_subReqSH_OnError      ==0);
//	CPPUNIT_ASSERT(queT_count_subReqSH_OnSuccess    ==2);// Subscribe(),Unsubscribe()
	 if(queT_count_subListener_OnUpdate  !=10 )
		 QueT_DumpStr("ERROR: queT_count_subListener_OnUpdate  !=10, ==",queT_count_subListener_OnUpdate);



	// Publish 5 more messages with the server back up.
	for( ; i < 15; i++)
	{
		Message pubMsg;

		char iBuf[12];
		itoa(i,iBuf,10);
		string payload = (string)"PAYLOAD_" + iBuf;

		pubMsg.Set("do_method", "notify");
		pubMsg.Set(L"kn_to", topic);
		pubMsg.Set("kn_response_format", "simple");
		pubMsg.Set("kn_expires", "+300");//5 minutes
		pubMsg.Set("kn_payload", payload);
		QueT_DumpStr("<pub>:",i);
		pubConn.Publish(pubMsg, &pubReqSH);
	}

	Sleep(3000);

	// Check that sub got all the pub payloads.
	QueT_DumpPubPayloads();
	QueT_DumpSubPayloads();

	// Check that all pubs and subs occurred.
	 QueT_DumpCounters();
//	//CPPUNIT_ASSERT(queT_count_pubConnSH_OnConnStatus==?);// 1st Publish(), ??? depends on server up/down
//	CPPUNIT_ASSERT(queT_count_pubReqSH_OnStatus     ==0);
//	CPPUNIT_ASSERT(queT_count_pubReqSH_OnError      ==5);// Publish() 5 updates with server down.
//	CPPUNIT_ASSERT(queT_count_pubReqSH_OnSuccess    ==10);// Publish() 5 updates
//	CPPUNIT_ASSERT(queT_count_subListener_OnUpdate  ==15);// updates
//	//CPPUNIT_ASSERT(queT_count_subConnSH_OnConnStatus==?);// 1st Subscribe(), ??? depends on server up/down
//	CPPUNIT_ASSERT(queT_count_subReqSH_OnStatus     ==0);
//	CPPUNIT_ASSERT(queT_count_subReqSH_OnError      ==0);
//	CPPUNIT_ASSERT(queT_count_subReqSH_OnSuccess    ==2);// Subscribe(),Unsubscribe()
	 if(queT_count_subListener_OnUpdate  !=15 )
		 QueT_DumpStr("ERROR: queT_count_subListener_OnUpdate  !=15, ==",queT_count_subListener_OnUpdate);

}

/**
 * If any tests above failed the server msy still be down. Restart it for any following test suites.
 */
void QueueTest::testStartServer()
{
	QueT_DumpInit();

	// Make sure started.
	QueT_StartKnowNowServer();

}
