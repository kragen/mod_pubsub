/* fdwatch.c - fd watcher routines, either select() or poll()
**
** Copyright © 1999,2000 by Jef Poskanzer <jef@acme.com>.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifdef HAVE_POLL_H
#include <poll.h>
#else /* HAVE_POLL */
#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif /* HAVE_SYS_POLL_H */
#endif /* HAVE_POLL */

#include "fdwatch.h"

#if defined(HAVE_SELECT) && defined(HAVE_POLL)
#define HAVE_SELECT_AND_POLL
#endif

#ifdef HAVE_SELECT
#ifndef FD_SET
#define NFDBITS         32
#define FD_SETSIZE      32
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      bzero((char*)(p), sizeof(*(p)))
#endif /* !FD_SET */
#endif /* HAVE_SELECT */


static int nfiles;
static long nselect, npoll;

#ifdef HAVE_SELECT_AND_POLL
static int numfds;
static int which;
#define USE_SELECT 1
#define USE_POLL 2
#endif /* HAVE_SELECT_AND_POLL */

#ifdef HAVE_SELECT
static fd_set master_rfdset;
static fd_set master_wfdset;
static fd_set working_rfdset;
static fd_set working_wfdset;
static int maxfd;
#endif /* HAVE_SELECT */

#ifdef HAVE_POLL
static struct pollfd* pollfds;
static int* fd_idx;
static int npollfds;
#endif /* HAVE_POLL */


/* Forwards. */

#ifdef HAVE_SELECT
static int fdwatch_select( long timeout_msecs );
#endif /* HAVE_SELECT */


/* Routines. */

/* Figure out how many file descriptors the system allows, and
** initialize the fdwatch data structures.  Returns -1 on failure.
*/
int
fdwatch_get_nfiles( void )
    {
#ifdef RLIMIT_NOFILE
    struct rlimit rl;
#endif /* RLIMIT_NOFILE */

    /* Figure out how many fd's we can have. */
    nfiles = getdtablesize();
#ifdef RLIMIT_NOFILE
    /* If we have getrlimit(), use that, and attempt to raise the limit. */
    if ( getrlimit( RLIMIT_NOFILE, &rl ) == 0 )
	{
	nfiles = rl.rlim_cur;
	if ( rl.rlim_max == RLIM_INFINITY )
	    rl.rlim_cur = 8192;         /* arbitrary */
	else
	    rl.rlim_cur = rl.rlim_max;
	if ( setrlimit( RLIMIT_NOFILE, &rl ) == 0 )
	    nfiles = rl.rlim_cur;
	}
#endif /* RLIMIT_NOFILE */

#if defined(HAVE_SELECT) && ! defined(HAVE_POLL)
    /* If we use select(), then we must limit ourselves to FD_SETSIZE. */
    nfiles = MIN( nfiles, FD_SETSIZE );
#endif /* HAVE_SELECT && ! HAVE_POLL */

    /* Initialize the fdwatch data structures. */
    nselect = npoll = 0;

#ifdef HAVE_POLL
    pollfds = (struct pollfd*) malloc( sizeof(struct pollfd) * nfiles );
    fd_idx = (int*) malloc( sizeof(int) * nfiles );
    if ( pollfds == (struct pollfd*) 0 || fd_idx == (int*) 0 )
	return -1;
#endif /* HAVE_POLL */

    return nfiles;
    }


/* Clear the fdwatch data structures. */
void
fdwatch_clear( void )
    {
#ifdef HAVE_SELECT_AND_POLL
    numfds = 0;
#endif /* HAVE_SELECT_AND_POLL */

#ifdef HAVE_SELECT
    FD_ZERO( &master_rfdset );
    FD_ZERO( &master_wfdset );
    maxfd = -1;
#endif /* HAVE_SELECT */

#ifdef HAVE_POLL
    npollfds = 0;
#endif /* HAVE_POLL */
    }


/* Add a descriptor to the watch list.  rw is either FDW_READ or FDW_WRITE.  */
void
fdwatch_add_fd( int fd, int rw )
    {
#ifdef HAVE_SELECT_AND_POLL
    ++numfds;
#endif /* HAVE_SELECT_AND_POLL */

#ifdef HAVE_SELECT
    switch ( rw )
	{
	case FDW_READ: FD_SET( fd, &master_rfdset ); break;
	case FDW_WRITE: FD_SET( fd, &master_wfdset ); break;
	}
    if ( fd > maxfd )
	maxfd = fd;
#endif /* HAVE_SELECT */

#ifdef HAVE_POLL
    pollfds[npollfds].fd = fd;
    switch ( rw )
	{
	case FDW_READ: pollfds[npollfds].events = POLLIN; break;
	case FDW_WRITE: pollfds[npollfds].events = POLLOUT; break;
	}
    fd_idx[fd] = npollfds;
    ++npollfds;
#endif /* HAVE_POLL */
    }


