// SubTestMT.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <LibKN\Connector.h>
#include <LibKN\StrUtil.h>
#include <LibKN\Interlocked.h>
#include <LibKN\CS.h>
#include <process.h>

volatile bool g_Quit = false;
CWhenZero<int> g_NumThreads = 0;
Connector g_Connector;
CCriticalSection g_Screen;
CInterlockedScalar<int> g_NumRequests = 0;
CInterlockedScalar<int> g_NumEvents = 0;

const char* Prefix = "/MT/Foo";

enum 
{
	Program,
	Server,
	NumThreads,
	Expected
};


class MyListener : public IListener
{
public:
	MyListener() :
		m_TID(GetCurrentThreadId())
	{
	}

	void OnUpdate(const Message& msg)
	{
		LockImpl<CCriticalSection> lock(&g_Screen);

		printf("Got an event (%d, %d) %d ============\n", m_TID, GetCurrentThreadId(), g_NumEvents++);
		for (Message::Container::const_iterator it = msg.GetContainer().begin(); it != msg.GetContainer().end(); ++it)
		{
			printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
		}
		printf("\n");
	}

	int m_TID;
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
		m_TID(GetCurrentThreadId())
	{
	}

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

	int n = reinterpret_cast<int>(arg);
	
	{
		LockImpl<CCriticalSection> lock(&g_Screen);
		printf("Entering (%d, %d)\n", n, GetCurrentThreadId());
	}

	char buffer[1024];
	sprintf(buffer, "%s/%d", Prefix, n);

	wstring topic = ConvertToWide(buffer);
	
	MyListener mListener;
	MyRequestHandler mReqH;

	{
		LockImpl<CCriticalSection> lock(&g_Screen);
		printf("Topic %S\n", topic.c_str());
	}

	wstring rid = g_Connector.Subscribe(topic, &mListener, &mReqH);

	if (rid.length() == 0)
	{
		LockImpl<CCriticalSection> lock(&g_Screen);
		printf("Failed to subscribe\n");
	}
	else
	{
		{
			LockImpl<CCriticalSection> lock(&g_Screen);
			printf("rid = %S\n", rid.c_str());
		}

		while (!g_Quit)
		{
			Sleep(10);
		}

		if (!g_Connector.Unsubscribe(rid, &mReqH))
		{
			{
				LockImpl<CCriticalSection> lock(&g_Screen);
				printf("failed to unsubscribe\n");
			}
		}
		else
		{
			//LockImpl<CCriticalSection> lock(&g_Screen);
			//printf("Successfully unsubscribed\n");
		}
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

	ITransport::Parameters p;

	MyConnStatusHandler mConnSH;
	p.m_ServerUrl = argv[Server];

	g_Connector.AddConnectionStatusHandler(&mConnSH);
	if (g_Connector.Open(p))
	{
		int nThreads = atoi(argv[NumThreads]);

		printf("%d threads.\n", nThreads);

		for (int i = 0; i < nThreads; ++i)
		{
			_beginthread(WorkerThread, 0, reinterpret_cast<void*>(i));
		}

		Sleep(1000);

		if (g_NumThreads > 0)
		{
			printf("Waiting...\n");
			WaitForSingleObject(g_NumThreads, INFINITE);
		}
		else
		{
			printf("Skipped?\n");
		}

		g_Connector.Close();
	}
	else
	{
		printf("Failed to find server at %s\n", argv[Server]);
	}

	g_Connector.RemoveConnectionStatusHandler(&mConnSH);

	printf("End of program.\n");

	return 0;
}

