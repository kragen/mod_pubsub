/**
 * Copyright 2000-2004 KnowNow, Inc.  All rights reserved.
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
 **/

/* $Id: evdispatch.c,v 1.2 2004/04/19 05:39:15 bsittler Exp $ */

#include "evdispatch.h"
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

/*****************************************************************
 * Data structures for event dispatch
 * Linked list headers for channels in various states 
 */

static ev_channel active_channels; /* Channels with actual queued data */
static ev_channel idle_channels;   /* Channels open, with nothing to send */
static ev_channel pending_channels; /* Channels with data being queued */

/* Array saying which queue is for channels in which state */

static ev_channel *state_channel[CHAN_STATE_MAX] = {
    NULL,			/* CHAN_NEW */
    &active_channels,		/* CHAN_SENDING_DATA */
    &pending_channels,		/* CHAN_PENDING */
    &idle_channels		/* CHAN_IDLE */
    };

/* We use dummy channel structures as linked list heads ...
 * marked as dummies by -1 in fd
 */

static inline void init_chan_list (ev_channel *list_head)
    {
    list_head->next = list_head;
    list_head->prev = list_head;
    list_head->fd = -1;
    }

static inline void unlink_channel (ev_channel *chan)
    {
    chan->prev->next = chan->next;
    chan->next->prev = chan->prev;
    }

static inline void link_channel (ev_channel *head, ev_channel *chan)
    {
    chan->next = head;
    chan->prev = head->prev;
    head->prev->next = chan;
    head->prev = chan;
    }

/* How to initialize all our list headers */

static void
ev_list_init (void)
    {
    init_chan_list (&active_channels);
    init_chan_list (&pending_channels);
    init_chan_list (&idle_channels);
    }

/* Channel state manipulation function.
 * This is the only way that chan->state should ever be set, aside from
 * initial placement on the idle list on creation.  It maintains the idle
 * and active lists, and chan->last_data_sent.
 */

static void
set_channel_state (ev_channel *chan, ev_chan_state state)
    {
    ev_channel *from_queue = state_channel[chan->state];
    ev_channel *to_queue = state_channel[state];
	
    if (from_queue != to_queue)
	{
	if (from_queue != NULL) unlink_channel (chan);
	
	link_channel (to_queue, chan);

	if (to_queue == &idle_channels)
	    chan->last_data_sent = time(NULL); /* XX lotta syscalls here... */
	}
    
    chan->state = state;
    }

/* Iterating over channels in a given state.  Note that we try
 * not to blow up if the channels we are processing change state or
 * vanish out from under us
 */

ev_channel *
first_channel (ev_chan_state state, ev_channel_iter *iter)
    {
    ev_channel *head = state_channel[state];
    if (head->next == head) return NULL;
    
    iter->list_head = head;
    iter->next = head->next->next;
    return head->next;
    }

ev_channel *
next_channel (ev_channel_iter *iter)
    {
    ev_channel *next = iter->next;
	
    if (next == iter->list_head)
	return NULL;

    iter->next = next->next;
    return next;
    }

/*****************************************************************
 * Hash table
 */

static ev_channel *hash_buckets[EV_HASH_BUCKETS];

static int
hash (char *topic_name)
    {
    /* Tcl hash function; assumes that we have long strings of
     * digits in there someplace
     */

    unsigned int hash = 0;

    while (*topic_name != '\0')
	{
	int c = *topic_name++;
	hash = hash*9 + c;
	}

    return hash % EV_HASH_BUCKETS;
    }

/* Iterating through channels subscribed to a given topic */

ev_channel *
topic_chan_first (char *topic)
    {
    ev_channel *chan = hash_buckets[hash(topic)];
    
    while (chan != NULL && strcmp (chan->topic, topic) != 0)
	chan = chan->h_next;

    return chan;
    }

ev_channel *
topic_chan_next (char *topic, ev_channel *chan)
    {
    chan = chan->h_next;
	
    while (chan != NULL && strcmp (chan->topic, topic) != 0)
	chan = chan->h_next;

    return chan;
    }

/* Linking channels into the hash table, and tossing them out */

static void
put_channel_in_hash (ev_channel *chan)
    {
    int hash_code = hash (chan->topic);
    ev_channel *old_bucket_head = hash_buckets[hash_code];

    if (old_bucket_head == NULL)
	{
	hash_buckets[hash_code] = chan;
	chan->h_next = chan->h_prev = NULL;
	}
    else
	{
	old_bucket_head->h_prev = chan;
	chan->h_prev = NULL;
	chan->h_next = old_bucket_head;
	hash_buckets[hash_code] = chan;
	}
    }

static void
evict_channel_from_hash (ev_channel *chan)
    {
    int hash_code = hash (chan->topic);

    if (hash_buckets[hash_code] == chan)
        hash_buckets[hash_code] = chan->h_next;
      
    if (chan->h_prev != NULL)
	chan->h_prev->h_next = chan->h_next;

    if (chan->h_next != NULL)
	chan->h_next->h_prev = chan->h_prev;
    }

