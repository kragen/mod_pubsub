/**
 * Copyright (c) 2001-2003 KnowNow, Inc.  All rights reserved.
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
 *
 * $Id: throughput_sub.c,v 1.1 2003/04/29 07:31:52 ifindkarma Exp $
 **/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include "throughput_sub.h"

double gettimeofday_double() {
        struct timeval now_t;
        gettimeofday( &now_t, NULL );
        return now_t.tv_sec + (now_t.tv_usec/1e6);
}


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
        double elapsedTime;
        struct TestData *perfData;
        char   printbuf[MAXCARRYOVERBUFFERSZ];
        unsigned long lastFoundAddr;
        unsigned long szBufferProcessed;

        perfData = (struct TestData *) data;
        numEvents = perfData->numEvents;
        verbose = perfData->verbose;

        if( verbose != 0 ) {
            printf("%s:%d|Thread %d|have %d events, looking for %d events, got some more....\n",
                    __FILE__, 
                    __LINE__,
                    pthread_self(),
                    perfData->currentNumEvents,
                    numEvents);
        }

        tempBuff = (char *)malloc((size * nmemb) + perfData->bytesInBuffer + 1);

        if( verbose>3 ) {
          memcpy(tempBuff, ptr, size*nmemb);
          tempBuff[size*nmemb] = '\0';
          printf("%s:%d|Thread %d|from Curl (w/ocarryover)|Size: %ld|\tData:|%s|\n", 
                 __FILE__,
                 __LINE__,
                 pthread_self(),
                 (size * nmemb), 
                 tempBuff );
        } /* paranoia - see what we got before we merge */

        /*
         *  Merge w/ the carry over from the previous test
         */
        if( tempBuff ){
                if(perfData->bytesInBuffer>0)
                       memcpy( tempBuff, perfData->carryOverBuffer, perfData->bytesInBuffer);
                memcpy( (void *)((unsigned long)tempBuff + perfData->bytesInBuffer), ptr, size*nmemb );
                tempBuff[ (size*nmemb) + perfData->bytesInBuffer ] = '\0';      
        } /*ok, merge carry over and what we got */

        if( verbose > 1 ) {
                printf("%s:%d|Thread %d|Size:%ld|Data:|%s|\n", 
                       __FILE__,
                       __LINE__,
                       pthread_self(),
                       (size * nmemb)+perfData->bytesInBuffer, 
                       tempBuff ); 
                /* printf("Size: %ld\n", (size * nmemb) );  */
        }

        pbuf = tempBuff;
        
        eventsThisCallback = 0;

        /*
         *  Previously the logic commented out below was used.  But, tests showed that
         * we would sometimes not see the status events.  So, rely on this.
         *
         *  If the callback is called, then the tunnel has to be alive and we are in the process
         * of processing data.  So, go for it!
         */
        if( 0 == perfData->statusEventRecd )
        {
                perfData->statusEventRecd = 1;
                        
                printf("%s:%d|Thread %d|Status Event Recd - Tunnel is alive\n", 
                       __FILE__,
                       __LINE__,
                       pthread_self() ); 
                fflush(stdout);
                fflush(stderr);
        }
        /************************************************************************
         *  First look for the status event that tells us that the tunnel to the 
         * journal has been opened.  Only, then are we ready to recv events.  
         * This is a big deal since if this not open and the publisher started, 
         * then we missed a bunch of events.  Now, this thread will never stop 
         * since it will be waiting to recv the specific number of events!
        if( 0 == perfData->statusEventRecd )
        {
            pbuf = (char *)strstr(pbuf, "status: 200=20Watching=20topic");
            if (pbuf)
            {
                perfData->statusEventRecd = 1;
                                
                printf("%s:%d|Thread %d|Status Event Recd - Tunnel is alive\n", 
                       __FILE__,
                       __LINE__,
                       pthread_self() ); 
                fflush(stdout);
                fflush(stderr);
            }
        }
        ********************************************************************/

        /*
         *  Set the lastFoundAddr to where the tempbuff is in case this
         * buffer doesn't have the kn_perfzzyyxx string we are looking for
         */
        lastFoundAddr=(unsigned long)tempBuff;
        while( pbuf != NULL ) {
                pbuf = (char *)strstr( pbuf, "kn_perfzzyyxx" );
                if ( pbuf != NULL ){
                        /***************************************************** 
                         * An event was found.  If it was the first event, 
                         * record the time.
                         *****************************************************/ 
                        if( perfData->currentNumEvents == 0 ) {
                                perfData->firstEventTime = gettimeofday_double();
                                printf("%s:%d|Thread %d|First event recd....\n",
                                        __FILE__, 
                                        __LINE__,
                                        pthread_self() );
                        }

                        pbuf+= 13; /*skip beyond the kn_perfzzyyxx header */
                        lastFoundAddr=(unsigned long)pbuf;

                        eventsThisCallback++;
                        perfData->currentNumEvents = perfData->currentNumEvents + 1;
                        if( perfData->currentNumEvents % 10 == 0 ) {
                                printf("Thread %d => Received %d events to date\n", 
                                       pthread_self(), 
                                       perfData->currentNumEvents );
                                fflush(stdout);
                                fflush(stderr);
                        }
                        if( perfData->currentNumEvents >= numEvents ) {
                                /************************************************* 
                                 * Found all of the events we are looking for.  
                                 * Save the time, print a message and stop 
                                 * receiving data from the connection.
                                 ************************************************/ 
                                perfData->lastEventTime = gettimeofday_double();
                                elapsedTime = perfData->lastEventTime - 
                                                  perfData->firstEventTime;
                                perfData->elapsedTime = elapsedTime;

                                printf("%s:%d|Thread %d|Found %d events - elapsed time %f....\n",
                                        __FILE__, 
                                       __LINE__,
                                        pthread_self(),
                                        perfData->currentNumEvents,
                                        elapsedTime);

                                return 0;
                        } /* if currentNumEvents >= numEvents */
                }/* if pbuf != NULL */
        } /* while pbuf */

        if( verbose != 0 ) {
                printf( "Thread %d => Found %d events this callback.\n", 
                        pthread_self(), 
                        eventsThisCallback );
                printf( "Thread %d => Found %d events total\n",
                        pthread_self(), 
                        perfData->currentNumEvents );
        }

        szBufferProcessed = lastFoundAddr -  ((unsigned long)tempBuff);
        perfData->bytesInBuffer = ( (size * nmemb ) + perfData->bytesInBuffer) - szBufferProcessed;

        if(perfData->bytesInBuffer < MAXCARRYOVERBUFFERSZ) {
          memcpy(perfData->carryOverBuffer, (void *)lastFoundAddr, perfData->bytesInBuffer);

          if (verbose > 3 && (perfData->bytesInBuffer>0) ) {
            memset(printbuf, 0, MAXCARRYOVERBUFFERSZ-1);
            if (perfData->bytesInBuffer < MAXCARRYOVERBUFFERSZ) {
              memcpy(printbuf, (void *)lastFoundAddr, perfData->bytesInBuffer);
          
              printf("%s:%d|Thread %d =>carryover|Size:%ld|Data:%s|\n",
                     __FILE__, 
                     __LINE__,
                     pthread_self(), perfData->bytesInBuffer, printbuf);
            }
            else {
              printf("%s:%d|Thread %d =>carryover too large %ld\n",
                     __FILE__, 
                     __LINE__,
                     pthread_self(), perfData->bytesInBuffer);
            }
          } /* verbose >3 and there is stuff to carry over */
        } /* bytesInBuffer < MAXBUFSZ */
        else {
          printf("%s:%d|Thread %d =>carryover too large %ld\n",
                 __FILE__, 
                 __LINE__,
                 pthread_self(), perfData->bytesInBuffer);
        }


        free( tempBuff );       
        return size * nmemb;
}

