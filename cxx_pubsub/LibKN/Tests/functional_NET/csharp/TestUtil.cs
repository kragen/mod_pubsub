using System;
using System.Xml;
using System.Collections;
using System.IO;

namespace TestUtil
{
	/// <summary>
	/// Summary description for TestUtil.
	/// </summary>
	public class TestUtil
	{
		public static readonly string TU_OK = "OK";
		public static readonly string TU_PARAMS_FILE = "../../../../params.xml";

		private static bool TU_VERBOSE_ALL = false;
		private static string TU_TESTSUITE = "";
		private static string TU_TESTCASE = "";
		private static string TU_SERVER = "";
		private static string TU_MISSINGSERVER = "";
		private static string TU_SERVERNAME = "";

		private static ArrayList tu_servers = new ArrayList();
		
		private static int tu_count_subListener_OnUpdate = 0;
		private static int tu_count_subConnSH_OnConnStatus = 0;
		private static int tu_count_subReqSH_OnError = 0;
		private static int tu_count_subReqSH_OnSuccess = 0;
		private static int tu_count_pubConnSH_OnConnStatus = 0;
		private static int tu_count_pubReqSH_OnError = 0;
		private static int tu_count_pubReqSH_OnSuccess = 0;


		public TestUtil()
		{
			//
			// TODO: Add constructor logic here
			//
		}

		//////////////////////////////////////////////////////////////////////////
		//// Public Events
		//////////////////////////////////////////////////////////////////////////

		public class SubListener : LibKNDotNet.IListener
		{
			public override void OnUpdate(LibKNDotNet.Message msg)
			{
				dumpMsg("TU_SubListener.OnUpdate", msg);
				tu_count_subListener_OnUpdate++;
			}
		}

		public class SubConnStatusHandler : LibKNDotNet.IConnectionStatusHandler
		{
			public override void OnConnectionStatus(LibKNDotNet.Message msg)
			{
				dumpMsg("TU_SubConnStatusHandler.OnConnectionStatus", msg);
				tu_count_subConnSH_OnConnStatus++;
			}
		}

		public class PubConnStatusHandler : LibKNDotNet.IConnectionStatusHandler
		{
			public override void OnConnectionStatus(LibKNDotNet.Message msg)
			{
				dumpMsg("TU_PubConnStatusHandler.OnConnectionStatus", msg);
				tu_count_pubConnSH_OnConnStatus++;
			}
		}

		public class SubRequestStatusHandler : LibKNDotNet.IRequestStatusHandler
		{
			public override void OnError(LibKNDotNet.Message msg)
			{
				dumpMsg("TU_SubRequestStatusHandler.OnError", msg);
				tu_count_subReqSH_OnError++;
			}
			public override void OnSuccess(LibKNDotNet.Message msg)
			{
				dumpMsg("TU_SubRequestStatusHandler.OnSuccess", msg);
				tu_count_subReqSH_OnSuccess++;
			}
		}

		public class PubRequestStatusHandler : LibKNDotNet.IRequestStatusHandler
		{
			public override void OnError(LibKNDotNet.Message msg)
			{
				dumpMsg("TU_PubRequestStatusHandler.OnError", msg);
				tu_count_pubReqSH_OnError++;
			}
			public override void OnSuccess(LibKNDotNet.Message msg)
			{
				dumpMsg("TU_PubRequestStatusHandler.OnSuccess", msg);
				tu_count_pubReqSH_OnSuccess++;
			}
		}

		
		//////////////////////////////////////////////////////////////////////////
		//// Public
		//////////////////////////////////////////////////////////////////////////
		
		public static int getCount_subListener_OnUpdate(){ return tu_count_subListener_OnUpdate; }
		public static int getCount_subConnSH_OnConnStatus(){ return tu_count_subConnSH_OnConnStatus; }
		public static int getCount_subReqSH_OnError(){ return tu_count_subReqSH_OnError; }
		public static int getCount_subReqSH_OnSuccess(){ return tu_count_subReqSH_OnSuccess; }
		public static int getCount_pubConnSH_OnConnStatus(){ return tu_count_pubConnSH_OnConnStatus; }
		public static int getCount_pubReqSH_OnError(){ return tu_count_pubReqSH_OnError; }
		public static int getCount_pubReqSH_OnSuccess(){ return tu_count_pubReqSH_OnSuccess; }

