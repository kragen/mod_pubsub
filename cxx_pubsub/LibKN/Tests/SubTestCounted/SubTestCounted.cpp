// SubTestCounted.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <LibKN\Connector.h>
#include <LibKN\StrUtil.h>
#include <list>

volatile bool g_Quit = false;

enum 
{
	Program,
	Server,
	Topic,
	NumOfSubs,
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
#if 0
		printf(".");
#else
		wstring v;
		if (msg.Get("status_code", v))
		{
			if (v != L"200")
			{
				printf("**** Request Status\n");
				for (Message::Container::const_iterator it = msg.GetContainer().begin(); it != msg.GetContainer().end(); ++it)
				{
					printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
				}
				printf("\n");
			}
			else
				printf(".");
		}
		else
		{
			printf("Cannot get status_code!\n");
			for (Message::Container::const_iterator it = msg.GetContainer().begin(); it != msg.GetContainer().end(); ++it)
			{
				printf("\t%S = %S\n", (*it).first.c_str(), (*it).second.c_str());
			}
			printf("\n");
		}

#endif
	}
};


BOOL WINAPI CtrlHandler(DWORD ctrlType)
{
	printf("Exiting...\n");
	g_Quit = true;
	return TRUE;
}

void DumpDiff(int numSub, char* msg, DWORD st, DWORD ed)
{
	printf("%d %s. Time elapsed = %d msec\n", numSub, msg, ed - st);
	printf("Avg %f subs/sec\n", (numSub * 1000.0 / (ed - st)));
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
	
	c.AddConnectionStatusHandler(&mConnSH);

	ITransport::Parameters p;

	p.m_ServerUrl = argv[Server];
	p.m_ShowUI = true;
	p.m_ParentHwnd = GetDesktopWindow();

	if (c.Open(p))
	{
		std::list<wstring> rids;

		DWORD st = GetTickCount();
		int numSub = atoi(argv[NumOfSubs]);

		if (numSub == 0)
		{
			numSub = 10;
		}

		wstring t = ConvertToWide(argv[Topic]);
		t += L"/";

		for (int i = 0; i < numSub && !g_Quit; i++)
		{
			wchar_t buffer[32];
			swprintf(buffer, L"%d", i);

			wstring topic = t;
			topic += buffer;

			printf("%S", topic.c_str());

			wstring rid = c.Subscribe(topic, &mListener, &mReqH);

			if (rid.length() > 0)
			{
				printf("+");
				rids.push_back(rid);
			}
			else
			{
				printf("%d is empty\n", i);
			}
		}

		DWORD ed = GetTickCount();

		int nItems = rids.size();
		DumpDiff(nItems, "subs", st, ed);

		while (!g_Quit)
		{
			Sleep(10);
		}

		g_Quit = false;

		st = GetTickCount();

		while (!rids.empty() && !g_Quit)
		{
			wstring rid = rids.back();
			rids.pop_back();
			if (c.Unsubscribe(rid, &mReqH))
			{
				printf("-");
			}
			else
			{
				printf("Failed %S\n", rid.c_str());
			}
		}

		ed = GetTickCount();

		if (nItems > 0)
		{
			DumpDiff(nItems, "unsubs", st, ed);
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
