using System;
using System.Threading;
using LibKNDotNet;

enum Args
{
	Server,
	NumThreads,
	Expected,
};

namespace SubTestMT
{
	class Util
	{
		static public void DumpMsg(string text, LibKNDotNet.Message msg)
		{
			Console.WriteLine(text);
			foreach (MessageEntry me in msg)
			{
				Console.WriteLine("\t{0}:{1}", me.Field, me.Value);
			}
		}
	}

	class MyHandler : IRequestStatusHandler
	{
		public MyHandler()
		{
		}

		public override void OnSuccess(LibKNDotNet.Message msg)
		{
			Util.DumpMsg("Success", msg);
		}

		public override void OnError(LibKNDotNet.Message msg)
		{
			Util.DumpMsg("Error", msg);
		}
	}

	class MyListener : IListener
	{
		int m_Id;

		public MyListener(int id)
		{
			m_Id = id;
		}

		public override void OnUpdate(LibKNDotNet.Message msg)
		{
			Util.DumpMsg("OnUpdate " + m_Id.ToString(), msg);
		}
	}

	class CSubThread
	{
		const string Prefix = "/MT/Foo";
		string m_Topic = "";
		Thread m_T = null;
		int m_Id = 0;
		MyHandler m_MyH = null;
		MyListener m_MyListener = null;

		public CSubThread(int id)
		{
			m_Id = id;
			m_MyH = new MyHandler();
			m_MyListener = new MyListener(id);
			m_Topic = Prefix + "/" + id.ToString();
			m_T = new Thread(new ThreadStart(ThreadProc));
		}

		private void ThreadProc()
		{
			Console.WriteLine("Begin ThreadProc [{0}]", m_Id);

			string rid = Test.GetConnector().Subscribe(m_Topic, m_MyListener, new Message(), m_MyH);

			if (rid.Length != 0)
			{
				Console.WriteLine("{0}: {1}", m_Id.ToString(), rid);

				while (!Test.Quit())
					Thread.Sleep(100);

				Test.GetConnector().Unsubscribe(rid, m_MyH);
			}

			Console.WriteLine("End ThreadProc [{0}]", m_Id);
		}

		public void Start()
		{
			m_T.Start();
		}

		public bool IsAlive()
		{
			return m_T.IsAlive;
		}
	}

	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Test
	{
		private static Parameters m_Params = null;
		private static Connector m_Connector = null;
		private static bool m_Quit = false;
		int m_NumThreads = 0;

		public static Connector GetConnector()
		{
			lock (typeof(Test))
			{
				return m_Connector;
			}
		}

		public static bool Quit()
		{
			lock (typeof(Test))
			{
				return m_Quit;
			}
		}

		public Test(string[] args)
		{
			m_Connector = new Connector();
			m_Params = new Parameters();

			m_Params.ServerUrl = args[(int)Args.Server];

			m_NumThreads = int.Parse(args[(int)Args.NumThreads]);
		}

		private void AppThread()
		{
			Console.WriteLine("In AppThread");

			if (m_Connector.Open(m_Params))
			{
				CSubThread[] pts = new CSubThread[m_NumThreads];
 
				for (int tid = 0; tid < m_NumThreads; tid++)
				{
					pts[tid] = new CSubThread(tid);
					pts[tid].Start();
				}

				Console.WriteLine("Waiting for events, enter a string to exit the application.");
				Console.ReadLine();

				m_Quit = true;

				bool toContinue = true;
				while (toContinue)
				{
					toContinue = false;

					for (int i = 0; i < m_NumThreads; i++)
					{
						toContinue = toContinue | pts[i].IsAlive();
					}

					Thread.Sleep(100);
				}

				m_Connector.Close();
			}

			Console.WriteLine("Exiting AppThread");
		}

		public void Go()
		{
			Thread t = new Thread(new ThreadStart(AppThread));
			t.Start();
		}

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			//
			// TODO: Add code to start application here
			//
			if (args.Length != (int)(Args.Expected))
			{
				Console.WriteLine("Expecting {0} args, got {1} instead.", (int)(Args.Expected), args.Length);
				return;
			}

			Test t = new Test(args);
			t.Go();
		}
	}
}
