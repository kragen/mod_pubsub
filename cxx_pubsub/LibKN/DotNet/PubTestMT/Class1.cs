using System;

enum Args
{
	Expected = 1,
};

namespace PubTestMT
{
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
			if (args.Length != Args.Expected)
			{
				Console.WriteLine("Expecting {0} args, got {1} instead.", Args.Expected, args.Length);
				return;
			}
		}
	}
}
