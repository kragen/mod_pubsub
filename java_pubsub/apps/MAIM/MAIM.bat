@echo off

rem  Adam Rifkin, adam@knownow.com -- Java AIM Bridge

rem  Note: you must have your CLASSPATH variable set correctly,
rem  and include the Java Microserver in your CLASSPATH.

rem  Useful variables you can set in this .bat file:

rem     router = where the PubSub Server is
rem     topic  = topic on that Server to publish & subscribe to
rem     aimhandle = the AIM account to send messages to
rem     messagehead = prefix for any line sent to that account
rem     bridgeaimhandle = the AIM account that acts as a bridge
rem     bridgepassword = the password for that account

rem  Here's the command line I use:

rem  java -Drouter="http://sales.knownow.com/kn" -Dtopic="/what/chat" -Daimhandle="KnowNowBuddy" -Dmessagehead="<br>" -Dbridgeaimhandle="KnowNowAIM" MAIM

java -Drouter="http://sales.knownow.com/kn" -Dtopic="/exceldemo" -Daimhandle="KnowNowBuddy" -Dmessagehead="<br>West Leads are now: " -Dbridgeaimhandle="KnowNowAIM" MAIM



