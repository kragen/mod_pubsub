/**
 * Copyright (c) 2000-2002 KnowNow, Inc.  All rights reserved.
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

/* $Id: ev_pubsub.c,v 1.1 2002/11/07 07:09:41 troutgirl Exp $ */

#include "evdispatch.h"
#include "ev_pubsub.h"

#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>		/* mkdir declared *here*? */
#include <dirent.h>

#include <assert.h>

/* This code is effectively a C paraphrase of kn.cgi and EventFormat.pm,
 * or more precisely, the relevant portions thereof.
 *
 * It's divided more or less roughly into halves; the code "above the
 * line" is concerned with the layout of data in the file system --- the
 * log file(s), event directories, and so forth.  The code below the line
 * is mostly concerned with the delivery of data to the user --- formatting
 * it, queueing it onto associated channels, channel creation and deletion,
 * and related housekeeping.  The two concerns meet in the routine
 * dump_event_to_chan, which reads the data out of an event file and
 * formats it for delivery on a channel.
 */

/* First, the static file-globals holding the state of the filesystem,
 * and the state of our current reference to the logfile.  They are:
 *
 * ev_dir --- base of the directory hierarchy for events
 *
 * ev_log_filename --- name of log file, or template if we are doing
 *       log rotation
 *
 * ev_log_file --- open FILE* to the (current) event log file.
 *       This is *almost* always seeked to the last event which
 *       has been shipped by process_kn_events, which is the function
 *       that handles ordinary event delivery.  Exceptions are (minor)
 *       when process_kn_events has read an event which it hasn't
 *       finished delivering, and (major) during restart of a dropped
 *       connection, when queue_events_since seeks back to recover
 *       events which were posted after the connection got dropped.
 *
 *       See record_ev_posn/seek_to_ev_posn, which are the routines
 *       which are used by queue_event_... to seek the log.
 *
 * ev_log_num --- if we aren't doing log rotation, -1.  Otherwise, the
 *       number of the log that ev_log_file is currently open to.  This
 *       is used to avoid gratuitously reopening the log file when seeking
 *       in seek_to_ev_posn.
 */

static char *ev_dir;
static char *ev_log_filename;
static FILE *ev_log_file = NULL;
static int ev_log_num = -1;

/* Dealing with the log file name and templates */

/* Identify most recent log file */

static int
most_recent_log_file (char *log_file_name)
    {
    DIR *dir;
    struct dirent *dirp;
    char *filep;
    int max = -1;
	
    /* If file name does not end in '%', return -1 immediately. */

    int len = strlen (log_file_name);
    
    if (log_file_name [len - 1] != '%')
	return -1;

    /* Separate name into dir and file components, and open directory.
     * YYY This fails if the file named is *in* the root directory.
     *     It's an odd corner case, which won't happen anyway, so
     *     just do an assert to make the bug obvious...
     */

    filep = strrchr (log_file_name, '/');

    if (filep == NULL)
	{
	filep = log_file_name;
	dir = opendir (".");
	}
    else if (filep == log_file_name)
	{
	assert (0 != "Can't rotate log files in the root directory");
	return -1;		/* quash "dir might not be inited" warning */
	}
    else
	{
	*filep++ = '\0';	/* temporarily; see below */
	dir = opendir (log_file_name);
	}

    if (dir == NULL)
	{
	fprintf (stderr, "Couldn't open dir %s: %s\n", 
		 ((filep == log_file_name) ? "." : log_file_name),
		 strerror (errno));
	exit (1);
	}
    
    /* Scan the directory, looking for log files as they appear */

    len = strlen (filep) - 1;	/* length of log name prefix, w/o '%' char */
    
    while ((dirp = readdir (dir)))
	if (!strncmp (dirp->d_name, filep, len))
	    {
	    int current_num = atoi (dirp->d_name + len);

	    if (current_num > max)
		max = current_num;
	    }
    
    /* patch up the filename, if needed, and return */

    if (filep != log_file_name)
	{
	*--filep = '/';
	}

    return max;
    }

/* Open a particular log file.  Resets the globals above.
 * This should be a class method on a KN::EventFormat type thing...
 * Returns 1 on success, which should generally be followed immediately
 * by an fseek on ev_log_file...
 */

