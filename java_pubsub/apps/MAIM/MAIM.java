/* August 12, 2002.  Adam Rifkin, Adam@KnowNow.Com

   MAIM = Mod_pubsub AIM (a bridge example)

   This application is an example of a bridge between
   KnowNow's Java Microserver and AOL Instant Messenger.

   It needs to be ported to use the mod_pubsub Java PubSub library.

   This program uses some classes that are Copyright 2002 by Jeff Heaton
   We found Jeff's source at: http://www.jeffheaton.com/source/aim.zip
   You can find the GNU GPL at: http://www.gnu.org/copyleft/gpl.html

   To compile this program, you need a KnowNow Java Microserver
   installed and have your CLASSPATH set properly to include it.
   You also need a Java Runtime; I suggest http://java.sun.com/j2se/1.4/

   To use this program, you need to sign up for a "bridge" AIM account
   that will send messages to your AIM account.

   This program sets up a connection as follows:

    events -> PubSub Server topics -> KnowNow Java Microserver ->
       MAIM program -> bridgeAIMHandle account -> myAIMHandle account

   This code is completely unsupported in every possible way.

   To compile: javac MAIM.java
   To run: java MAIM

   How I compile: c:\j2sdk1.4.0_01\bin\javac MAIM.java
   Another example of how to run:  java -Drouter="http://sales.knownow.com/kn" -Dtopic="/what/chat" -Daimhandle="KnowNowBuddy" -Dmessagehead="" -Dbridgeaimhandle="KnowNowAIM" MAIM

*/

/**
 * Copyright (c) 2002-2003 KnowNow, Inc.  All rights reserved.
 *
 * @KNOWNOW_LICENSE_START@
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 * 
 * 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
 * be used to endorse or promote any product without prior written
 * permission from KnowNow, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @KNOWNOW_LICENSE_END@
 **/


import com.knownow.microserver.*;
import com.knownow.common.*;
import java.util.*;
import java.net.*;
import java.io.*;

public class MAIM implements KNEventListener,Chatable {

  // AIM Java library class
  public JavaTOC myJavaTOC;

  // KnowNow Java Microserver library classes
  static public KNJServer myJavaMicroserver;
  static public KNEvent anEvent = new KNEvent();

  // Use java -Dtopic="/what/chat" to set property topic.  Et cetera.
  // These are Strings used by MAIM.
  static public String myRouter = System.getProperty("router");
  static public String myTopic = System.getProperty("topic");
  static public String myAIMHandle = System.getProperty("aimhandle");
  static public String messageHead = System.getProperty("messagehead");
  static public String bridgeAIMHandle = System.getProperty("bridgeaimhandle");
  static public String bridgePassword = System.getProperty("bridgepassword");

  /* Constructor, baby! */
  public MAIM() {
    myJavaTOC = new JavaTOC(this);
    myJavaMicroserver = new KNJServer(myRouter);   
  }

  /* Chatable interfaces are three: unknown, im, and error */
  public void unknown(String str) {} 

  public void im(String from, String message) {
    // A typical AOL instant message comes in the format
    // <HTML><BODY BGCOLOR="#ffffff"><FONT LANG="0">Hi!</FONT></BODY></HTML>
    // so we have to parse to the fourth token.
    // Damn this is an ugly hack.  It doesn't work for IM's formatted
    // any other way.
    String pubMessage;
    StringTokenizer parsedMessage = new StringTokenizer(message, "<>");
    parsedMessage.nextToken();
    parsedMessage.nextToken();
    parsedMessage.nextToken();
    pubMessage = parsedMessage.nextToken();
    anEvent.put("kn_payload", pubMessage);
    anEvent.put("displayname", from);
    anEvent.put("user_id", from);
    System.out.println("Published: [" + from + "] " + pubMessage);
    myJavaMicroserver.publish(myTopic, anEvent, null);
  }

  public void error(String str,String var) {
    String error;
    switch ( Integer.parseInt(str) ) {
      case 901:error="$1 not currently available";break;
      case 902:error="Warning of $1 not currently available";break;
      case 903:error="A message has been dropped, you are exceeding the server speed limit";break;
      case 911:error="Error validating input";break;
      case 912:error="Invalid account";break;
      case 913:error="Error encountered while processing request";break;
      case 914:error="Service unavailable";break;
      case 950:error="Chat in $1 is unavailable.";break;
      case 960:error="You are sending message too fast to $1";break;
      case 961:error="You missed an im from $1 because it was too big.";break;
      case 962:error="You missed an im from $1 because it was sent too fast.";break;
      case 970:error="Failure";break;
      case 971:error="Too many matches";break;
      case 972:error="Need more qualifiers";break;
      case 973:error="Dir service temporarily unavailable";break;
      case 974:error="Email lookup restricted";break;
      case 975:error="Keyword Ignored";break;
      case 976:error="No Keywords";break;
      case 977:error="Language not supported";break;
      case 978:error="Country not supported";break;
      case 979:error="Failure unknown $1";break;
      case 980:error="Incorrect nickname or password.";break;
      case 981:error="The service is temporarily unavailable.";break;
      case 982:error="Your warning level is currently too high to sign on.";break;
      case 983:error="You have been connecting and disconnecting too frequently.\nWait 10 minutes and try again.\n If you continue to try, you will need to wait even longer.";break;
      case 989:error="An unknown signon error has occurred $1";break;
      default:error="Unknown";break;
    }
    int i = error.indexOf("$1");
    if ( i!=-1 ) {
      error = error.substring(0,i);
      error+= var;
      error+= error.substring(i+2);
    }
    System.out.println(error);
    System.exit(0);
  }

