using System;
using System.Threading;
using LibKNDotNet;

enum Args
{
	Server,
	NumThreads,
	NumEvents,
	Expected,
};

namespace PubTestMT
{
	class MyHandler : IRequestStatusHandler
	{
		public MyHandler()
		{
		}

		private void DumpMsg(string text, LibKNDotNet.Message msg)
		{
			Console.WriteLine(text);
			foreach (MessageEntry me in msg)
			{
				Console.WriteLine("\t{0}:{1}", me.Field, me.Value);
			}
		}

		public override void OnSuccess(LibKNDotNet.Message msg)
		{
			DumpMsg("Success", msg);
		}

		public override void OnError(LibKNDotNet.Message msg)
		{
			DumpMsg("Error", msg);
		}
	}

	class CPubThread
	{
		const string Prefix = "/MT/Foo";
		int m_NumEvents = 0;
		string m_Topic;
		Thread m_T;
		int m_Id;
		MyHandler m_MyH = null;

		public CPubThread(int id, int numEvents)
		{
			m_MyH = new MyHandler();

			m_Id = id;
			m_NumEvents = numEvents;
			m_Topic = Prefix + "/" + id.ToString();
			m_T = new Thread(new ThreadStart(ThreadProc));
		}

		private void ThreadProc()
		{
			Console.WriteLine("Begin ThreadProc [{0}]", m_Id);

				Message m = new Message();
				m.Set("do_method", "notify");
				m.Set("kn_to", m_Topic);
				m.Set("nickname", "dotnet " + m_Id.ToString());
				m.Set("kn_response_format", "simple");

				for (int i = 0; i < m_NumEvents; i++)
				{
					m.Set("kn_payload", "Hello " + i.ToString());

					{
						if (!Test.GetConnector().Publish(m, m_MyH))
						{
							Console.WriteLine("[{0}] failed {1}", m_Id, i.ToString());
						}
					}
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

		public static Connector GetConnector()
		{
			lock (typeof(Test))
			{
				return m_Connector;
			}
		}

		int m_NumThreads = 0;
		int m_NumEvents = 0;

		public Test(string[] args)
		{
			m_Connector = new Connector();
			m_Params = new Parameters();

			m_Params.ServerUrl = args[(int)Args.Server];

			m_NumThreads = int.Parse(args[(int)Args.NumThreads]);
			m_NumEvents = int.Parse(args[(int)Args.NumEvents]);
		}

		private void AppThread()
		{
			Console.WriteLine("In AppThread");

			if (m_Connector.Open(m_Params))
			{
				CPubThread[] pts = new CPubThread[m_NumThreads];
 
				for (int tid = 0; tid < m_NumThreads; tid++)
				{
					pts[tid] = new CPubThread(tid, m_NumEvents);
					pts[tid].Start();
				}

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