static int
open_log_file (int new_log_num)
    {
    int len = strlen (ev_log_filename);
    FILE *f;
	
    /* Just do the dumb thing if no rotation */
    
    if (ev_log_num == -1)
	f = fopen (ev_log_filename, "r");
    else
	{
	/* Compute name of this log file */
    
	char *log_file_buf = malloc(len + 30); /* > len of longest int */
	strcpy (log_file_buf, ev_log_filename);
	sprintf (log_file_buf + len - 1, "%d", new_log_num);

	/* Try to open it */
    
	f = fopen (log_file_buf, "r");
	free (log_file_buf);
	}
    
    if (f == NULL) return 0;

    /* Adjust globals on success */

    if (ev_log_file != NULL) fclose (ev_log_file);

    ev_log_file = f;
    
    if (ev_log_num != -1) ev_log_num = new_log_num;

    return 1;
    }

/* Initialization */

static void
init_log_files (char *log_file_name, char *dir_name)
    {
    ev_dir = dir_name;
    ev_log_filename = log_file_name;
    ev_log_num = most_recent_log_file (log_file_name);
      
    if (!open_log_file (ev_log_num))
	{
	/* XXX probably a better way to handle errors than this ---
	 * will stderr still be open??
	 */
	fprintf (stderr, "Could not open log file %s\n", log_file_name);
	exit(1);
	}

    fseek (ev_log_file, 0, SEEK_END);
    }

static FILE *
fopen_event_file (char *mode, char *topic, char *ev_file_name)
    {
    char *name = malloc (strlen (ev_dir) + strlen (topic)
			 + strlen (ev_file_name) + 2);
    FILE *rv;

    strcpy (name, ev_dir); 
    strcat (name, topic);
    strcat (name, "/");
    strcat (name, ev_file_name);

    rv = fopen(name, mode);
    free(name);

    return rv;
    }

/* Dealing with connection restart.  When we're restarting a connection,
 * we have to go back in the log to pick up events since the last one
 * which it received; we immediately queue all events from there to the
 * end of the current log.  That requires us to ship event IDs to the
 * client which we can use to recover events past a point, i.e.,
 * seeking the log...
 */

#define EV_LOC_SZ 64		/* size of a char buffer for ev_posn cookies */

static void
record_ev_posn (char *buf)
    {
    sprintf (buf, "a%dx%ld",
	     (ev_log_num == -1 ? 0 : ev_log_num),
	     ftell (ev_log_file));
    }

static void
seek_to_ev_posn (char *buf)
    {
    char *xposn = strchr (buf, 'x');
    long posn;
    int fileno = atoi (buf + 1);

    if (xposn == NULL) return;	/* client supplied bad data --- don't seek */

    posn = atol (xposn + 1);

    if (!open_log_file (fileno)) return; /* bad data again... we did nothing */

    /* To avoid possible screws from garbled positions, we seek to
     * (what should be) the last character of the preceding record,
     * and advance.  So the worst a client can do by specifying a
     * nonsense offset is to get a lot of data queued for it, which
     * it could do anyway.
     */
    
    if (posn <= 0)
	fseek (ev_log_file, 0, SEEK_SET);
    else
	{
	int c;
	    
	fseek (ev_log_file, posn - 1, SEEK_SET);

	while ((c = getc (ev_log_file)) != EOF && c != '\n')
	    continue;
	}
    }

/* Get an event out of a log file.  Assumes that log entries are small
 * enough to be written atomically, so that we will read a whole record
 * if we get anything at all.  For this to be true, log entries must stay
 * smaller than PIPSIZ.  Sets topic and event if a record is found, and
 * returns 1; otherwise, returns 0.
 *
 * Note that the use of a static buffer creates a thread-safety hazard,
 * but there are single-threaded assumptions all over...
 */

static char ev_buf[1024];	/* Long enough? */
static char ev_loc_buf[EV_LOC_SZ];

static int
read_kn_event (char **topic, char **event, char **posn)
    {
    if (!fgets (ev_buf, sizeof(ev_buf), ev_log_file))
	{
	/* At EOF, at least for now; clear EOF flag to make sure
	 * that we'll be able to read more the next time through...
	 */
	clearerr (ev_log_file);
	return 0;
	}
    else
	{
	/* Check here for log rotation, and reopen the file
	 * if we have one, then recurse to get the actual event, if any.
	 * Note that the new file must be created *before* the end record
	 * is written to the old one, so that we have something to open
	 * here...
	 */

	if (!strncmp (ev_buf, "end", 3))
	    {
	    assert (open_log_file (++ev_log_num));
	    return read_kn_event (topic, event, posn);
	    }
	else
	    {
	    char *tab1 = strchr (ev_buf, '\t');
	    char *tab2 = strchr (tab1 + 1, '\t');
	    char *newline = strchr (tab2 + 1, '\n');

	    *tab2 = '\0'; *newline = '\0';
	
	    record_ev_posn (ev_loc_buf);
	
	    *topic = tab1 + 1;
	    *event = tab2 + 1;
	    *posn = ev_loc_buf;
	    
	    return 1;
	    }
	}
    }

