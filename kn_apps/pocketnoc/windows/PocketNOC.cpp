/**
 * PocketNOC - v1.2
 * author: Paul Sharpe
 *
 * PocketNOC.cpp
 *
 * Includes the main function for the PocketNOC program as well as
 * all the PubSub functionality.
 */

/*
Copyright (c) 2000-2003 KnowNow, Inc.  All rights reserved.

@KNOWNOW_LICENSE_START@

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.

3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
be used to endorse or promote any product without prior written
permission from KnowNow, Inc.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@KNOWNOW_LICENSE_END@
*/

#include "PocketNOC.h"


/**
 * Prints an application usage message to the console.
 */

void PrintUsage(void)
{
	printf("%s v%s\n", APP_NAME, APP_VERSION);
	printf("usage: %s options\n", APP_NAME);
	printf("  required:\n");
	printf("    -server|--server  server hostname\n");
	printf("    -port|--port      server port\n");
	printf("    -path|--path      path to router\n");
	printf("  optional:\n");
	printf("    -user|--user  username\n");
	printf("    -pass|--pass  password\n");
	printf("    -rate|--rate  update rate in seconds\n");
}


/**
 * Gets the machine's hostname from the winsock information.  Puts
 * the result in the passed string.
 *
 * @param hostname String to store hostname in
 * @param hostlen Length of the passed string
 */

void GetHostname(char *hostname, int hostlen)
{
	// winsock data object

	WSADATA wsaData;

	strcpy(hostname, "");


	// Open winsock

	if(WSAStartup(MAKEWORD(1,1), &wsaData) != 0)
		return;


	// Copy hostname into the passed string

	gethostname(hostname, hostlen);


	// Close up winsock

	WSACleanup();
}


/**
 * Parses through passed arguments and stores the results in the
 * corresponding variables in the passed settings collection.
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @param settings Router settings collection
 */

void ParseArguments(int argc, char *argv[], RouterSettings *settings)
{
	// enum of cases for the switch statement

	enum cases {
		server = 1,
		port,
		path,
		user,
		pass,
		rate
	};


	// ints for getopt

	int options;
	int longind = 0;


	// struct of long possible options for command-line parameters

	struct option longOpts[] =
	{
		{ "server",		required_argument, 0, server	},
		{ "port",		required_argument, 0, port		},
		{ "path",		required_argument, 0, path		},
		{ "user",		required_argument, 0, user		},
		{ "pass",		required_argument, 0, pass		},
		{ "rate",		required_argument, 0, rate		},
		{ 0, 0, 0, 0 }
	};


	// Parse through options passed and match them to the corresponding
	// variables in the settings collection.  If illegal params are passed
	// print a usage message and exit.

	while((options = 
		getopt_long_only(argc, argv, "", longOpts, &longind)) != -1)
	{
		switch(options)
		{
		case server:
			settings->routerServer = optarg;
			break;
		
		case port:
			settings->routerPort = atoi(optarg);
			break;
		
		case path:
			settings->routerPath = optarg;
			break;
		
		case user:
			settings->routerUser = optarg;
			break;
		
		case pass:
			settings->routerPass = optarg;
			break;
		
		case rate:
			settings->updateRate = atoi(optarg);
			break;
		
		default:
			PrintUsage();
			exit(0);
		}
	}


	// If the required settings haven't been set, print usage 
	// message and exit.

	if(settings->routerServer == NULL
		|| settings->routerPort == 0
		|| settings->routerPath == NULL
	) {
		PrintUsage();
		exit(0);
	}
}


/**
 * Closes up HTTP connection on program exit.
 *
 * @param sig Signal passed from the signal() function
 */

static void CleanUp(int sig)
{
	// Close the HTTP connection

	httpHelper->disconnect();

	delete httpHelper;
	delete sysInfo;

	exit(0);
}


void main(int argc, char *argv[])
{
	// Settings collection to store router info.  Setup some
	// default values.

	RouterSettings settings = { NULL, NULL, "", "", 0, 5 };


	// If there are arguments, parse them, otherwise print the
	// usage message and exit.

	if(argc > 1)
		ParseArguments(argc, argv, &settings);		
	else {
		PrintUsage();
		return;
	}


	

	// Catch CTRL-C to clean up the microserver connection.

	if(signal(SIGINT, CleanUp) == SIG_ERR)
		printf("ERROR\n");


	// String to store HTTP POST payload in

	char payload[2048];


	// Size of payload for use in appending text to it

	int len;


	// Get the hostname of the machine.

	char hostname[256];
	GetHostname(hostname, sizeof(hostname));


	// Create a new SysInfo object to collect stats with.

	sysInfo = new CSysInfo();


	// Create a PubSub topic based on the hostname of the machine.

	int topicLen = strlen("/what/pocketnoc/") + strlen(hostname);

	char *topic = new char[topicLen + 1];

	strcpy(topic, "/what/pocketnoc/");
	strcat(topic, hostname);

	// Create HTTP object

	httpHelper = new CHttpHelper(settings, topic);

	httpHelper->connect();

	delete[] topic;


	// Collect system stats and post them.

	while(1) {
		if(sysInfo->GetStats()) {

			// Get CPU usage 

			len = sprintf(payload, "%s", "cpuusage=");
			len += sprintf(payload + len, "%u", sysInfo->GetCPUUsage());
			
			
			// Get total physical memory
		
			len += sprintf(payload + len, "%s", ";memtotal=");
			len += sprintf(payload + len, "%u", sysInfo->GetMemTotal());

	
			// Get available physical memory
		
			len += sprintf(payload + len, "%s", ";memfree=");
			len += sprintf(payload + len, "%u", sysInfo->GetMemAvail());


			// Get total swap memory

			len += sprintf(payload + len, "%s", ";swaptotal=");
			len += sprintf(payload + len, "%u", sysInfo->GetSwapTotal());

		
			// Get available swap memory
	
			len += sprintf(payload + len, "%s", ";swapfree=");
			len += sprintf(payload + len, "%u", sysInfo->GetSwapAvail());

			// Send refresh rate

			len += sprintf(payload + len, "%s", ";rate=");
			len += sprintf(payload + len, "%u", settings.updateRate);

			// Send os type

			len += sprintf(payload + len, "%s", ";os=windows");
			

			// publish stats to topic

			httpHelper->publish(payload);
		}

		// Pause for the defined rate.  Default is 5 seconds.

		Sleep(settings.updateRate * 1000);

	}

}
