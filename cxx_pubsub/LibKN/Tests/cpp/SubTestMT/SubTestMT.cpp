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
	NumEvents,
	Expected
};


class MyListener : public IListener
{
public:
	MyListener(bool* quitThread) :
		m_QuitThread(quitThread),
		m_TID(GetCurrentThreadId()),
		m_NumEvents(0)
	{
	}

	void OnUpdate(const Message& msg)
	{
		LockImpl<CCriticalSection> lock(&g_Screen);

		if (g_NumEvents > 0)
		{
			m_NumEvents++;
			if (m_NumEvents >= g_NumEvents)
				if (m_QuitThread)
					*m_QuitThread = true;
		}

//		printf("Got an event (%d, %d) %d ============\n", m_TID, GetCurrentThreadId(), g_NumEvents++);
#if 0
		for (Message::Container::const_iterator it = msg.GetContainer().begin(); it != msg.GetContainer().end(); ++it)
		{
			printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
		}
		printf("\n");
#endif
		printf("[%d]", m_TID);
	}

	bool* m_QuitThread;
	int m_TID;
	int m_NumEvents;
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
	MyRequestHandler(int n) :
		m_TID(GetCurrentThreadId()),
		m_N(n)
	{
	}

	void OnSuccess(const Message& msg)
	{
		LockImpl<CCriticalSection> lock(&g_Screen);
		printf("Success %d\n", g_NumRequests++);
	}

	void OnError(const Message& msg)
	{
		LockImpl<CCriticalSection> lock(&g_Screen);
		printf("Error %d\n", m_N);
		
		printf("**** Request Status (%d, %d)\n", m_TID, GetCurrentThreadId());
		for (Message::Container::const_iterator it = msg.GetContainer().begin(); it != msg.GetContainer().end(); ++it)
		{
			printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
		}
		printf("\n");

	}

	int m_TID;
	int m_N;
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

	bool quitThread = false;
	
	MyListener mListener(&quitThread);
	MyRequestHandler mReqH(n);

	{
		LockImpl<CCriticalSection> lock(&g_Screen);
		printf("Topic %S\n", topic.c_str());
	}

	wstring rid = g_Connector.Subscribe(topic, &mListener, Message(), &mReqH);

	if (rid.length() == 0)
	{
		LockImpl<CCriticalSection> lock(&g_Screen);
		printf("Failed to subscribe\n");
	}
	else
	{
		{
			LockImpl<CCriticalSection> lock(&g_Screen);
			printf("%d: rid = %S\n", n, rid.c_str());
		}

		while (!g_Quit && !quitThread)
		{
			Sleep(10);
		}

		printf("%d: Unsubscribing %S\n", n, rid.c_str());

		if (!g_Connector.Unsubscribe(rid, &mReqH))
		{
			{
				LockImpl<CCriticalSection> lock(&g_Screen);
				printf("failed to unsubscribe %d\n", n);
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

	g_NumEvents = atoi(argv[NumEvents]);

	g_Connector.AddConnectionStatusHandler(&mConnSH);
	if (g_Connector.Open(p))
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

