// RSSPub.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "RSS.h"
#include <LibKN\Connector.h>

const int NumSemaphores = 10;
HANDLE g_Semaphore = 0;

Connector g_Conn;

void DumpFeeds(IXMLDOMDocumentPtr ptr)
{
	if (g_Semaphore == 0)
	{
		printf("Failed to create semaphore\n");
		return;
	}

	IXMLDOMNodeListPtr nodes = ptr->selectNodes(_bstr_t("/rss/channel/item"));

	if (nodes == 0)
		return;

	printf("%d items\n", nodes->Getlength());

	int nItems = nodes->Getlength();
//	int nItems = 5;

	for (int i = 0; i < nItems; i++)
	{
		WaitForSingleObject(g_Semaphore, INFINITE);
		printf("Starting %d of %d\n", i, nItems);
		IXMLDOMNodePtr node = nodes->Getitem(i);

		RSSFeed* f = new RSSFeed();
		f->i = i;
		FillRSSFeed(node, f);
		ProcessRSS(f);
	}
}

enum TArgs
{
	ProgramPath = 0,
	XmlFile,
	Router,
	Expected,
};

int main(int argc, char* argv[])
{
	if (argc != Expected)
	{
		printf("Expecteding %d args, got %d instead\n", Expected, argc);
		printf("Usage: %s xmlfile router\n", argv[0]);
		return -1;
	}

	g_Semaphore = CreateSemaphore(0, NumSemaphores, NumSemaphores, 0);
	CoInitializeEx(0, COINIT_APARTMENTTHREADED);

	//Transport& t = g_Conn.GetTransport();
	ITransport::Parameters p;
	p.m_ServerUrl = argv[Router];
	bool b = g_Conn.Open(p);

	int att = 0;

	while (true)
	{
		try
		{
			IXMLDOMDocumentPtr ptr("Microsoft.FreeThreadedXMLDOM");
	
			ptr->async = VARIANT_FALSE;
			ptr->validateOnParse = VARIANT_FALSE;
	
	#if 1
			printf("Using file %s\n", argv[XmlFile]);
			_bstr_t xmlStr = argv[XmlFile];
			_variant_t v(xmlStr);
			ptr->load(v);
	#else
			ptr->load("http://www.syndic8.com/genfeed.php?Format=rss");
	#endif
	
			DumpFeeds(ptr);
		}
		catch (...)
		{
		}
	}

	if (b)
	{
		g_Conn.Close();
	}

	CoUninitialize();
	CloseHandle(g_Semaphore);
	g_Semaphore = 0;
	return 0;
}

