#include "stdafx.h"
#include "RSS.h"
#include <LibKN\Message.h>
#include <process.h>
#include <ctype.h>
#include <string>

IXMLDOMNodePtr GetAttributeNode(IXMLDOMNodePtr node, const _bstr_t& name)
{
	if (node == NULL)
		return NULL;

	IXMLDOMNamedNodeMapPtr attributes = node->Getattributes();
	if (attributes == NULL)
		return NULL;

	IXMLDOMNodePtr att = attributes->getNamedItem(name);
	return att;
}

_bstr_t GetAttributeValue(IXMLDOMNodePtr node, const _bstr_t& name)
{
	IXMLDOMNodePtr attr = GetAttributeNode(node, name);

	if (attr == NULL)
		return _bstr_t();

	return attr->GetnodeValue();
}

_bstr_t GetPathValue(IXMLDOMNodePtr node, const _bstr_t& path)
{
	if (node == NULL)
		return _bstr_t();
	
	IXMLDOMNodePtr n = node->selectSingleNode(path);
	
	if (n == NULL)
		return _bstr_t();

	return n->Gettext();
}

#if 0
_bstr_t HttpGetUrl(const _bstr_t& url)
{
	_bstr_t retval;

	HINTERNET hSession = InternetOpen(_T("HttpGet"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

	if (hSession)
	{
		bool isSsl = false;
		//const char* u = static_cast<const char*>(url);

		HINTERNET hFile = InternetOpenUrl(hSession, url, NULL, 0, 0, 0);
		
		if (hFile)
		{
			DWORD bytesAvailable = 0;
			
			if (InternetQueryDataAvailable(hFile, &bytesAvailable, 0, 0))
			{
				const int BUFFLEN = 1024;
				TCHAR buffer[BUFFLEN];
				DWORD dwRead;
	
				while (InternetReadFile(hFile, buffer, BUFFLEN - sizeof(TCHAR), &dwRead))
				{
					if (dwRead == 0)
						break;
	
					buffer[dwRead / sizeof(TCHAR)] = _T('\0');
					retval += buffer;
				}
	
				InternetCloseHandle(hFile);
			}
		}

		InternetCloseHandle(hSession);
	}

	return retval;
}
#endif

_bstr_t ConvertLinkToId(const _bstr_t& link)
{
	_bstr_t retVal;
	const char* l = (const char*)link;
	std::string s;

	for (int i = 0; i < strlen(l); i++)
	{
		char c = l[i];
		if (isalnum(c))
		{
			s += c;
		}
	}
	retVal = s.c_str();
	return retVal;
}


void ProcessRSSItem(RSSFeed* f)
{
	CoInitializeEx(0, COINIT_APARTMENTTHREADED);

	try
	{
		IXMLDOMDocumentPtr ptr("Microsoft.FreeThreadedXMLDOM");
		
		ptr->async = VARIANT_FALSE;
		ptr->validateOnParse = VARIANT_FALSE;
		ptr->load(f->link);

		if (ptr != 0)
		{
			IXMLDOMNodeListPtr nodes = ptr->selectNodes(_bstr_t("/rss/channel/item"));

			if (nodes != 0)
			{
				int nItems = nodes->Getlength();
//				printf("%d headlines\n", nItems);
	
				for (int i = 0; i < nItems; i++)
				{
					_bstr_t xml = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n";
					xml += "<rss version=\"0.92\">\n";
					xml += "<channel>\n";
					xml += "<item>\n";

					IXMLDOMNodePtr node = nodes->Getitem(i);
					_bstr_t title = GetPathValue(node, _bstr_t("title"));
					xml += "<title>";
					xml += title;
					xml += "</title>\n";

					_bstr_t link = GetPathValue(node, _bstr_t("link"));
					xml += "<link>";
					xml += link;
					xml += "</link>\n";

					_bstr_t description = GetPathValue(node, _bstr_t("description"));
					xml += "<description>";
					xml += description;
					xml += "</description>\n";

					xml += "</item>\n";
					xml += "</channel>\n";
					xml += "</rss>\n";

					Message m;
					m.Set("do_method", "notify");
					m.Set("kn_to", "/headlines");
					m.Set("title", (const char*)title);
					m.Set("link", (const char*)link);
					m.Set("description", (const char*)description);
					m.Set("kn_payload", (const char*)xml);
					m.Set("kn_id", (const char*)ConvertLinkToId(link));
					m.Set("source_title", (const char*)f->title);
					m.Set("source_link", (const char*)f->link);
					m.Set("source_description", (const char*)f->description);

					m.Set("displayname", "RSS");
					m.Set("rss_source", (const char*)f->title);
					m.Set("rss_link", (const char*)link);
					m.Set("rss_title", (const char*)title);
					m.Set("rss_description", (const char*)description);

					g_Conn.Publish(m, 0);

//					printf("%d: %s <%s>\n", i, (const char*)title, (const char*)link);
				}
			}
		}
	}
	catch (...)
	{
	}

	CoUninitialize();
}

void ProcessRSSItemImpl(void* arg)
{
	RSSFeed* f = (RSSFeed*)arg;

	_bstr_t title = f->title;
	_bstr_t link = f->link;
	_bstr_t description = f->description;

//	printf("===============================\n");
//	printf("%s\n", static_cast<const char*>(title));
	ProcessRSSItem(f);

	printf("Ending %d\n", f->i);

	delete f;

	ReleaseSemaphore(g_Semaphore, 1, 0);
}

void ProcessRSS(RSSFeed* f)
{
	if (f == 0)
		return;

	_beginthread(ProcessRSSItemImpl, 0, (void*)f);
}

void FillRSSFeed(IXMLDOMNodePtr rss, RSSFeed* f)
{
	if (f == 0 || rss == 0)
		return;

	_bstr_t title = GetPathValue(rss, _bstr_t("title"));
	_bstr_t link = GetPathValue(rss, _bstr_t("link"));
	_bstr_t description = GetPathValue(rss, _bstr_t("description"));

	f->link = link;
	f->title = title;
	f->description = description;
}