/* Freshening the hearbeat on a topic.  Returns 1 on success, 0 on
 * error; this is only checked right now on channel creation, to see
 * if the topic exists
 */

static int
fresh_heartbeat (char *topic)
    {
    FILE *f = fopen_event_file ("w", topic, ".heartbeat");

    if (!f) return 0;

    fputs ("\n", f);
    fclose (f);
    return 1;
    }

static int
ensure_event_loc_exists (char *topic)
    {
    int name_len = strlen (ev_dir) + strlen (topic) + 1;
    char *name = malloc (name_len);
    
    int i;

    strcpy (name, ev_dir); strcat (name, topic);

    for (i = strlen (ev_dir) + 1; i < name_len; ++i)
	{
	if (name[i] == '/' || name[i] == '\0')
	    {
	    name[i] = '\0';

	    if (mkdir (name, 0777) < 0 && errno != EEXIST)
	    {
		free(name);
		return 0;
	    }
	    
	    name[i] = '/';
	    }
	}

    free(name);
    return 1;
    }

/* Wrapping up data packets on a kn-stream.  In addition to the usual
 * stuff, we also tickle the hearbeat file, at least for the moment.
 * (There really ought to be a way to do this that doesn't involve so
 * many syscalls, but this is for communication with event delivery,
 * which itself needs an overhaul).
 */

static void
kn_data_complete (ev_channel *chan)
    {
    fresh_heartbeat (chan->topic);
    channel_data_complete (chan);
    }

/****************************************************************
 *
 * Code below this point is concerned with delivery of messages to
 * clients... formatting them for browsers, channel management,
 * and other housekeeping.
 */

/* We need to write at least 4k to some browsers to get them to
 * notice they've received any data at all.  So...
 */

static char tickle_buf[4096];

static void
init_tickle_buf (void)
    {
    memset (tickle_buf, ' ', 4095);
    tickle_buf[4095] = '\0';
    }

static inline void
ev_write_tickler (ev_block_stream *out)
    {
    ev_puts (tickle_buf, out);
    }

/* Reading and writing UTF-8 characters */

static int
ev_read_utf (FILE *in)
    {
    int c;
    int clen;
    int cmin;

    if ((c = getc(in)) == EOF)
        {
        return EOF;
        }
    else if ((c & 0x80) == 0)
        {
        return c;
        }
    else if ((c & 0xE0) == 0xC0)
        {
        c    = (c & 0x1F) << 6;
        clen = 0;
        cmin = 0x7F;
        }
    else if ((c & 0xF0) == 0xE0)
        {
        c    = (c & 0xF) << 12;
        clen = 6;
        cmin = 0x7FF;
        }
    else if ((c & 0xF8) == 0xF0)
        {
        c    = (c & 0x7) << 18;
        clen = 12;
        cmin = 0xFFFF;
        }
    else if ((c & 0xFC) == 0xF8)
        {
        c    = (c & 0x3) << 24;
        clen = 18;
        cmin = 0x1FFFFF;
        }
    else if ((c & 0xFE) == 0xFC)
        {
        c   = (c & 0x1) << 30;
        clen = 24;
        cmin = 0x3FFFFFF;
        }
    else /* bad value or the reserved byte */
        {
        return EOF;
        }

    for (; clen >= 0; clen -= 6)
        {
        int nc;

        if ((nc = getc(in)) == EOF || (nc & 0xC0) != 0x80)
            return EOF;

        c |= (nc & 0x3F) << clen;
        }

    return (c > cmin ? c : EOF);
    }

