// SubTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <LibKN\Connector.h>
#include <LibKN\StrUtil.h>

volatile bool g_Quit = false;

enum 
{
	Program,
	Server,
	Topic,
	Expected
};


class MyListener : public IListener
{
public:
	void OnUpdate(const Message& msg)
	{
		printf("Got an event ============\n");
		for (Message::Container::const_iterator it = msg.GetContainer().begin(); it != msg.GetContainer().end(); ++it)
		{
			printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
		}
		printf("\n");
	}
};

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

class MyRequestHandler : public IRequestStatusHandler
{
public:
	void OnStatus(const Message& msg)
	{
		printf("**** Request Status\n");
		for (Message::Container::const_iterator it = msg.GetContainer().begin(); it != msg.GetContainer().end(); ++it)
		{
			printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
		}
		printf("\n");
	}
};


BOOL WINAPI CtrlHandler(DWORD ctrlType)
{
	printf("Exiting...\n");
	g_Quit = true;
	return TRUE;
}

int main(int argc, char* argv[])
{
	if (argc != Expected)
	{
		printf("Expecting %d args. Got %d instead\n", Expected, argc);
		return -1;
	}

	SetConsoleCtrlHandler(CtrlHandler, TRUE);

	Connector c;
	MyListener mListener;
	MyConnStatusHandler mConnSH;
	MyRequestHandler mReqH;

	ITransport::Parameters p;

	p.m_ServerUrl = argv[Server];
	p.m_ShowUI = true;
	p.m_ParentHwnd = GetDesktopWindow();

	c.AddConnectionStatusHandler(&mConnSH);

	if (c.Open(p))
	{
		wstring topic = ConvertToWide(argv[Topic]);
		wstring rid = c.Subscribe(topic, &mListener, Message(), &mReqH);

		if (rid.length() == 0)
		{
			printf("Failed to subscribe\n");
		}
		else
		{
			printf("rid = %S\n", rid.c_str());
	
			while (!g_Quit)
			{
				Sleep(10);
			}

			printf("Unsubscribing\n");
	
			if (!c.Unsubscribe(rid, &mReqH))
				printf("failed to unsubscribe\n");
		}

		c.Close();
	}
	else
	{
		printf("Failed to find server at %s\n", argv[Server]);
	}

	c.RemoveConnectionStatusHandler(&mConnSH);

	return 0;
}
