// RandMT.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <LibKN\Interlocked.h>
#include <LibKN\CS.h>
#include <process.h>

volatile bool g_Quit = false;
CWhenZero<int> g_NumThreads = 0;
CWhenZero<int> g_NumIters = 0;
CCriticalSection g_Screen;

enum 
{
	Program,
	NumThreads,
	NumIters,
	Expected
};

BOOL WINAPI CtrlHandler(DWORD ctrlType)
{
	printf("Exiting...\n");
	g_Quit = true;
	return TRUE;
}

void WorkerThread(void* arg)
{
	int n = reinterpret_cast<int>(arg);

	srand(GetTickCount() + GetCurrentThreadId());
	g_NumThreads++;

	
	{
		for (int i = 0; i < g_NumIters; i++)
		{
			LockImpl<CCriticalSection> lock(&g_Screen);
			printf("%d, %d: %d\n", GetCurrentThreadId(), i, rand());
		}
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

	g_NumIters = atoi(argv[NumIters]);

	printf("Starting...\n");

	{
		int nThreads = atoi(argv[NumThreads]);

		printf("%d threads.\n", nThreads);

		for (int i = 0; i < nThreads; ++i)
		{
			_beginthread(WorkerThread, 0, reinterpret_cast<void*>(i));
		}

		Sleep(1000);

		printf("Waiting...\n");
		WaitForSingleObject(g_NumThreads, INFINITE);
	}

	printf("End of program.\n");

	return 0;
}

