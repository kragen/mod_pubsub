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
 * $Id: throughput_pub.c,v 1.2 2004/04/19 05:39:15 bsittler Exp $
 **/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#define NUM_THREADS 3

char *urls[] = {
"http://127.0.0.1/kn/foo/kn_journal?do_method=notify&kn_payload=Hellothere",
"http://127.0.0.1/kn?kn_response_format=simple&do_method=route&kn_id=0011&do_max_age=infinity&kn_to=/kn/curl_jn/kn_journal&kn_from=/kn/curl",
"http://127.0.0.1/kn?kn_response_format=simple&do_method=notify&kn_id=2&kn_payload=get payload&kn_to=/kn/curl"
};

char *postdata[] = {
"kn_response_format=simple&do_method=notify&kn_id=3&kn_payload=post payload&kn_to=/kn/curl",
"kn_response_format=simple&do_method=route&kn_id=0022&do_max_age=infinity&kn_to=/kn/curl_jn/kn_journal&kn_from=/kn/curl",
"kn_response_format=simple&do_method=notify&kn_id=4&kn_payload=post payload&kn_to=/kn/curl"
};


/*
 * this function is used to write to null stream from ptr 
 * basically ignore the input stream 
 */
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  register int written;
  written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}



void *do_get_curl_stuff(void *url);
void *do_post_curl_stuff(void *postfields);

int main(int argc, char **argv)
{
  int i;
  pthread_t tgid[NUM_THREADS], tpid[NUM_THREADS], subtid[NUM_THREADS];

  for ( i = 0; i < NUM_THREADS; i++) {
    pthread_create(&tgid[i], NULL, do_get_curl_stuff, urls[i]);
   /* pthread_create(&tpid[i], NULL, do_post_curl_stuff, postdata[i]); */
  }

  for ( i = 0; i < NUM_THREADS; i++) {
    pthread_join(tgid[i], NULL);
    /* pthread_join(tpid[i], NULL); */
  }

  return 0;
}


void *do_get_curl_stuff(void *url)
{
  char *response;
  CURL *curl_handle;
  CURLcode res;
  long int status = 0;
  int j = 0;

  /* init the curl session */
  curl_handle = curl_easy_init();

  printf ("GET %s\n", curl_version());

  if(curl_handle) {
    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "PubSub Throughput 1.0");
#ifdef DDEBUG
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1);
#else
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curl_handle, CURLOPT_MUTE, 1);
#endif

   while ( j < 5000 ) {
    printf("\nthread %d\n", pthread_self());
    j++;

    /* get it! */
    res = curl_easy_perform(curl_handle);
    if(res==0) {
        printf( "Received zero response" );
      /* curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CODE, &status); */
      /* printf("Status Code : %ld\n%s", status, response); */
    } else {
        printf("Received non-zero response");
      /* curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CODE, &status); */
      /* printf("Status Code : %ld\n%s", status, response); */
        } 
   }
  }
  curl_easy_cleanup(curl_handle);
  return 0;
}

void *do_post_curl_stuff(void *postfields)
{
  char *response, url[] = "http://127.0.0.1/kn";
  CURL *curl_handle;
  CURLcode res;
  long int status = 0;
  int j = 0;

  /* init the curl session */
  curl_handle = curl_easy_init();

  printf ("POST %s\n", curl_version());

  if(curl_handle) {

    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postfields);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "PubSub Throughput 1.0");
#ifdef DDEBUG
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1);
#else
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curl_handle, CURLOPT_MUTE, 1);
#endif

   while ( j < 5000 ) {
    printf("\nthread %d\n", pthread_self());
    j++;

    /* get it! */
    res = curl_easy_perform(curl_handle);
    if(res==0) {
      curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CODE, &status);
      printf("Status Code : %ld\n%s", status, response); 
    } else {
                printf("Received non-zero response");
      curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CODE, &status);
      printf("Status Code : %ld\n%s", status, response); 
         }      
   }
  }
  curl_easy_cleanup(curl_handle);
  return 0;
}