static void
ev_write_utf (int c, ev_block_stream *out)
    {
    int slen;
    char s[6]; /* max. UTF-8 sequence length */
    char *ptr;

    if (c <= 0x7F)
        {
        ev_putc (c, out);
        return;
        }
    else if (c <= 0x7FF)
        {
        s[0] = 0xC0 | ((c >> 6) & 0x1F);
        slen = 0;
        }
    else if (c <= 0xFFFF)
        {
        s[0] = 0xE0 | ((c >> 12) & 0xF);
        slen = 6;
        }
    else if (c <= 0x1FFFFF)
        {
        s[0] = 0xF0 | ((c >> 18) & 0x7);
        slen = 12;
        }
    else if (c <= 0x3FFFFFF)
        {
        s[0] = 0xF8 | ((c >> 24) & 0x3);
        slen = 18;
        }
    else
        {
        s[0] = 0xFC | ((c >> 30) & 0x1);
        slen = 24;
        }

    for (ptr = s + 1; slen >= 0; slen -= 6)
        {
            *ptr++ = 0x80 | ((c >> slen) & 0x3F);
        }

    ev_puts_block (s, ptr - s, out);
    }

static inline void
ev_putc_escaped (int c, ev_block_stream *out)
    {
    if (c <= 0x7F)
        switch (c)
	    {
	    case '>':  ev_puts ("&gt;", out);   break;
	    case '<':  ev_puts ("&lt;", out);   break;
	    case '&':  ev_puts ("&amp;", out);  break;
	    case '"':  ev_puts ("&quot;", out); break;
	    
	    default: ev_putc (c, out);
	    }
    else
        ev_write_utf (c, out);
    }

/* This function takes Kragen's pseudo-RFC822 format event dumps
 * (no RFC822 continuations, single-newline line termination)
 * and reformats as a JS-form...
 */

static void
dump_event_to_chan (FILE *in, ev_channel *chan, char *posn)
    {
    ev_block_stream *out = ready_channel (chan);
    char *callback = chan->fmt_data;
    int c;
    
    /* Start JS format event dump; initial form tag */
    
    ev_printf (out, "<form name=\"%s\">", posn);

    /* Turn the header lines into pseudo-inputs */

    while ((c = ev_read_utf(in)) != '\n' && c != EOF)
	{
	/* Just read start of a new header line.  Filter and process.
	 * We deliberately eschew fixed-size buffers here, which
	 * unfortunately makes the output harder to read than it ought
	 * to be (you've got to read HTML source to find header names).
	 */
	
	ev_puts ("<input name=\"", out);
	
	do {
	    ev_write_utf (c, out);
	    c = ev_read_utf(in);
	} while (c != '\n' && c != ':' && c != EOF);

	if (c == ':')
	    {
	    ev_puts ("\" value=\"", out);

	    while ((c = ev_read_utf(in)) != EOF && c != '\n' && isspace(c))
		continue;

	    while (c != EOF && c != '\n')
		{
		ev_putc_escaped (c, out);
		c = ev_read_utf(in);
		}
	    }

	ev_puts ("\" size=\"40\" /><br />\n", out);
	}

    /* Just read a blank line (or EOF).
     * Should now have advanced to start of body; copy it in.
     */

    ev_puts ("<textarea cols=\"40\" rows=\"3\" name=\"kn_payload\">", out);

    while ((c = ev_read_utf(in)) != EOF)
	ev_putc_escaped (c, out);

    ev_puts ("</textarea></form>\n<script language=\"javascript\">\n", out);

    /* And wrap up, writing out invocation of JS callback */
    
    ev_printf (out, "parent.%s(document.%s);\n", callback, posn);
    ev_puts ("</script>\n", out);
    }

static void
process_kn_event (char *topic, char *event, char *posn)
    {
    FILE *f = fopen_event_file ("r", topic, event);
    ev_channel *chan;

    if (f == NULL)
	{
	/* XXX log error!  NB thttpd closes stderr... */
	return;
	}

    for (chan = topic_chan_first (topic);
	 chan;
	 chan = topic_chan_next (topic, chan))
	{
	fseek (f, 0, SEEK_SET);
	dump_event_to_chan (f, chan, posn);
	}

    fclose (f);
    }

/* Process all new events since the last time we checked; returns the
 * number of channels with new data to write (zero if none).
 */

int
process_kn_events (void)
    {
    int nchannels = 0;
    
    ev_channel *chan;
    ev_channel_iter iter;
    char *topic, *event, *ev_locator;

    /* Handle everything that has shown up in the event log */
    
    while (read_kn_event (&topic, &event, &ev_locator))
	process_kn_event (topic, event, ev_locator);

    /* ... and wrap up the events we just wrote, chasing with the tickler */

    for (chan = first_channel (CHAN_PENDING, &iter);
	 chan;
	 chan = next_channel (&iter))
         {
	 ev_write_tickler (ready_channel (chan));
	 kn_data_complete (chan);
	 ++nchannels;
         }
    
    return nchannels;
    }

