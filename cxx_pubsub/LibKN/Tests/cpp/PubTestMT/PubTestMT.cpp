// PubTestMT.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <LibKN\Connector.h>
#include <LibKN\StrUtil.h>
#include <LibKN\Interlocked.h>
#include <LibKN\CS.h>
#include <process.h>

volatile bool g_Quit = false;
CWhenZero<int> g_NumThreads = 0;
Connector* g_Connector;
CCriticalSection g_Screen;
CInterlockedScalar<int> g_NumRequests = 0;
CInterlockedScalar<int> g_NumEvents = 0;

const char* Prefix = "/MT/Foo";

enum 
{
	Program,
	Server,
	NumThreads,
	NumEvents,
	Expected
};


class MyConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg)
	{
		LockImpl<CCriticalSection> lock(&g_Screen);

		printf("---- Connection Status\n");
		for (Message::Container::const_iterator it = msg.GetContainer().begin(); it != msg.GetContainer().end(); ++it)
		{
			printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
		}
		printf("\n");
	}
};

class MyRequestHandler : public IRequestStatusHandler
{
public:
	MyRequestHandler() :
		m_TID(GetCurrentThreadId()),
		m_Count(0)
	{
	}

	void OnSuccess(const Message& msg)
	{
		LockImpl<CCriticalSection> lock(&g_Screen);
		printf("%d", m_Count % 10);
		m_Count++;
	}

	void OnError(const Message& msg)
	{
		LockImpl<CCriticalSection> lock(&g_Screen);
		printf("Failed\n");
		for (Message::Container::const_iterator it = msg.GetContainer().begin(); it != msg.GetContainer().end(); ++it)
		{
			printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
		}
		printf("\n");
	}

#if 0
	void OnStatus(const Message& msg)
	{
		LockImpl<CCriticalSection> lock(&g_Screen);
		
		printf("**** Request Status (%d, %d)\n", m_TID, GetCurrentThreadId());
		printf("Request %d\n", g_NumRequests++);
		for (Message::Container::const_iterator it = msg.GetContainer().begin(); it != msg.GetContainer().end(); ++it)
		{
			printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
		}
		printf("\n");
	}
#endif

	int m_Count;
	int m_TID;
};

BOOL WINAPI CtrlHandler(DWORD ctrlType)
{
	printf("Exiting...\n");
	g_Quit = true;
	return TRUE;
}

void WorkerThread(void* arg)
{
	g_NumThreads++;

	srand(GetCurrentThreadId());

	int n = reinterpret_cast<int>(arg);
	
	{
		LockImpl<CCriticalSection> lock(&g_Screen);
		printf("Entering (%d, %d)\n", n, GetCurrentThreadId());
	}

	char buffer[1024];
	sprintf(buffer, "%s/%d", Prefix, n);

	wstring topic = ConvertToWide(buffer);

	{
		LockImpl<CCriticalSection> lock(&g_Screen);
		printf("Topic %s\n", buffer);
	}

	MyRequestHandler mReqH;

	Message m;
	m.Set("do_method", "notify");
	m.Set("kn_to", buffer);
	m.Set("nickname", "PubTestMT");
	m.Set("kn_response_format", "simple");
	char payload[1024];
	ZeroMemory(payload, sizeof payload);
	memset(payload, 'a', sizeof payload - 1);

	for (int i = 0; i < g_NumEvents; i++)
	{
//		sprintf(payload, "hello %d %d", n, i);
		m.Set("kn_payload", payload);
		bool b = g_Connector->Publish(m, &mReqH);
//		Sleep(rand() % 10);
	}

	{
		LockImpl<CCriticalSection> lock(&g_Screen);
		printf("Leaving (%d, %d)\n", n, GetCurrentThreadId());
	}

	g_NumThreads--;
}

int main(int argc, char* argv[])
{
	SetConsoleCtrlHandler(CtrlHandler, TRUE);

	if (argc != Expected)
	{
		printf("Expecting %d args. Got %d instead\n", Expected, argc);
		return -1;
	}

	printf("Starting...\n");

	g_Connector = new Connector();

	g_NumEvents = atoi(argv[NumEvents]);

	ITransport::Parameters p;
	p.m_ServerUrl = argv[Server];

	if (g_Connector->Open(p))
	{
		int nThreads = atoi(argv[NumThreads]);

		printf("%d threads.\n", nThreads);

		DWORD st = GetTickCount();
		
		for (int i = 0; i < nThreads; ++i)
		{
			_beginthread(WorkerThread, 0, reinterpret_cast<void*>(i));
		}

		Sleep(500);

		printf("Waiting...\n");
		WaitForSingleObject(g_NumThreads, INFINITE);

		DWORD et = GetTickCount();

		DWORD duration = et - st;

		if (duration == 0)
			duration = 1;

		printf("\nTime is %d milliseconds\n", duration);

		g_Connector->Close();
	}
	else
	{
		printf("Failed to find server at %s\n", argv[Server]);
	}

	printf("End of program.\n");

	delete g_Connector;
	g_Connector = 0;

	return 0;
}