/************************************************************* 
 * Function startProcessingEvents
 *************************************************************/
void startProcessingEvents( int numEvents, int numThreads, int size, 
                            double delay, int verbose, char **urls, 
                            char *outputfile )
{
        pthread_t *subtid;
        int i, ret ;
        int numurls=0;
        int urlindex;
        struct TestData testData[1024];
        char *url;
        FILE *fp;
        char outputLine[1024];

        subtid = (pthread_t *)malloc( numThreads * sizeof( pthread_t ) );
        
        i = 0;
        
        /* numurls = strlen((char *)urls);*/
        numurls = 0;
        while( urls[numurls] != NULL ){
                        numurls++;
        }
        printf( "Found %d urls.\n", numurls );
        fflush(stdout);
        fflush(stderr);
        
        if( numurls == 0 ) {
                printf( "Error... No urls supplied.\nExiting.");
                exit(2);
        }
        
        for( i = 0; i < numThreads; i++ ) {
                if( numurls == 1 || i == 0 ){ 
                        urlindex = 0;
                } else {
                        urlindex =  i % numurls;
                }

                testData[i].numEvents = numEvents;
                testData[i].verbose = verbose;
                testData[i].url = urls[urlindex];
                testData[i].statusEventRecd = 0;
                testData[i].bytesInBuffer=0;



                ret = pthread_create(&subtid[i], NULL, do_subscribe_stuff, 
                                     (void *)&testData[i]);
                if ( 0 != ret ) {
                    printf("************#################******\n");
                    printf("Error cannot create thread #%d\n", i);
                }

                while ( testData[i].statusEventRecd == 0) {
                        if (verbose > 2)
                          printf( "%s:%d|Created thread %d|waiting for status event\n", 
                                  __FILE__, __LINE__, i );
                        sleep(1);
                }
                /*
                 *  Windows seems to have a limit on how many requests
                 * you can make to that PubSub Server in a short burst.  This is
                 * an attempt to circumvent the problem
                 */
                if (i % 10 == 0) {
                        printf ("created i threads\n", i);
                        fflush(stdout);
                        fflush(stderr);
                }
        } /* for loop */

        for( i = 0; i < numThreads; ) {
                if ( testData[i].statusEventRecd == 0) {
                        /* sleep(5); */
                        if (verbose)
                                printf("Waiting for thread %d to get status event\n", i);
                }
                else
                        i++;
        }

        printf("All Subscribers connected\n");
        fflush(stdout);
        fflush(stderr);
        /* sleep(2); */
        
        i = 0;
        for( i = 0; i < numThreads; i++ ) {
                pthread_join(subtid[i], NULL);
        }

        if( outputfile != NULL ){
                fp = fopen( (char *)outputfile, "w" );
                if( fp == NULL ){
                        for( i = 0; i < 3; i++ ) {
                                printf( "\n********************************\n");
                        }
                        printf( "Error... Could not open output file\n");
                                                
                        for( i = 0; i < 3; i++ ) {
                                printf( "\n********************************\n");
                        }
                        exit(2);
                } else {
                        for( i = 0; i < numThreads; i++ ) {
                                fprintf( fp, "%d,%f,%f,%f\n", 
                                        testData[i].numEvents,
                                        testData[i].firstEventTime,
                                        testData[i].lastEventTime,
                                        testData[i].elapsedTime );
                                /* fprintf( fp, outputLine ); */
                        }
                        
                        fclose( fp );
                }
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
        double currEventTime;
        
        if( td->verbose != 0 ){
                printf ("%s:%d|Thread %d|Lookng for %d events|Listening on url: %s\n", 
                        __FILE__,
                        __LINE__, 
                        pthread_self(),
                        td->numEvents,
                        td->url );
        }

        /* Initialize current count to 0 */
        td->currentNumEvents = 0;

        /* init the curl session */
        curl_handle = curl_easy_init();

        count = 0;
        if(curl_handle) {
            /* specify URL to get */
            curl_easy_setopt(curl_handle, CURLOPT_URL, td->url );
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
                        if( td->verbose != 0 ){
                                currEventTime = gettimeofday_double();
                                printf ("%s:%d|Thread %d|Opening tunnels|%s|time|%f\n", 
                                        __FILE__,
                                        __LINE__, 
                                        pthread_self(),
                                        td->url,
                                        currEventTime );
                        }

                        /* connect and make tunnel */
                        res = curl_easy_perform(curl_handle);
                        curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CODE, &status);
                        if(td->verbose>3){

                            printf ("%s:%d|Thread %d|return code from getinfo|%ld|\n",
                                    __FILE__,
                                    __LINE__,
                                    pthread_self(),
                                        res );
                        }

                        if(res==0 ) {

                            printf ("%s:%d|Thread %d|Zero response recd\n", 
                                    __FILE__,
                                    __LINE__, 
                                    pthread_self() );
                                break;

                        }  else {

                                /* printf("Non-zero response received.\n"); */
                                if( td->currentNumEvents == 0 ) {
                                        numRetries++;
                                        /* printf( "No events received.  Retrying on the same connection.\n" ); */
                                } else {
                
                                    printf ("%s:%d|Thread %d|HTTP Code %d|Cleaning up the connection\n", 
                                            __FILE__,
                                            __LINE__, 
                                            pthread_self(),
                                            status );

                                        break;
                                }
                        }
                        /* back off if tunnel creation didn't succeed */
                        sleep(10);
                } /* while not MAXRETRIES */
        }  /* if we got a curl handle */
        else {
                currEventTime = gettimeofday_double();
                printf ("%s:%d|Thread %d|Could NOT open tunnel|%s|time|%f\n", 
                        __FILE__,
                        __LINE__, 
                        pthread_self(),
                        td->url,
                        currEventTime );
        } /* we didn't get a handl */

  curl_easy_cleanup(curl_handle);
  fflush(stdout);
  fflush(stderr);
  return 0;
}



