using System;
using LibKNDotNet;

namespace PubTest1
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

	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Class1
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			//
			// TODO: Add code to start application here
			//
			if (args.Length != 2)
			{
				Console.WriteLine("Wrong number of arguments. Got {0} instead.", args.Length);
				return;
			}

			MyHandler myH = new MyHandler();
			Parameters p = new Parameters();
			p.ServerUrl = args[0];

			Connector c = new Connector();
			if (c.Open(p))
			{
				Message m = new Message();
				m.Set("do_method", "notify");
				m.Set("kn_to", args[1]);
				m.Set("kn_payload", "Hello");
				m.Set("nickname", "dotnet");
				m.Set("kn_response_format", "simple");

				c.Publish(m, myH);

				c.Close();
			}
		}
	}
}
