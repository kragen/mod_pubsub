// <%@LANGUAGE = JScript %>
// <%

var g_routerURI = "http://www.topiczero.com:8000/kn/";
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

function loadSales(url)
{
	var feed = loadXML(url);
	return feed.selectNodes("/ProductInfo/Details");
}

function forwardSales(topic,items)
{
	var poster = WScript.CreateObject("Microsoft.XMLHTTP");
	var item;
	var title;
	var author="Ms. Anona";
	var link;
	var rank;
	var body;

	var content;

	for (var i=0; i < items.length; i++)
	{
		item = items[i];
		
		title = item.selectSingleNode("ProductName");
		if (title)
			title = title.text;

		rank = item.selectSingleNode("SalesRank");
		if (rank)
			rank = rank.text;
		else
			rank = "none";

		author = item.selectSingleNode("Authors/Author");
		if (author)
			author = author.text;			
		else
			author = "";

		link = item.selectSingleNode("@url");
		if (link)
			link = link.text;
			
		if (title && link)
		{
			var reg;

			poster.open("POST",g_routerURI,false);
			body = "do_method=notify"+
			"&kn_expires=%2b120"+
			"&type=status"+
			"&kn_to="+topic+
			"&title="+title+
			"&rank="+rank+
			"&author="+author+
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

function forward(pages,categoryId,mode,topic)
{
	var sales;
	for (var i=1; i <= pages; i++)
	{
		sales=loadSales("http://xml.amazon.com/onca/xml2?t=topiczero-20&dev-t=DSJ8HIOHBMG48&BrowseNodeSearch="+categoryId+"&mode="+mode+"&type=heavy&page="+i+"&f=xml&sort=+salesrank");
		forwardSales(topic,sales);
	}	
}

function main(args)
{
	// books, top selling=1000
	forward(4,1000,"books","/amazon.com/sales/books/");
	forward(4,301668,"music","/amazon.com/sales/music/");
	forward(4,404272,"vhs","/amazon.com/sales/movies/");
	forward(4,130,"dvd","/amazon.com/sales/movies/");
	forward(4,491286,"software","/amazon.com/sales/software/");
	forward(4,491290,"toys","/amazon.com/sales/toys/");
}	

// %>
