#include <stdarg.h>

ev_block_stream *open_ev_block (ev_stream_state *state)
    {
    ev_block_stream *str = (ev_block_stream*)malloc(sizeof(ev_block_stream));
    str->buf = str->initial_buffer;
    str->nalloc = INITIAL_BUF_SIZE;
    str->nwritten = sizeof (ev_chan_block); /* reserve space for hdr */
    return str;
    }

struct ev_chan_block *close_ev_block (ev_block_stream *stream,
				      ev_stream_state *state)
    {
    struct ev_chan_block *header;

    header = (struct ev_chan_block *)stream->buf;
    
    if (stream->buf == stream->initial_buffer)
	{
	char *ext_copy = malloc (stream->nwritten);
	memcpy (ext_copy, stream->buf, stream->nwritten);
	header = (struct ev_chan_block *)ext_copy;
	}
    
    header->data = (char*)(header + 1);
    header->length = stream->nwritten - sizeof(struct ev_chan_block);
    header->next = NULL;

    free (stream);

    return header;
    }

static void ensure_space (ev_block_stream *str, int sz)
    {
    if (sz <= str->nalloc - str->nwritten)
	return;
    else
	{
	int new_nalloc = str->nalloc * 2;
	char *new_buf;

	while (sz > new_nalloc - str->nwritten)
	    new_nalloc *= 2;

	new_buf = malloc (new_nalloc);
	memcpy (new_buf, str->buf, str->nwritten);

	if (str->buf != str->initial_buffer)
	    free (str->buf);

	str->buf = new_buf;
	str->nalloc = new_nalloc;
	}
    }

void ev_fputc (int c, ev_block_stream *str)
    {
    if (str->nwritten == str->nalloc)
	{
	ensure_space (str, 1);
	}

    str->buf [str->nwritten++] = c;
    }

void ev_puts_block (char *s, int len, ev_block_stream *str)
    {
    ensure_space (str, len);
    memcpy (str->buf + str->nwritten, s, len);
    str->nwritten += len;
    }

void ev_puts (char *s, ev_block_stream *str)
    {
    int len = strlen (s);
    
    ensure_space (str, len);
    memcpy (str->buf + str->nwritten, s, len);
    str->nwritten += len;
    }

void ev_printf (struct ev_block_stream *str, char *fmt_string, ...)
    {
    va_list args;

    va_start (args, fmt_string);

    while (*fmt_string != '\0')
        {
	/* Go to next directive.  If none, just dump the rest of the
	 * format string, and we're done.
	 */
	
	char *endp = strchr (fmt_string, '%');
	char int_buf[30];

	if (endp == NULL)
	    {
	    ev_puts (fmt_string, str);
	    break;
	    }

	/* Have one.  Intervening characters should be copied literally */

	ev_puts_block (fmt_string, endp - fmt_string, str);

	/* And deal with the directive */

	fmt_string = endp + 1;

	switch (*fmt_string++)
	    {
	    case '\0': goto out; /* string ended in '%' */
	    case '%': ev_putc ('%', str); break;
	    
	    case 's':
		ev_puts (va_arg (args, char*), str);
		break;

	    case 'd':
		sprintf (int_buf, "%d", va_arg (args, int));
		ev_puts (int_buf, str);
		break;

	    default: break;		/* Sigh... */
	    }
	}

    out:
    va_end (args);
    return;
    }
