/**
 * Copyright 2001-2004 KnowNow, Inc.  All rights reserved.
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
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the KnowNow, Inc., nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @KNOWNOW_LICENSE_END@
 * 
 *
 * $Id: throughput_test.c,v 1.2 2004/04/19 05:39:15 bsittler Exp $
 **/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#define NUM_THREADS 3
#define MAXRETRIES 10

/************************************************************* 
 * Global data and data structures
 *************************************************************/ 

char *subdata[] = {
"http://127.0.0.1/kn/foo/kn_journal?kn_response_format=simple"
};

struct TestData {
        int numEvents;
        char **urls;
        time_t firstEventTime;
        time_t lastEventTime;
        int currentNumEvents;
        int verbose;
};


/************************************************************* 
 * Function Signatures
 *************************************************************/

void *do_subscribe_stuff(void *postfields);
void printUsage( void );
void startProcessingEvents( int numEvents, int numThreads, int size, double delay, int verbose );
size_t EventCountCallback(void *ptr, size_t size, size_t nmemb, void *data);
void geturlsfromfile( char *filename );


/************************************************************* 
 * Function EventCountCallback
 *************************************************************/ 
size_t EventCountCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
        char *tempBuff;
        char *pbuf;
        int nextIndex, numEvents;
        int count;
        int i;
        int verbose;
        int eventsThisCallback;
        struct TestData *perfData;

        perfData = (struct TestData *) data;
        numEvents = perfData->numEvents;
        verbose = perfData->verbose;

        if( verbose != 0 ) {
                printf("\nCallback called\n"); 
                printf( "Thread %d =>Found %d events so far\n",pthread_self(), perfData->currentNumEvents );
                printf( "Stopping at %d events.\n", numEvents );
        }

        tempBuff = (char *)malloc((size * nmemb) + 1);
        if( tempBuff ){
                memcpy( tempBuff, ptr, size*nmemb );
                tempBuff[ size*nmemb ] = '\0';  
        }

        if( verbose > 1 ) {
                printf("Size: %ld\nData:\n%s\n", (size * nmemb), tempBuff ); 
                printf("Size: %ld\n", (size * nmemb) ); 
        }

        pbuf = tempBuff;
        
        eventsThisCallback = 0;

        while( pbuf != NULL ) {
                pbuf = (char *)strstr( pbuf, "kn_perfzzyyxx" );
                if ( pbuf != NULL ){
                        pbuf++;
                        eventsThisCallback++;
                        perfData->currentNumEvents = perfData->currentNumEvents + 1;
                        if( perfData->currentNumEvents >= numEvents ) {
                                printf( "\n**************************************\n" );
                                printf( "Thread %d => Stopping here.\n", pthread_self() );
                                printf( "Thread %d => Found %d events.\n", pthread_self(), perfData->currentNumEvents );
                                printf( "**************************************\n" );
                                return 0;
                        }
                }
        }

        if( verbose != 0 ) {
                printf( "Thread %d => Found %d events this callback.\n", pthread_self(), eventsThisCallback );
                printf( "Thread %d => Found %d events total\n",pthread_self(), perfData->currentNumEvents );
        }

        free( tempBuff );       
        return size * nmemb;
}

/************************************************************* 
 * Function startProcessingEvents
 *************************************************************/
void startProcessingEvents( int numEvents, int numThreads, int size, double delay, int verbose )
{
        pthread_t *subtid;
        int i;
        struct TestData testData[1024];

        subtid = (pthread_t *)malloc( numThreads * sizeof( pthread_t ) );
        
        i = 0;

        for( i = 0; i < numThreads; i++ ) {
                testData[i].numEvents = numEvents;
                testData[i].urls = subdata;
                testData[i].verbose = verbose;


                /* printf( "Creating thread %d\n", i ); */
                pthread_create(&subtid[i], NULL, do_subscribe_stuff, (void *)&testData[i]);
        }

        sleep(2);
        
        i = 0;
        for( i = 0; i < numThreads; i++ ) {
                pthread_join(subtid[i], NULL);
        }

        free( subtid );
}

/*************************************************************
 * Function printUsage
 *************************************************************/ 
void printUsage(void) 
{
        printf("Usage: throughput_sub -n numEvents [-d delay] [-t numThreads] [-s size]\n" );
}

/************************************************************* 
 * Function do_subscribe_stuff
 *************************************************************/ 
