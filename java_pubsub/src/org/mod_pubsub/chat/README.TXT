This is a simple mod-pubsub chat client (think html chat1) that is implemented using the wx4j (http://www.wx4j.org) Java GUI toolkit. wx4j is a Java binding for the wxWindows cross-platform C++ library which provides a Java GUI toolkit using native widgets and as of this writing is at version 0.1.0. The downloads can be found at http://www.wx4j.org/downloads/. Note that its only been tested with Windows 2000 and Java 1.4.1_01 at this stage, but it did have a good workout chatting to Joyce one evening!

The ChatApp expects the command line parameters to be the URL of the kn server to subscribe to and secondly the unique user name/client-id you wish to be known as, so something like the following should work:

	java ChatApp http://www.mod-pubsub.org:9000/kn JavaChat


Enjoy!