		public static string getTestsuiteName(){ return TU_TESTSUITE; }
		public static string getTestcaseName(){ return TU_TESTCASE; }

		public static string getServerName(){ return TU_SERVERNAME; }
		public static string getServer() { return TU_SERVER; }
		public static string getServer(int index) 
		{
			if( tu_servers.Count < index )
			{
				return (string)tu_servers[index];
			}
			else
			{
				return "";
			}
		}

		public static string getMissingServer(){ return TU_MISSINGSERVER; }

		/*
		 * Use this if you are rerunning the same testsuite or testcase and
		 * want to force re-reading from the XML params.xml file.
		 */
		public static void RESET_TESTSUITE()
		{
			// reset everything.
			TU_VERBOSE_ALL = false;
			TU_TESTSUITE = "";
			TU_TESTCASE = "";
			TU_SERVER = "";
			TU_MISSINGSERVER = "";
			TU_SERVERNAME = "";
			tu_servers = new ArrayList();
			resetCounters();
		}

		public static void INIT_TESTSUITE(string testSuiteName)
		{
			// if new testsuite name restart everything, 
			// else same testsuite so leave everything the same.
			if( testSuiteName.Equals(TU_TESTSUITE) ) return;

			// reset everything.
			RESET_TESTSUITE();

			// setup everything from XML params.xml
			TU_TESTSUITE = testSuiteName;
			TU_VERBOSE_ALL = false;
			
			XmlTextReader reader = new XmlTextReader(TU_PARAMS_FILE);
			while(reader.Read())
			{	
				if( reader.Name.Equals("verbose") )
				{
					TU_VERBOSE_ALL = true;				
				}
				else if( reader.Name.Equals("serverName") )
				{
					if( reader.IsStartElement() )
					{
						reader.Read();
						TU_SERVERNAME = reader.Value;	
					}
				}
				else if( reader.Name.Equals("server") )
				{
					if( reader.IsStartElement() )
					{
						reader.Read();
						tu_servers.Add(reader.Value);	
					}
				}
				else if( reader.Name.Equals("missingServer") )
				{
					if( reader.IsStartElement() )
					{
						reader.Read();
						TU_MISSINGSERVER = reader.Value;	
					}
				}
			}
			if( tu_servers.Count > 0 )
			{
				TU_SERVER = (string)tu_servers[0];
			}

			if(!isVerbose()) return;

			Console.WriteLine();
			Console.WriteLine("===============================================================================");
			Console.WriteLine("TU_VERBOSE_ALL  =" + TU_VERBOSE_ALL);
			Console.WriteLine("TU_SERVERNAME   =" + TU_SERVERNAME);
			Console.WriteLine("TU_SERVER       =" + TU_SERVER);
			Console.WriteLine("TU_MISSINGSERVER=" + TU_MISSINGSERVER);
			Console.WriteLine("tu_servers      =" + tu_toString(tu_servers));
			Console.WriteLine("===============================================================================");
		}

		public static void INIT_TESTCASE(string testCaseName)
		{
			TU_TESTCASE = testCaseName;
			if(!isVerbose()) return;

			Console.WriteLine();
			Console.WriteLine("===============================================================================");
			Console.WriteLine("=== Start test: " + TU_TESTSUITE + "." + testCaseName);
			Console.WriteLine("===============================================================================");
		}

		public static void dump(string text)
		{
			if(!isVerbose()) return;

			Console.Write("<" + TU_TESTSUITE + "." + TU_TESTCASE + "> ");
			Console.WriteLine(text);
		}
		
		public static void dumpMsg(string text, LibKNDotNet.Message msg)
		{
			if(!isVerbose()) return;

			Console.Write("<" + TU_TESTSUITE + "." + TU_TESTCASE + "> ");
			Console.WriteLine(text);

			System.Collections.IEnumerator ienum = msg.GetEnumerator();
			LibKNDotNet.MessageEntry me;
			while(ienum.MoveNext())
			{
				me = (LibKNDotNet.MessageEntry)ienum.Current;
				Console.WriteLine("        " + me.Field + ":" + me.Value);
			}
		}

