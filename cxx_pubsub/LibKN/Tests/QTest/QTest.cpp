// QTest.cpp : Defines the entry point for the console application.
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

class MyConnStatusHandler : public IConnectionStatusHandler
{
public:
	void OnConnectionStatus(const Message& msg)
	{
		printf("---- Connection Status\n");
		for (Message::Container::const_iterator it = msg.GetContainer().begin(); it != msg.GetContainer().end(); ++it)
		{
			printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
		}
		printf("\n");
	}
};

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
	MyConnStatusHandler mConnSH;

	ITransport::Parameters p;
	p.m_ServerUrl = argv[Server];

	c.AddConnectionStatusHandler(&mConnSH);

	if (c.Open(p))
	{
		printf("Opened connection\n");

		int numPub = atoi(argv[NumOfPubs]);

		Message m;
		m.Set("do_method", "notify");
		m.Set("kn_to", argv[Topic]);
		m.Set("nickname", "PubTest");
		m.Set("kn_response_format", "simple");

		char buffer[100];

		for (int i = 0; i < numPub && !g_Quit; i++)
		{
			sprintf(buffer, "Hello %d", i);
			m.Set("kn_payload", buffer);
			if (!c.Publish(m, &myRequestStatusHandler))
				printf("Failed %d", i);
			Sleep(300);
		}

		printf("Published all\n");

		do
		{
			Sleep(20);
		} while (!g_Quit);
		
		c.Close();
	}
	else
	{
		printf("Failed to find server at %s\n", argv[Server]);
	}
	
	c.RemoveConnectionStatusHandler(&mConnSH);

	return 0;
}