/* Initializing the hash table */

static void
hash_init(void)
    {
    int i;

    for (i = 0; i < EV_HASH_BUCKETS; ++i)
	hash_buckets[i] = NULL;
    }

/*****************************************************************
 * Initialize all data structures
 */

void
ev_data_init (void)
    {
    ev_list_init();
    hash_init();
    }

/*****************************************************************
 * Low level I/O to channels
 */

#ifndef GLIBC_MEMSTREAMS
#include "mem_stream.c"
#endif

ev_block_stream *ready_channel (ev_channel *chan)
    {
    if (chan->datastream == NULL)
	{
	chan->datastream = open_ev_block (&chan->streamstate);
	set_channel_state (chan, CHAN_PENDING);
	}

    return chan->datastream;
    }

void channel_data_complete (ev_channel *chan)
    {
    /* Have all data for this event block.  Wrap it up ... */
	
    ev_chan_block *header =
	close_ev_block (chan->datastream, &chan->streamstate);
	
    chan->datastream = NULL;

    /* ... and queue it */

    if (chan->cur_data == NULL)
	{
	chan->cur_data = chan->last_data = header;
	chan->bytes_left = header->length;
	}
    else
	{
	chan->last_data->next = header;
	chan->last_data = header;
	}

    /* Lastly, mark this channel active */

    set_channel_state (chan, CHAN_SENDING_DATA);
    }

/* More block manipulation at this level, for write support:
 * flushing a data block after we are done writing it.  The I/O
 * itself is below, because we may have to nuke the channel if we
 * got an error trying to write, and those functions haven't been
 * defined yet.  (Look, ma, I'm being literate!)
 */

static void done_with_ev_block (ev_channel *chan)
    {
    ev_chan_block *flushed_data = chan->cur_data;

    /* Drop the data block */
    
    chan->cur_data = flushed_data->next;
    flushed_data->next = NULL;
    flushed_data->data = NULL;
    free (flushed_data);

    /* Set up next data block, if we have one.  Otherwise, 
     * idle the channel.
     */

    if (chan->cur_data != NULL)
	chan->bytes_left = chan->cur_data->length;
    else
	set_channel_state (chan, CHAN_IDLE);
    }

/*****************************************************************
 * Creating and destroying channels
 */

ev_channel *create_channel (int fd, char *topic)
    {
    ev_channel *chan = (ev_channel *)malloc (sizeof(ev_channel));
    /* Check for ENOMEM? */

    chan->fd = fd;
    chan->topic = strdup (topic);
    chan->state = CHAN_NEW;
    chan->cur_data = chan->last_data = NULL;
    chan->datastream = NULL;
    chan->fmt_data = NULL;	/* may be reset by caller */

    put_channel_in_hash (chan);

    return chan;
    }

void destroy_channel (ev_channel *chan)
    {
    /* Ditch all pending data */

    if (chan->datastream != NULL)
	free (close_ev_block (chan->datastream, &chan->streamstate));
	
    while (chan->cur_data)
	done_with_ev_block (chan);

    /* Get the channel off linked lists */

    unlink_channel (chan);

    /* Get it out of the hash table */

    evict_channel_from_hash (chan);

    /* Ditch other ancillary stuff associated with the channel,
     * other than formatting data, which we assume has already been
     * ditched by the caller.
     */

    assert (chan->fmt_data == NULL);
    free (chan->topic);

    /* Close the socket */

    close (chan->fd);
    
    /* Ditch the channel itself */
    
    free(chan);
    }

/*****************************************************************
 *
 * Try actually writing some data to a channel's client.
 */

int
try_sending_on_channel (ev_channel *chan)
    {
    /* We could try to write multiple pending blocks with a writev(),
     * if we have them, but the way we're setting up, that's not likely.
     * Don't bother, for now.
     */
    
    while (chan->cur_data != NULL)
	{
	ev_chan_block *buf = chan->cur_data;
	char *buf_start = buf->data + buf->length - chan->bytes_left;
	int rv = write (chan->fd, buf_start, chan->bytes_left);

	if (rv == chan->bytes_left)
	    done_with_ev_block (chan);
	else if (rv > 0)
	    {
	    /* Partial write.  Try again... */
	    chan->bytes_left -= rv;
	    }
	else if (rv == 0)
	    {
	    /* Couldn't write.  Done */
	    break;
	    }
	else 
	    {
	    /* rv < 0.  Either we would block trying to send, or we
	     * have a real error.  If the latter, return error.
	     */
		
	    if (errno != EWOULDBLOCK && errno != EAGAIN)
		return -1;

	    break;
	    }
	}
    
    /* Nothing blew up.  Tell caller if there's anything left to send */
    
    return (chan->cur_data == NULL);
    }
