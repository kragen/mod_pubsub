using System;
using LibKNDotNet;

namespace MessageTest1
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Class1
	{
		static void DumpMessage(Message m)
		{
			foreach (MessageEntry me in m)
			{
				Console.WriteLine(me.Field + ": " + me.Value);
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
			Message m = new Message();
			DumpMessage(m);
			m.Set("t", "abc");
			DumpMessage(m);

			String v = "a";
			if (m.Get("t", ref v))
			{
				Console.WriteLine("succeeded: {0}", v);
			}
			else
			{
				Console.WriteLine("Get failed");
			}
		}
	}
}
