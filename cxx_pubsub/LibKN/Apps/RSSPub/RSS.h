#if !defined(RSS_H)
#define RSS_H

#include <LibKN\Connector.h>

struct RSSFeed
{
	int i;
	_bstr_t link;
	_bstr_t title;
	_bstr_t description;
};

IXMLDOMNodePtr GetAttributeNode(IXMLDOMNodePtr node, const _bstr_t& name);
_bstr_t GetAttributeValue(IXMLDOMNodePtr node, const _bstr_t& name);
_bstr_t GetPathValue(IXMLDOMNodePtr node, const _bstr_t& path);
void ProcessRSS(RSSFeed* f);
void FillRSSFeed(IXMLDOMNodePtr rss, RSSFeed* f);

extern HANDLE g_Semaphore;
extern Connector g_Conn;

#endif

