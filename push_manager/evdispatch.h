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

/* $Id: evdispatch.h,v 1.2 2004/04/19 05:39:15 bsittler Exp $ */

#ifndef EV_DISPATCH_H
#define EV_DISPATCH_H

#include <time.h>
#include <stdio.h>

/*****************************************************************
 * Blocks of data queued to be sent on a channel --- if data is dynamically
 * allocated, then we assume it immediately follows the block header, so
 * that a single free() will take care of the lot.  See open_ev_block below.
 */

typedef struct ev_chan_block {
    char *data;
    int length;
    struct ev_chan_block *next;
    } ev_chan_block;

#ifndef GLIBC_MEMSTREAMS

#define INITIAL_BUF_SIZE 500
#define MIN_EXTRA_SPACE 500

typedef struct ev_block_stream {
    char *buf;
    int nwritten;
    int nalloc;
    char initial_buffer[INITIAL_BUF_SIZE];
    } ev_block_stream;

typedef struct ev_stream_state {
    int dummy;
    } ev_stream_state;

ev_block_stream *open_ev_block (ev_stream_state *state);
struct ev_chan_block *close_ev_block (ev_block_stream *stream,
				      ev_stream_state *state);
void ev_block_ensure_space (ev_block_stream *str, int sz);
void ev_fputc (int c, ev_block_stream *str);
void ev_puts_block (char *s, int len, ev_block_stream *str);
void ev_puts (char *s, ev_block_stream *str);
void ev_printf (ev_block_stream *str, char *fmt_string, ...);

#define ev_putc ev_fputc
#else
/*
 * It's convenient to be able to assemble data blocks for the client
 * via printf()-like functions.  Fortunately, on Linux, there's a
 * nonstandard library function that makes it trivial to arrange for
 * this, as follows.  NB the typedefs and #defines are here to allow
 * this stuff to be redefined portably later without changes elsewhere.
 */

typedef FILE ev_block_stream;
extern FILE *open_memstream (char **, size_t *); /* nonstandard GNU fn */

typedef struct ev_stream_state {
    size_t sz;
    char *data;
    } ev_stream_state;

static inline ev_block_stream *
open_ev_block (ev_stream_state *state)
    {
    ev_chan_block header;
    
    FILE* stream = open_memstream (&state->data, &state->sz);
    header.data = 0; header.length = 0; header.next = 0;
    fwrite (&header, sizeof(header), 1, stream); /* reserve space for hdr */
    return stream;
    }

#define ev_printf fprintf
#define ev_puts   fputs
#define ev_putc   putc

static inline ev_chan_block *
close_ev_block (ev_block_stream *stream, ev_stream_state *state)
    {
    struct ev_chan_block *header;
    
    fclose (stream);
    header = (struct ev_chan_block *)state->data;
    header->data = (char*)(header + 1);
    header->length = state->sz - sizeof(struct ev_chan_block);
    header->next = NULL;

    return header;
    }
#endif

/*****************************************************************
 * What we know about a channel to an individual client.
 * We probably should have a format_type here, and dispatch based on
 * that to formatting routines of various kinds (and also use that to
 * control the need for the obnoxious extra blanks and datastream
 * keepalive pseudo-events).
 * But for now, KISS.
 */

/* States of a channel --- basically what we think we are sending, if
 * anything
 */

typedef enum ev_chan_state {
    CHAN_NEW,			/* Just allocated */
    CHAN_SENDING_DATA,		/* Sending data */
    CHAN_PENDING,		/* Newly written data pending */
    CHAN_IDLE,			/* Nothing left to send; may need keepalive */
    CHAN_STATE_MAX,		/* not valid... */
    } ev_chan_state;

/* Channel headers themselves */

typedef struct ev_channel {

    /* We keep doubly linked lists of all channels in a given state */
    
    struct ev_channel *next;
    struct ev_channel *prev;    

    /* And there's another linked list structure for the hash table
     * mapping topic names to channels
     */

    struct ev_channel *h_next;
    struct ev_channel *h_prev;
    
    /* High-level info on what we are reading, and for whom.
     * May someday include format flag for outgoing data
     */
    
    int fd;			/* Socket to client */
    char *topic;		/* Topic client is reading */
    int space_padding;		/* Amount of spaces to pad data transmissions
				 * with, if needed to "wake up" the client
				 */
    int keepalive_spaces;	/* Do we need the pseudo-keepalive
				 * behavior on this channel?  If so, this is
				 * number of spaces to send.
				 */

    /* State of the channel --- data format level.
     * Set and maintained by other code; at this level, it's just a cookie.
     */

    void *fmt_data;
    
    /* State of the channel --- I/O level */
    
    ev_chan_state state;	/* What we're currently sending */
    int bytes_left;		/* How much of it left to send */
    time_t last_data_sent;	/* If idle, time when we went idle;
				 * used to control "space" keepalives
				 */
    
    ev_chan_block *cur_data;	/* ptr to data block we're currently sending,
				 * if any.
				 */
    ev_chan_block *last_data;	/* ptr to last data block currently queued,
				 * to make it easy to queue more of them.
				 */

    /* State of data which we are in the process of writing, and
     * which will eventually show up as our cur_data when finished,
     * if we are pending.
     */
    
    ev_block_stream *datastream;
    ev_stream_state streamstate;
    
    } ev_channel;

/* Writing data to channels.
 *
 * The model here is that at some point, you find out about new data
 * for a bunch of channels.  You call ready_channel on each of them,
 * perhaps more than once if you have several events to write to a
 * given channel.  That gives you an ev_block_stream to write the
 * data on, and sets its state to CHAN_PENDING (even if it already
 * has data queued).
 * 
 * When you're done with writing data to a channel, call
 * channel_data_complete.  That wraps up the buffering, and sets its
 * state to CHAN_SENDING_DATA.
 */

extern ev_block_stream *ready_channel (ev_channel *chan);
extern void channel_data_complete (ev_channel *chan);

/* Try calling this when we think that a channel's client is ready
 * to accept some queued data.  Returns 1 if all data sent, -1 on error,
 * 0 otherwise (in which case, some data may still have been sent).
 */

extern int try_sending_on_channel (ev_channel *chan);

/* Creating and destroying channels
 */

extern ev_channel *create_channel (int fd, char *topic);
extern void destroy_channel (ev_channel *chan);

/* Finding channels on a topic.  WEIRDNESS --- may have multiple channels
 * associated with a given topic.  So, we actually set up iteration to
 * retrieve one *or more* channels...
 */

extern ev_channel *topic_chan_first (char *topic);
extern ev_channel *topic_chan_next  (char *topic, ev_channel *chan);

/* Iterating over channels in various states */

typedef struct ev_channel_iter {
    ev_channel *list_head;
    ev_channel *next;
    } ev_channel_iter;

extern ev_channel *first_channel (ev_chan_state, ev_channel_iter *);
extern ev_channel *next_channel (ev_channel_iter *);

/* Global initialization */

extern void ev_data_init(void);

/* Data structure magic numbers */

#define EV_HASH_BUCKETS 1023

#endif