  /* Used by onEvent */
  protected String normalize(String name) {
    String rtn="";
    for ( int i=0;i<name.length();i++ ) {
      if ( name.charAt(i)==' ' )
        continue;
      rtn+=Character.toLowerCase(name.charAt(i));
    }
    return rtn;
  }

  /* KNEventListener */
  public void onEvent(KNEvent anEvent) {
    String from = anEvent.get("displayname");
    if (from == null) { from = anEvent.get("userid"); }
    String message = anEvent.getPayload();
    message = message.substring(0, message.length()-1);  // removes \n
    if (from == null) {
      // Don't print who it's from if there's no displayname or userid
      String message3 = messageHead + message;
      System.out.println("Received: " + message);
      myJavaTOC.send(myAIMHandle, message3);
    } else { 
      // Print who it's from if there's a displayname or userid
      String message2 = "[" + from + "] " + message;
      String message3 = messageHead + "<b>" + from + "</b> says: " + message;
      String fromNorm = normalize(from);
      String myAIMHandleNorm = normalize(myAIMHandle);
      // This suppresses duplicates so I don't receive what I send.
      if (! fromNorm.equals(myAIMHandleNorm)) {
        System.out.println("Received: " + message2);
        myJavaTOC.send(myAIMHandle, message3);
      }
    }
  }


  /* main lobster */

  public static void main(String[] args) throws IOException {

    System.out.println("Welcome to MAIM!  You can set these system properties in the commandline\nby running java with a -D option:\n");
    System.out.println("   -Drouter=\"where the KnowNow router is\"");
    System.out.println("   -Dtopic=\"topic on that router to publish & subscribe to\"");
    System.out.println("   -Daimhandle=\"the AIM account to send messages to\"");
    System.out.println("   -Dmessagehead=\"prefix for any line sent to that account, for example EXCEL SAYS\"");
    System.out.println("   -Dbridgeaimhandle=\"the AIM account that acts as a bridge\"");
    System.out.println("   -Dbridgepassword=\"the password for that account\"\n\n");

    InputStreamReader inputstreamreader = new InputStreamReader (System.in);
    BufferedReader bufferedreader = new BufferedReader (inputstreamreader);

    if (myRouter == null) { myRouter = "http://sales.knownow.com/kn"; }
    if (myTopic == null)  { myTopic = "/what/chat"; }
    if (messageHead == null) { messageHead = ""; }

    if (myAIMHandle == null) { 
      System.err.print("AIM account to route messages to? ");
      myAIMHandle = bufferedreader.readLine(); 
    }

    if (bridgeAIMHandle == null) {
      System.err.println("\nTo use this program, you need to sign up for a bridge AIM account\nthat will send messages to your AIM account.");
      System.err.print("Bridge AIM account handle? ");
      bridgeAIMHandle = bufferedreader.readLine(); 
      if (bridgeAIMHandle.length() < 3) {
        System.err.println("That name is too short.");
        System.exit(0);
      }
    }

    if (bridgePassword == null) {
      System.err.println("\nWARNING: passwords are printed in cleartext.\n");
      System.err.print(bridgeAIMHandle + "'s password? ");
      // Cleartext passwords suck: http://www.itworld.com/nl/java_sec/02082002/
      bridgePassword = bufferedreader.readLine(); 
    }

    KNOptions myOptions = new KNOptions();
    myOptions.add("displayname", bridgeAIMHandle);
    myOptions.add("user_id", bridgeAIMHandle);

    MAIM myAIM = new MAIM();

    if (! myAIM.myJavaTOC.login(bridgeAIMHandle, bridgePassword)) {
      System.err.println("\nFailed to login as " + bridgeAIMHandle + ".");
      myJavaMicroserver.shutdown();
      System.exit(0);
    }

    System.out.println("\n" + bridgeAIMHandle + " has logged in.");
    System.out.println("Subscribing to topic " + myTopic + " on router " + myRouter + " ... \n");

    KNRoute route2Callback = myJavaMicroserver.subscribe(myTopic, myAIM, myOptions, null);

    if (route2Callback.getHttpStatus() >= 300) {
      System.err.println("Failed to create route to Callback, HTTP Error Code " + route2Callback.getHttpStatus());
      myJavaMicroserver.shutdown();
      System.exit(0);
    }

    myAIM.myJavaTOC.send(myAIMHandle, "<font color=\"blue\"><i>Subscribing to topic " + myTopic + " on " + myRouter + " ... ready, fire, AIM!!</i></font>\n\n");

    // Infinitely process incoming events from AOL Instant Messenger
    myAIM.myJavaTOC.processTOCEvents();

    // It never gets to these because it runs in an infinite loop.
    // myJavaMicroserver.shutdown();
    // myAIM.myJavaTOC.logout();

  } // End of main

} // End of MAIM
