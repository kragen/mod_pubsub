// <%@LANGUAGE = JScript %>
// <%

main(WScript.Arguments);

function alert(text)
{
	WScript.echo(text);
}

function loadXML(url)
{
	var dom = WScript.CreateObject("MSXML.DOMDocument");
	dom.async = false;
	dom.load(url);
	return dom;
}

function loadNews(url)
{
	var feed = loadXML(url);
	return feed.selectNodes("//item");
}

function forwardNews(topic,items)
{
	var poster = WScript.CreateObject("Microsoft.XMLHTTP");
	var item;
	var title;
	var description;
	var link;
	var body;

	var content;

	for (var i=0; i < items.length; i++)
	{
		item = items[i];
		
		title = item.selectSingleNode("title");
		if (title)
			title = title.text;

		description = item.selectSingleNode("description");
		if (description)
			description = description.text;			
		else
			description = "";

		link = item.selectSingleNode("link");
		if (link)
			link = link.text;
			
		if (title && link)
		{
			var reg;

			poster.open("POST","http://www.topiczero.com:8000/kn/",false);
			body = "do_method=notify"+
//			"&kn_expires=%2b36000"+
			"&type=news"+
			"&kn_to="+topic+
			"&title="+title+
			"&description="+description+
			"&link="+encodeURI(link);
			body = body.replace(/\s/g,"+");
		poster.setRequestHeader("Content-Type","application/x-www-form-urlencoded");		
		poster.setRequestHeader("Content-Length",body.length);		
			poster.send(body+"\n\n" );
			poster.responseText;
if (poster.status == '400')
{
//	alert(poster.status + " " +link);
//	alert(body);
}
		}
	}
		
}

function main(args)
{
	var news;


	/////
	// General
	// description=text
	news = loadNews("http://www.newsisfree.com/HPE/xml/feeds/42/1842.xml");
	forwardNews("/topiczero.com/news/",news);

	// description=text
	news = loadNews("http://www.wired.com/news_drop/netcenter/netcenter.rdf");
	forwardNews("/topiczero.com/news/",news);

	/////
	// Business
	/*
	// description=byline
	news = loadNews("http://www.moreover.com/cgi-local/page?index_e-commerce+rss");
	forwardNews("/dmoz.org/News/Breaking_News/Business_and_Economy/",news);
	*/
	
	// description=text
	news = loadNews("http://www.newsisfree.com/HPE/xml/feeds/78/1678.xml");
	forwardNews("/dmoz.org/News/Breaking_News/Business_and_Economy/",news);

	// description=text
	news = loadNews("http://www.newsisfree.com/HPE/xml/feeds/66/2766.xml");
	forwardNews("/dmoz.org/News/Breaking_News/Business_and_Economy/",news);

	/////
	// Internet

	// description=text
	// internetworld.com
	news = loadNews("http://headlines.internet.com/internetnews/top-news/news.rss");
	forwardNews("/dmoz.org/News/By_Subject/Information_Technology/Internet/",news);

	/* boring
	// description=text
	news = loadNews("http://www.w3.org/2000/08/w3c-synd/home.rss");
	forwardNews("/dmoz.org/News/By_Subject/Information_Technology/Internet/",news);

	// description=text
	// internetworld.com
	news = loadNews("http://www.newsisfree.com/HPE/xml/feeds/64/1164.xml");
	forwardNews("/dmoz.org/News/By_Subject/Information_Technology/Internet/",news);

	// description=text
	// internetnews.com E-Commerce
	news = loadNews("http://www.newsisfree.com/HPE/xml/feeds/27/127.xml");
	forwardNews("/dmoz.org/News/By_Subject/Information_Technology/Internet/",news);

	// description=text
	// internetweek
	news = loadNews("http://www.newsisfree.com/HPE/xml/feeds/98/1598.xml");
	forwardNews("/dmoz.org/News/By_Subject/Information_Technology/Internet/",news);
	*/

	/////
	// Technology
	// description=text
	news = loadNews("http://trainedmonkey.com/news/rss.php?s=31");
	forwardNews("/dmoz.org/News/By_Subject/Information_Technology/",news);

	// description=text
	news = loadNews("http://www.newsisfree.com/HPE/xml/feeds/72/1472.xml");
	forwardNews("/dmoz.org/News/By_Subject/Information_Technology/",news);

	// description=text
	news = loadNews("http://www.newsisfree.com/HPE/xml/feeds/16/1016.xml");
	forwardNews("/dmoz.org/News/By_Subject/Information_Technology/",news);

	/////
	// Society
	// description=text	
	news = loadNews("http://www.newsisfree.com/HPE/xml/feeds/33/1633.xml");
	forwardNews("/dmoz.org/Society/Future/",news);

	// description=text	
	news = loadNews("http://www.newsisfree.com/HPE/xml/feeds/14/914.xml");
	forwardNews("/dmoz.org/Society/Future/",news);
}	

// %>