		public static void dumpCounters()
		{
			if(!isVerbose()) return;

			Console.WriteLine("<" + TU_TESTSUITE + "." + TU_TESTCASE + "> Event Counters:");
			Console.WriteLine("        sub Listener OnUpdate     =" + tu_count_subListener_OnUpdate);
			Console.WriteLine("        sub ConnSH   OnConnStatus =" + tu_count_subConnSH_OnConnStatus);
			Console.WriteLine("        sub ReqSH    OnError      =" + tu_count_subReqSH_OnError);
			Console.WriteLine("        sub ReqSH    OnSuccess    =" + tu_count_subReqSH_OnSuccess);
			Console.WriteLine("        pub ConnSH   OnConnStatus =" + tu_count_pubConnSH_OnConnStatus);
			Console.WriteLine("        pub ReqSH    OnError      =" + tu_count_pubReqSH_OnError);
			Console.WriteLine("        pub ReqSH    OnSuccess    =" + tu_count_pubReqSH_OnSuccess);
		}

		public static void resetCounters()
		{
			tu_count_subListener_OnUpdate = 0;
			tu_count_subConnSH_OnConnStatus = 0;
			tu_count_subReqSH_OnError = 0;
			tu_count_subReqSH_OnSuccess = 0;
			tu_count_pubConnSH_OnConnStatus = 0;
			tu_count_pubReqSH_OnError = 0;
			tu_count_pubReqSH_OnSuccess = 0;
		}

		public static string checkCounters(int sL_OU
			, int sCSH_OCS
			, int sRSH_OE
			, int sRSH_OS
			, int pCSH_OCS
			, int pRSH_OE
			, int pRSH_OS )
		{
			bool err = false;
			StringWriter sw = new StringWriter();
			sw.WriteLine();
			if( sL_OU != -1){
				sw.WriteLine("         sub Listener OnUpdate   Got:" + tu_count_subListener_OnUpdate + " Expected:" + sL_OU);
				if( sL_OU != tu_count_subListener_OnUpdate ) err = true;
			}
			if( sCSH_OCS != -1){
				sw.WriteLine("         sub ConnSH OnConnStatus Got:" + tu_count_subConnSH_OnConnStatus + " Expected:" + sCSH_OCS);
				if( sCSH_OCS != tu_count_subConnSH_OnConnStatus ) err = true;
			}
			if( sRSH_OE != -1){
				sw.WriteLine("         sub ReqSH OnError       Got:" + tu_count_subReqSH_OnError + " Expected:" + sRSH_OE);
				if( sRSH_OE != tu_count_subReqSH_OnError ) err = true;
			}
			if( sRSH_OS != -1)
			{
				sw.WriteLine("         sub ReqSH OnSuccess     Got:" + tu_count_subReqSH_OnSuccess + " Expected:" + sRSH_OS);
				if( sRSH_OS != tu_count_subReqSH_OnSuccess ) err = true;
			}
			if( pCSH_OCS != -1){
				sw.WriteLine("         pub ConnSH OnConnStatus Got:" + tu_count_pubConnSH_OnConnStatus + " Expected:" + pCSH_OCS);
				if( pCSH_OCS != tu_count_pubConnSH_OnConnStatus ) err = true;
			}
			if( pRSH_OE != -1){
				sw.WriteLine("         pub ReqSH OnError       Got:" + tu_count_pubReqSH_OnError + " Expected:" + pRSH_OE);
				if( pRSH_OE != tu_count_pubReqSH_OnError ) err = true;
			}
			if( pRSH_OS != -1){
				sw.WriteLine("         pub ReqSH OnSuccess     Got:" + tu_count_pubReqSH_OnSuccess + " Expected:" + pRSH_OS);
				if( pRSH_OS != tu_count_pubReqSH_OnSuccess ) err = true;
			}
			if( err )
			{
				return sw.ToString();
			}
			else
			{
				return TU_OK;
			}
		}

		private static bool isVerbose()
		{
			if(TU_VERBOSE_ALL) return true;
	
			bool result = false;
			//for(int i = 0; i < tu_verbose.size(); i++)
			//{
			//	if(TU_TESTSUITE == tu_verbose[i])
			//	{
			//		return true;
			//	}
			//}
			return result;
		}

		private static string tu_toString(ArrayList al)
		{
			StringWriter sw = new StringWriter();
			for(int i = 0; i < al.Count; i++)
			{
				sw.Write(al[i].ToString() + " ");
			}
			return sw.ToString();
		}

	}
}
