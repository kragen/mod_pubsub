using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using LibKNDotNet;

namespace SubTest2
{
	class MyHandler : IRequestStatusHandler
	{
		public MyHandler(Form1 f)
		{
			m_F = f;
		}

		private void DumpMsg(string text, LibKNDotNet.Message msg)
		{
			m_F.AddString(text);
			foreach (MessageEntry me in msg)
			{
				string s;
				s = "\t" + me.Field + ":" + me.Value;
				m_F.AddString(s);
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

		Form1 m_F = null;
	}

	class MyListener : IListener
	{
		public MyListener(Form1 f)
		{
			m_F = f;
		}

		public override void OnUpdate(LibKNDotNet.Message msg)
		{
			//Console.WriteLine("OnUpdate");
			m_F.AddString("OnUpdate");
			foreach (MessageEntry me in msg)
			{
				m_F.AddString(me.Field + ":" + me.Value);
			}
		}

		Form1 m_F = null;
	}

	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class Form1 : System.Windows.Forms.Form
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		private System.Windows.Forms.TextBox textBox1;
		MyListener m_Listener = null;
		MyHandler m_MyH = null;
		Connector m_Connector = null;
		string m_Rid = null;

		public Form1()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			m_Listener = new MyListener(this);
			m_MyH = new MyHandler(this);
			m_Connector = new Connector();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		public void AddString(string s)
		{
			textBox1.Text += s;
			textBox1.Text += "\r\n";
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.textBox1 = new System.Windows.Forms.TextBox();
			this.SuspendLayout();
			// 
			// textBox1
			// 
			this.textBox1.AccessibleName = "";
			this.textBox1.Location = new System.Drawing.Point(16, 16);
			this.textBox1.Multiline = true;
			this.textBox1.Name = "textBox1";
			this.textBox1.ScrollBars = System.Windows.Forms.ScrollBars.Both;
			this.textBox1.Size = new System.Drawing.Size(264, 232);
			this.textBox1.TabIndex = 0;
			this.textBox1.Text = "";
			// 
			// Form1
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(292, 266);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.textBox1});
			this.Name = "Form1";
			this.Text = "Form1";
			this.Closing += new System.ComponentModel.CancelEventHandler(this.Form1_Closing);
			this.Load += new System.EventHandler(this.Form1_Load);
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new Form1());
		}

		private void Form1_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			if (m_Connector.IsConnected())
			{
				m_Connector.Unsubscribe(m_Rid, m_MyH);
				m_Rid = null;
				m_Connector.Close();
				m_Connector = null;
			}
		}

		private void Form1_Load(object sender, System.EventArgs e)
		{
			Parameters p = new Parameters();
			p.ServerUrl = "http://localhost:8000/kn";

			if (m_Connector.Open(p))
			{
				string rid = m_Connector.Subscribe("/what/knchat/messages", m_Listener, new LibKNDotNet.Message(), m_MyH);
				m_Rid = rid;
				AddString("rid = " + rid);
			}
			else
			{
				AddString("Failed to connect");
			}
		}
	}
}
