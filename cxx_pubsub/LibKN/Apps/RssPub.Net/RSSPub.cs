using System;
using System.Threading;
using System.Collections;
using System.Xml;
using System.Text;
using LibKNDotNet;

enum Args
{
	Server,
	XmlPath,
	Expected,
};

namespace RssPub.Net
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class RSSPub
	{
		Connector m_Connector;

		RSSPub(string serverUrl)
		{
			m_Connector = new Connector();
			Parameters p = new Parameters();
			p.ServerUrl = serverUrl;
			m_Connector.Open(p);
		}

		void ProcessRss(XmlDocument xd)
		{
			XmlNodeList nl = xd.SelectNodes("/rss/channel/item");
			if (nl.Count == 0)
				return;

			Console.WriteLine("There are {0} items", nl.Count.ToString());

			IEnumerator ienum = nl.GetEnumerator();
			while (ienum.MoveNext())
			{
				XmlNode n = (XmlNode)ienum.Current;
				ProcessRSSItem(n);
				Thread.Sleep(60 * 1000);
			}
		}

		void ProcessRSSItem(XmlNode n)
		{
			string srcTitle = n["title"].InnerText;
			string srcLink = n["link"].InnerText;
			string srcDesc = n["description"].InnerText;

			Console.WriteLine(srcLink);

			XmlDocument xd = new XmlDocument();
			try
			{
				xd.Load(srcLink);
				XmlNodeList nl = xd.SelectNodes("/rss/channel/item");
				if (nl.Count == 0)
					return;

				string prefix = "  ";
				Console.WriteLine(prefix + "There are {0} items", nl.Count.ToString());

				Message m = new Message();

				for (int i = nl.Count - 1; i >= 0; i--)
				{
					try
					{
						XmlNode item = nl[i];
						string title = item["title"].InnerText;
						string link = item["link"].InnerText;
						string desc = item["description"].InnerText;
						Console.WriteLine(prefix + i.ToString() + ":" + title);
						Console.WriteLine(prefix + prefix + link);
						Console.WriteLine(prefix + prefix + desc);

						m.Set("do_method", "notify");
						m.Set("kn_response_format", "simple");
						m.Set("kn_to", "/headlines");
						m.Set("kn_id", link);
						m.Set("title", title);
						m.Set("link", link);
						m.Set("description", desc);
						m_Connector.Publish(m, null);
					}
					catch (Exception ie)
					{
					}
				}
			}
			catch (Exception e)
			{
				Console.WriteLine(e.ToString());
				Console.WriteLine("Failed to load " + srcTitle + " " + srcDesc);
			}
		}

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			if (args.Length != (int)Args.Expected)
			{
				Console.WriteLine("Wrong number of arguments. Expecting {0}, but got {1} instead.",
					(int)Args.Expected, args.Length);
				return;
			}

			RSSPub rp = new RSSPub(args[(int)Args.Server]);
			XmlDocument xd = new XmlDocument();
			string xmlFile = args[(int)Args.XmlPath];

			try
			{
				Console.WriteLine("Processing " + xmlFile);
				xd.Load(xmlFile);
			}
			catch (Exception e)
			{
				Console.WriteLine("Exception " + e.ToString());
				Console.WriteLine("Exiting...");
				return;
			}

			while (true)
			{
				rp.ProcessRss(xd);
				Thread.Sleep(60 * 1000);
			}
		}
	}
}