/* For channel restart, queue all events from some topic between
 * last_posn and the last event we have ever sent.
 *
 * (Events after that will be sent by process_kn_events, as usual,
 * even if they have already been written to the log).
 */

static void
queue_events_since (char *last_posn, char *topic, ev_channel *chan)
    {
    char *ev_topic, *event, *posn;
    char end_posn[EV_LOC_SZ];

    /* Note current position in the event log, then seek back to
     * recorded position
     */
    
    record_ev_posn (end_posn);
    seek_to_ev_posn (last_posn);
	
    while (1)
	{
	/* If not yet at end ... */
		
	char current_posn[EV_LOC_SZ];
	record_ev_posn (current_posn);

	if (!strcmp (current_posn, end_posn))
	    break;

	/* Read next event, and queue it if appropriate */
	    
	assert (read_kn_event (&ev_topic, &event, &posn) != 0);
	    
	if (!strcmp (topic, ev_topic))
	    {
	    FILE *f = fopen_event_file ("r", topic, event);

	    if (f != NULL)
		{
		dump_event_to_chan (f, chan, posn);
		fclose (f);
		}
	    }
	}
    }


/* Creating and destroying channels */

int
create_kn_channel (int fd, char *topic, int max_age, char *callback,
		   char *init, char *onload, char *onerror, char *last_posn)
    {
    ev_channel *chan;
    ev_block_stream *out;

    /* First, create the topic, if need be, and tickle the heartbeat.
     * Permissions errors here are kinda fatal.
     */
	
    if (!ensure_event_loc_exists (topic) || !fresh_heartbeat (topic))
	return 0;
	
    /* OK, create the channel data structure */
	
    chan = create_channel (fd, topic);
    chan->fmt_data = strdup (callback);
    
    /* Dump HTTP header to channel output */

    out = ready_channel (chan); 

    ev_puts ("HTTP/1.0 200 OK\r\n", out);
    ev_puts ("Content-type: text/html; charset=utf-8\r\n", out);
    ev_puts ("Server: KN-event-engine/0.0\r\n\r\n", out);

    /* Dump HTML start to out. */

    ev_puts ("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n", out);
    ev_printf (out, "<HTML><HEAD><TITLE>%s</TITLE></HEAD>\n", topic);
    
    ev_puts ("<BODY ", out);
    if (onerror && *onerror) ev_printf (out, "ONERROR=\"%s\" ", onerror);
    if (onload && *onload)   ev_printf (out, "ONLOAD=\"%s\" ", onload);
    ev_puts (">\n", out);

    if (init && *init)
	ev_printf (out, "<script type=\"text/javascript\">%s</script>", init);

    /* get events on channel between last_posn and last we have sent
     */

    assert (max_age == 0);
    
    if (last_posn != NULL)
	queue_events_since (last_posn, topic, chan);
    
    /* Arrange for data to get sent */
    
    ev_write_tickler (out);
    kn_data_complete (chan);

    /* Return success */
    
    return 1;
    }

void
destroy_kn_channel (ev_channel *chan)
    {
    free (chan->fmt_data);
    chan->fmt_data = NULL;
    destroy_channel (chan);
    }

int
schedule_kn_keepalives (void)
    {
    time_t needs_keepalive = time(NULL) - 30;
    ev_channel *chan;
    ev_channel_iter iter;
    int nchannels = 0;

    /* Idle list is in time order of channels going idle, so
     * we can stop at the first that doesn't need a keepalive
     */
    
    for (chan = first_channel (CHAN_IDLE, &iter);
	 chan;
	 chan = next_channel (&iter))
	{
	if (chan->last_data_sent > needs_keepalive)
	    break;
	
	/* Send one space keepalive.
	 * We don't need this I/O, in principle, to detect that connections
	 * have gone stale --- we could use TCP-level keepalives for that,
	 * though we'd have to hack fdwatch.c to look for error conditions.
	 * But we do need it anyway to convince the browsers that we haven't
	 * dropped dead...
	 */
	
	ev_putc (' ', ready_channel (chan));
	kn_data_complete (chan);
	++nchannels;
	}

    return nchannels;
    }

/* General initialization */

void
init_pubsub_evstreams (char *ev_log_name, char *ev_dir_name)
    {
    init_log_files (ev_log_name, ev_dir_name);
    init_tickle_buf();
    ev_data_init();
    }

   
