using System;
using LibKNDotNet;

namespace SubTest1
{
    class MyStatusHandler : IRequestStatusHandler
    {
        public override void OnStatus(LibKNDotNet.Message msg)
        {
            Console.WriteLine("OnStatus");
        }
    }
	class MyListener : IListener
	{
		public override void OnUpdate(LibKNDotNet.Message msg)
		{
			Console.WriteLine("OnUpdate");		
		}
	}

	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Class1
	{
		Connector m_C = new Connector();

		public Class1()
		{

		}

		public void Go()
		{
			Parameters p = new Parameters();
			p.ServerUrl = "http://localhost:8000/kn";

			if (m_C.Open(p))
			{
				string rid = m_C.Subscribe("/what/knchat/messages", new MyListener(), new Message(), new MyStatusHandler());
				Console.WriteLine("rid " + rid);

				m_C.Close();
			}
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
			Class1 c = new Class1();
			c.Go();
		}
	}
}
