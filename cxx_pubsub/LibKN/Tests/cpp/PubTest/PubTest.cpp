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

	void OnSuccess(const Message& msg)
	{
		printf("%d", m_Count % 10);
		m_Count++;
	}

	void OnError(const Message& msg)
	{
		printf("Failed\n");
		for (Message::Container::const_iterator it = msg.GetContainer().begin(); it != msg.GetContainer().end(); ++it)
		{
			printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
		}
		printf("\n");
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
		
		char payload[1024];
		ZeroMemory(payload, sizeof payload);
		memset(payload, 'a', sizeof payload - 1);

		Message m;
		m.Set("do_method", "notify");
		m.Set("kn_to", argv[Topic]);
//		m.Set("kn_payload", "Hello");
		m.Set("kn_payload", payload);
		m.Set("nickname", "PubTest");
		m.Set("kn_response_format", "simple");

		DWORD st = GetTickCount();

		for (int i = 0; i < numPub && !g_Quit; i++)
		{
			if (!c.Publish(m, &myRequestStatusHandler))
			{
				printf("Failed %d\n", i);
			}
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
