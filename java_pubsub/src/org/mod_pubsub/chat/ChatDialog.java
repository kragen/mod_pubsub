/*
 * Copyright (c) 2003 Robert Leftwich.  All Rights Reserved.
 *
 */
package org.mod_pubsub.chat;

import java.net.MalformedURLException;
import java.util.HashMap;
import java.util.Map;

import net.xmlrouter.mod_pubsub.client.Listener;
import net.xmlrouter.mod_pubsub.client.SimpleRouter;

import org.wxwindows.wxBoxSizer;
import org.wxwindows.wxButton;
import org.wxwindows.wxCloseEvent;
import org.wxwindows.wxCloseListener;
import org.wxwindows.wxCommandEvent;
import org.wxwindows.wxCommandListener;
import org.wxwindows.wxDialog;
import org.wxwindows.wxPoint;
import org.wxwindows.wxSize;
import org.wxwindows.wxTextCtrl;

/**
 * @author Robert Leftwich
 */
public class ChatDialog extends wxDialog {

	protected String myId;
	protected wxTextCtrl myText2Send;
	protected wxTextCtrl myChatText;
	protected SimpleRouter myRouter;
	protected String myRouteIds;

	public ChatDialog(String host, String id, String title, wxPoint pos, wxSize size) throws MalformedURLException {
		super(null, -1, title);

		SetSize(size);
		Move(pos);

		EVT_CLOSE(new OnQuit());

		wxBoxSizer overallSizer = new wxBoxSizer(wxVERTICAL);

		wxBoxSizer boxSizer = new wxBoxSizer(wxHORIZONTAL);
		
		myId = id;

		myText2Send = new wxTextCtrl(this, -1, "Type here...", wxDefaultPosition, new wxSize(400, 20));
		boxSizer.Add(myText2Send, 0, wxEXPAND | wxALL, 10);

		wxButton sendButton = new wxButton(this, -1, "Chat!");
		sendButton.SetDefault();
		EVT_BUTTON(sendButton.GetId(), new OnSend());
		boxSizer.Add(sendButton, 0, wxALL, 5);

		wxButton clearButton = new wxButton(this, -1, "Clear");
		EVT_BUTTON(clearButton.GetId(), new OnClear());
		boxSizer.Add(clearButton, 0, wxALL, 5);

		overallSizer.Add(boxSizer, 0, wxALIGN_CENTRE, 0);

		myChatText =
			new wxTextCtrl(this, -1, "", wxDefaultPosition, new wxSize(300, 300), wxTE_READONLY | wxTE_MULTILINE);
		overallSizer.Add(myChatText, 1, wxEXPAND | wxALL, 10);

		SetSizer(overallSizer);

		myRouter = new SimpleRouter(host);
		Map aMsg = new HashMap();
		aMsg.put("displayname", id);
		aMsg.put("userid", id);
		aMsg.put("client_id", id);
		aMsg.put("do_max_n", "10");
		myRouteIds = myRouter.subscribeWithRandomJournal("/what/chat", new OnMessage(), aMsg);

	}

	class OnQuit implements wxCloseListener {

		/* (non-Javadoc)
		 * @see org.wxwindows.wxCloseListener#handleEvent(org.wxwindows.wxCloseEvent)
		 */
		public void handleEvent(wxCloseEvent arg0) {
			String[] routeIds = myRouteIds.split("\\|");
			
			myRouter.unsubscribe_session(routeIds[1]);
			myRouter.unsubscribe_session(routeIds[0]);
			myRouter.unsubscribe(routeIds[0]);
			
			// --> Don't use Close with a wxDialog,
			// use Destroy instead.
			Destroy();
		}

	}

	class OnSend implements wxCommandListener {

		/* (non-Javadoc)
		* @see org.wxwindows.wxCommandListener#handleEvent(org.wxwindows.wxCommandEvent)
		*/
		public void handleEvent(wxCommandEvent arg0) {

			String text2Send = myText2Send.GetValue();
			
			if (null != text2Send && text2Send.length() > 0) {
				Map aMsg = new HashMap();
				aMsg.put("displayname", myId);
				aMsg.put("userid", myId);
				aMsg.put("kn_payload", text2Send);
				aMsg.put("kn_expires", "+86400");
				myRouter.publish("/what/chat", aMsg);
				myText2Send.Clear();
			}
			myText2Send.SetFocus();
		}

	}

	class OnClear implements wxCommandListener {

		/* (non-Javadoc)
		* @see org.wxwindows.wxCommandListener#handleEvent(org.wxwindows.wxCommandEvent)
		*/
		public void handleEvent(wxCommandEvent arg0) {

			myChatText.SetValue("");
		}

	}

	class OnMessage implements Listener {

		/* (non-Javadoc)
		* @see net.xmlrouter.mod_pubsub.client.Listener#onMessage(java.util.Map)
		*/
		public void onMessage(Map msg) {
			
			String displayName = (String)msg.get("displayname");
			String payload = (String)msg.get("kn_payload");
			
			if (null != displayName && null != payload) {
				String text = "[" + displayName + "] " + payload + "\n";
				String currValue = myChatText.GetValue();
				myChatText.SetValue(text + currValue);
			}
		}

	}

}
