// PubTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <LibKN\Connector.h>

bool g_Quit = false;

enum 
{
	Program,
	Server,
	Topic,
	NumOfPubs,
	Expected
};

BOOL WINAPI CtrlHandler(DWORD ctrlType)
{
	printf("Exiting...\n");
	g_Quit = true;
	return TRUE;
}

class MyRequestStatusHandler : public IRequestStatusHandler
{
public:
	MyRequestStatusHandler()
	{
		m_Count = 0;
	}

	~MyRequestStatusHandler()
	{
	}

	void OnStatus(const Message& msg)
	{
		IRequestStatusHandler::OnStatus(msg);
		printf("%d", m_Count % 10);
		m_Count++;
	}

	DWORD m_Count;
};

int main(int argc, char* argv[])
{
	if (argc != Expected)
	{
		printf("Expecting %d args. Got %d instead\n", Expected, argc);
		return -1;
	}

	SetConsoleCtrlHandler(CtrlHandler, TRUE);

	Connector c;
	MyRequestStatusHandler myRequestStatusHandler;

	ITransport::Parameters p;
	p.m_ServerUrl = argv[Server];
	p.m_ShowUI = true;

	if (c.Open(p))
	{
		printf("Opened connection\n");

		int numPub = atoi(argv[NumOfPubs]);

		Message m;
		m.Set("do_method", "notify");
		m.Set("kn_to", argv[Topic]);
		m.Set("kn_payload", "Hello");
		m.Set("nickname", "PubTest");
		m.Set("kn_response_format", "simple");

		DWORD st = GetTickCount();

		for (int i = 0; i < numPub && !g_Quit; i++)
		{
			c.Publish(m, &myRequestStatusHandler);
		}
		
		DWORD ed = GetTickCount();
		DWORD duration = ed - st;

		if (duration == 0)
			duration = 1;

		printf("\nTime is %d milliseconds\n", duration);

		c.Close();
	}
	else
	{
		printf("Failed to find server at %s\n", argv[Server]);
	}

	return 0;
}