void *do_subscribe_stuff(void *testData)
{
        CURL *curl_handle;
        CURLcode res;
        long int status = 0;
        FILE *file;
        int count;
        int numRetries;
        struct TestData *td;
        td = (struct TestData *)testData;       
        
        if( td->verbose != 0 ){
                printf( "Listening for %d events.\n", td->numEvents );
        }

        /* Initialize current count to 0 */
        td->currentNumEvents = 0;

        /* init the curl session */
        curl_handle = curl_easy_init();

        printf ("Started thread %d\n", pthread_self());
        count = 0;
        if(curl_handle) {
            /* specify URL to get */
            curl_easy_setopt(curl_handle, CURLOPT_URL, subdata[0]);
            curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0);
            curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
            curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "PubSub Throughput 1.0");
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, EventCountCallback);
                curl_easy_setopt(curl_handle, CURLOPT_FILE, (void *)td );
        #ifdef DEBUG
            curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1);
        #else
            curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);
            curl_easy_setopt(curl_handle, CURLOPT_MUTE, 1);
        #endif

        numRetries = 0;
        while( numRetries < MAXRETRIES ) {
                        res = curl_easy_perform(curl_handle);
                        if(res==0 ) {
                                printf("Zero response received");       
                                curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CODE, &status);
                                break;
                        }  else {
                                printf("Non-zero response received.\n");
                                if( td->currentNumEvents == 0 ) {
                                        numRetries++;
                                        printf( "No events received.  Retrying on the same connection.\n" );
                                } else {
                                        printf( "Cleaning up the connection\n" );
                                        break;
                                }
                        }
        }
 } 
  curl_easy_cleanup(curl_handle);
  return 0;
}

/************************************************************* 
 * Main Function
 *************************************************************/ 
int main(int argc, char **argv)
{
        int i;
        int numEvents=0;
        int numThreads = 1;
        int size = 32;
        double delay = 0.0;
        int verbose = 0;
        char *outputFile=NULL;
        char *inputFile=NULL;
        char **urls;

        for( i=1; i < argc; i++ ) {
                printf( "i is %d, argv[i] is %s\n", i, argv[i] );
                if( strcmp( argv[i], "-n" ) == 0 ){
                        if( i == argc - 1 ) {
                                printUsage();
                                exit(1);
                        } else {
                                i++;
                                numEvents = atoi( argv[i] );            
                        }
                } else if ( strcmp( argv[i], "-t" ) == 0 ){
                        if( i == argc - 1 ) {
                                printUsage();
                        } else {
                                i++;
                                numThreads = atoi( argv[i] );
                        }
                } else if ( strcmp( argv[i], "-s" ) == 0) {
                        if( i == argc -1 ) {
                                printUsage();
                        } else {
                                i++;
                                size = atoi( argv[i] );
                        }
                }else if ( strcmp( argv[i], "-d" ) == 0 ) {
                        if( i == argc - 1 ) {
                                printUsage();   
                        } else {
                                i++;
                                printf( "Delay functionality not working yet\n" );
                                /*
                                delay = atof( argv[i] );
                                printf( "Delay value changed to %f\n", delay );
                                */
                        }
                }else if ( strcmp( argv[i], "-o" ) == 0 ) {
                        if( i == argc - 1 ) {
                                printUsage();   
                        } else {
                                i++;
                                outputFile = argv[i];
                        }
                } else if ( strcmp( argv[i], "-v") == 0 ) {
                        if( i == argc -1 ) {
                                printUsage();
                        } else {
                                i++;
                                verbose = atoi( argv[i] );
                        }

                } else if ( strcmp( argv[i], "-i") == 0 ) {
                        if( i == argc -1 ) {
                                printUsage();
                        } else {
                                i++;
                                inputFile =  argv[i];
                        }
                }
        }
        
        if( numEvents == 0 ) {
                printf( "Error: You must specify the number of events to subscribe to with the -n option.\n" );
                printUsage();
        }

        printf( "NumEvents: %d\nThreads: %d\nSize: %d\nDelay: %f\n", numEvents, numThreads, size, delay);
        if( inputFile != NULL ) {
                printf( "Input file: %s\n", inputFile );
                geturlsfromfile( inputFile );
        }
        if( outputFile != NULL ) {
                printf( "Output file: %s\n", outputFile );
        }



        return 0;
}

void geturlsfromfile( char *filename ){
        FILE *fp;

        fp = fopen( (char *)filename, "r" );
        if( fp == NULL ){
                printf( "Error opening file %s\n.Exiting...\n", filename );
        } else {
                printf( "Able to open file: %s\n", filename );
                fclose( fp );
        }
}