/* Do the watch.  Return value is the number of descriptors that are ready,
** or 0 if the timeout expired, or -1 on errors.  A timeout of INFTIM means
** wait indefinitely.
*/
int
fdwatch( long timeout_msecs )
    {

#ifdef HAVE_SELECT_AND_POLL
    /* Decide whether to use select() or poll() this time around. */
    if ( maxfd >= FD_SETSIZE )
	/* If the high fd won't fit into an fd_set, we must use poll(). */
	which = USE_POLL;
    else
	{
	/* Figure out which call would transfer less data.  If the fd's are
	** a dense set, this is almost always select().
	*/
	int nfdmasks = ( maxfd + 1 + NFDBITS - 1 ) / NFDBITS;
	int select_size = ( nfdmasks * NFDBITS / 8 ) * 2;
	int poll_size = sizeof(struct pollfd) * numfds;
	/* Give somewhat of an edge to poll(), since it's generally simpler
	** in the kernel.
	*/
	if ( select_size < poll_size * 2 / 3 )
	    which = USE_SELECT;
	else
	    which = USE_POLL;
	}

    switch ( which )
	{
	case USE_SELECT: return fdwatch_select( timeout_msecs );
	case USE_POLL:
	++npoll;
	return poll( pollfds, npollfds, (int) timeout_msecs );
	default: return -1;
	}
#else /* HAVE_SELECT_AND_POLL */

# ifdef HAVE_SELECT
    return fdwatch_select( timeout_msecs );
# endif /* HAVE_SELECT */

# ifdef HAVE_POLL
    ++npoll;
    return poll( pollfds, npollfds, (int) timeout_msecs );
# endif /* HAVE_POLL */

#endif /* HAVE_SELECT_AND_POLL */
    }


#ifdef HAVE_SELECT
static int
fdwatch_select( long timeout_msecs )
    {
    struct timeval timeout;

    ++nselect;
    working_rfdset = master_rfdset;
    working_wfdset = master_wfdset;
    if ( timeout_msecs == INFTIM )
	return select(
	    nfiles, &working_rfdset, &working_wfdset, (fd_set*) 0,
	    (struct timeval*) 0 );
    timeout.tv_sec = timeout_msecs / 1000L;
    timeout.tv_usec = ( timeout_msecs % 1000L ) * 1000L;
    return select(
	maxfd + 1, &working_rfdset, &working_wfdset, (fd_set*) 0, &timeout );
    }
#endif /* HAVE_SELECT */


/* Check if a descriptor was ready.  rw is either FDW_READ or FDW_WRITE.  */
int
fdwatch_check_fd( int fd, int rw )
    {
#ifdef HAVE_SELECT_AND_POLL
    switch ( which )
	{

	case USE_SELECT:
	switch ( rw )
	    {
	    case FDW_READ: return FD_ISSET( fd, &working_rfdset );
	    case FDW_WRITE: return FD_ISSET( fd, &working_wfdset );
	    default: return 0;
	    }

	case USE_POLL:
	switch ( rw )
	    {
	    case FDW_READ: return pollfds[fd_idx[fd]].revents & POLLIN;
	    case FDW_WRITE: return pollfds[fd_idx[fd]].revents & POLLOUT;
	    default: return 0;
	    }

	default: return 0;
	}
#else /* HAVE_SELECT_AND_POLL */

# ifdef HAVE_SELECT
    switch ( rw )
	{
	case FDW_READ: return FD_ISSET( fd, &working_rfdset );
	case FDW_WRITE: return FD_ISSET( fd, &working_wfdset );
	default: return 0;
	}
# endif /* HAVE_SELECT */

# ifdef HAVE_POLL
    switch ( rw )
	{
	case FDW_READ: return pollfds[fd_idx[fd]].revents & POLLIN;
	case FDW_WRITE: return pollfds[fd_idx[fd]].revents & POLLOUT;
	default: return 0;
	}
# endif /* HAVE_POLL */

#endif /* HAVE_SELECT_AND_POLL */
    }


/* Return usage stats on the fdwatch package. */
void
fdwatch_stats( long* nselectP, long* npollP )
    {
    *nselectP = nselect;
    *npollP = npoll;
    nselect = npoll = 0;
    }
