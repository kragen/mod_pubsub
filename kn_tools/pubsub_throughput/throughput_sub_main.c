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
 * $Id: throughput_sub_main.c,v 1.1 2003/04/29 07:31:52 ifindkarma Exp $
 **/

/************************************************************* 
 * File: throughput_sub_main.c
 *
 * Processes input from the user and calls the functions 
 * needed to start the subscription processes
 * 
 *************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include "throughput_sub.h"

char *subdata[] = {
                "http://127.0.0.1/kn/foo/kn_journal?kn_response_format=simple"
};

/************************************************************* 
 * Function throughput_Log
 ************************************************************/
void throughput_Log( struct SharedFile *sharedFile, char *string ){
        pthread_mutex_lock( sharedFile->hMutex );
        fprintf( sharedFile->hFile, "%s", string );
        fflush( sharedFile->hFile );
        pthread_mutex_unlock( sharedFile->hMutex );
}


/************************************************************* 
 * Main Function
 *************************************************************/ 
int main(int argc, char **argv)
{
        /************************************************************* 
         * Common variables to all modes
         *************************************************************/ 
        int i;
        int numEvents=0;
        int numThreads = 1;
        int size = 32;
        double delay = 0.0;
        int verbose = 0;

        /************************************************************* 
        * Sequence mode variables
        *************************************************************/
        int sequenceMode = 0;
        int startseq=-1;
        int endseq=-1;
        char *urlbase;

        /************************************************************* 
         * File mode variables
         *************************************************************/ 
        char *outputFile=NULL;
        char *inputFile=NULL;
        char *inputUrls[MAXURLS];
        char **threadUrls=NULL;

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
                } else if ( strcmp( argv[i], "-seq") == 0 ) {
                        sequenceMode = 1;
                } else if ( strcmp( argv[i], "-startseq") == 0 ) {
                        if( i == argc -1 ) {
                                printUsage();
                        } else {
                                i++;
                                startseq = atoi( argv[i] );
                        }
                } else if ( strcmp( argv[i], "-endseq") == 0 ) {
                        if( i == argc -1 ) {
                                printUsage();
                        } else {
                                i++;
                                endseq = atoi( argv[i] );
                        }
                } else if ( strcmp( argv[i], "-urlbase") == 0 ) {
                        if( i == argc -1 ) {
                                printUsage();
                        } else {
                                i++;
                                urlbase = argv[i];
                        }
                } 
        }

        if( numEvents == 0 ) {
                printf( "Error: You must specify the number of events to subscribe to with the -n option.\n" );
                printUsage();
        }

        printf( "NumEvents: %d\nThreads: %d\nSize: %d\nDelay: %f\n", numEvents, numThreads, size, delay);

        /************************************************************* 
         * If getting data from a file, process the file and set the 
         * array of threads to listen to as appropriate.  If there are
         * no urls listed in the input file, exit with an error.
         *
         * If sequence mode is chosen, do not pay attention to any
         * input file parameters.
         * 
         *************************************************************/ 
        if( sequenceMode == 0 ) {
                if( inputFile != NULL ) {
                        printf( "Input file: %s\n", inputFile );

                        inputUrls[0] = NULL;
                        geturlsfromfile( inputFile, inputUrls );
                        
                        for( i=0; inputUrls[i] != NULL; i++ ){
                                printf("Input url => %s\n", inputUrls[i] );
                        }

                        if( inputUrls[0] == NULL ) {
                                printf( "No urls were listed in input file %s.\nExiting.\n", inputFile );
                                exit(2);
                        } 
                        
                        /************************************************************* 
                        /*      If we are here, the file was valid and some urls were
                         *      specified.  Set threadurls to the data that we got from the
                         *      file
                         *************************************************************/
                        (char *)threadUrls = (char *)inputUrls;

                        /*
                        */

                } else {
                        printf( "No input file specified.\nUsing internal data.\n" );
                        (char *)threadUrls = (char *)subdata; 
                }
                
                if( outputFile != NULL ) {
                        printf( "Output file: %s\n", outputFile );
                }
        } else {
                printf( "Using Sequence Mode\n");
                printf( "Start number: %d\n", startseq );
                printf( "End number: %d\n", endseq );
                printf( "Base url: %s\n", urlbase );
                geturlsfromSequence( urlbase, startseq, endseq, inputUrls );
                for( i = 0; inputUrls[i] != NULL; i++ ) {
                        printf( "Input url %d: => %s\n", i, inputUrls[i] );
                }
                (char *)threadUrls =  (char *)inputUrls;
                /* exit(2); */ 

        }

        startProcessingEvents( numEvents, numThreads, size, delay, verbose, threadUrls, outputFile );
/*
        for( i=0; inputUrls[i] != NULL; i++ ) {
                free( inputUrls[i] );
        }
*/
        return 0;
}



void geturlsfromSequence( char* urlbase, int startNum, int endNum, char **urls ){
        int i;
        int index;
        char tmpNum[10];
        char *inputurl;

        if( (endNum - startNum) > (MAXURLS - 1) ){
                printf( "Error... too many topics in sequence");
                printf( "Exiting..." );
                exit(2);
        }

        for( i=startNum, index = 0; i < endNum; i++, index++ ) {
                inputurl = (char*)malloc(MAXURLLENGTH);
                if( inputurl == NULL ) {
                        printf( "Internal Error...\nExiting.\n");
                        exit(2);
                }
                strcpy( inputurl, urlbase );
                sprintf( tmpNum, "%d", i );
                strcat( inputurl, tmpNum );
                strcat( inputurl, "/kn_journal?kn_response_format=simple");
                /* printf( "adding url: %s\n", inputurl ); */
                urls[index] = inputurl;
                urls[index + 1] = NULL;

        }
        
}


void geturlsfromfile( char *filename, char **urls ){
        FILE *fp;
        int numurls;
        char *inputurl;
        int i;
        char *tmp;
        char *purl;

        fp = fopen( (char *)filename, "r" );
        if( fp == NULL ){
                numurls = 0;
                printf( "Error opening file %s\nExiting...\n", filename );
                exit(2);
        } else {
                printf( "Able to open input url file: %s\n", filename );
                numurls = 0;
                while( 1 ){
                        inputurl = (char*)malloc(MAXURLLENGTH);
                        if( inputurl == NULL ) {
                                printf( "Internal Error...\nExiting.\n");
                                exit(2);
                        }                       
                        tmp = fgets( inputurl, MAXURLLENGTH-1, fp );
                        if( tmp == NULL ){
                                /* Got to the end of the file */
                                free( inputurl );
                                break;
                        }       
                        if( numurls >= MAXURLS-1 ) {
                                printf("Error... Too many urls listed in iput url file %s\n", filename );
                                printf("Exiting\n" );
                                exit(2);
                        } else {
                                purl = (char *)strchr( inputurl, '\n' );
                                if( purl != NULL ){
                                        *purl = '\0';
                                }

                                /************************************************************* 
                                 * Set last member of the array to NULL
                                 *************************************************************/ 
                                urls[numurls] = inputurl;
                                numurls++;
                                urls[numurls] = NULL;
                        }
                }

                fclose( fp );
        }
}




