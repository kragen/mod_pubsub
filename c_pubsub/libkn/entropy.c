/**
 * PubSub Client Library
 * Entropy generator
 *
 * Wilfredo Sanchez
 **/
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
 **/

#define _RAND_ID_ "$Id: entropy.c,v 1.3 2004/04/19 05:39:08 bsittler Exp $"

#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <openssl/rand.h>
#include "prng_cmds.h"

#define SSH_PRNG_COMMAND_FILE ETCDIR"/prng_cmds"

static int prng_read_commands(char *cmdfilename);
static double stir_from_system(void);
static double stir_from_programs(void);

static int prng_random_fd = -1;

static kn_BOOL init_prng ()
{
  if (prng_random_fd != -1) return kn_TRUE;

  prng_random_fd = open("/dev/urandom", O_RDONLY, (mode_t)0);

  if (prng_random_fd != -1) return kn_TRUE;

  prng_random_fd = open("/dev/random", O_RDONLY, (mode_t)0);

  if (prng_random_fd != -1) return kn_TRUE;

  if (prng_read_commands(SSH_PRNG_COMMAND_FILE)) return kn_TRUE;

#ifdef DEBUG
  fprintf(stderr, "Failed to read PRNG command file " SSH_PRNG_COMMAND_FILE "\n");
#endif

  return kn_FALSE;
}

typedef void (*sighandler_t)(int);

static kn_BOOL seed_prng ()
{
  int i;

  for (i = 0; i < 100; i++)
    {
      if (prng_random_fd >= 0)
	{
	  unsigned char buf[32];

	  if (read(prng_random_fd, buf, sizeof(buf)) != sizeof(buf))
	    {
#ifdef DEBUG
	      fprintf(stderr, "Failed to read from random device\n");
#endif
	      prng_random_fd = -1;
	      return kn_FALSE;
	    }

	  RAND_add(buf, sizeof(buf), sizeof(buf));

	  memset(buf, '\0', sizeof(buf));

	  if (RAND_status()) return kn_TRUE;
	}
      else
	{
	  sighandler_t old_handler;

	  old_handler = signal(SIGCHLD, SIG_DFL);

	  stir_from_programs();
	  stir_from_system  ();

	  signal(SIGCHLD, old_handler);

	  if (RAND_status()) return kn_TRUE;
	}
    }
  return kn_FALSE;
}

/*
 * Copyright (c) 2000 Damien Miller.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

static const char rand_copyright[] __attribute__((__unused__)) =
"@(#) entropy.c (from OpenSSH 2.9p2) copyright (c) 2000 Damien Miller.  All rights reserved.";

#define xmalloc  malloc
#define xrealloc realloc
#define xstrdup  strdup

#define error  printf
#define debug  printf
#define debug2 debug
#define debug3 debug

#include <sys/stat.h>

/* #define mysignal signal */

/* #include "atomicio.c" */
#include "strlcpy.c"

/* Number of times to pass through command list gathering entropy */
#define NUM_ENTROPY_RUNS	1

/* Scale entropy estimates back by this amount on subsequent runs */
#define SCALE_PER_RUN		10.0

/* Minimum number of commands to be considered valid */
#define MIN_ENTROPY_SOURCES 12

#define WHITESPACE " \t\n"

#ifndef RUSAGE_SELF
# define RUSAGE_SELF 0
#endif
#ifndef RUSAGE_CHILDREN
# define RUSAGE_CHILDREN 0
#endif

#define ENTROPY_TIMEOUT_MSEC 200

/* slow command timeouts (all in milliseconds) */
/* static int entropy_timeout_default = ENTROPY_TIMEOUT_MSEC; */
static int entropy_timeout_current = ENTROPY_TIMEOUT_MSEC;

typedef struct
{
	/* Proportion of data that is entropy */
	double rate;
	/* Counter goes positive if this command times out */
	unsigned int badness;
	/* Increases by factor of two each timeout */
	unsigned int sticky_badness;
	/* Path to executable */
	char *path;
	/* argv to pass to executable */
	char *args[5];
	/* full command string (debug) */
	char *cmdstring;
} entropy_source_t;

/* this is initialised from a file, by prng_read_commands() */
static entropy_source_t *entropy_sources = NULL;

static char *tmpfname = NULL;

/*
 * entropy command initialisation functions
 */
static int
prng_read_commands(char *cmdfilename)
{
	FILE *f;
	char *cp;
	char line[1024];
	char cmd[1024];
	char path[256];
	int linenum;
	int num_cmds = 64;
	int cur_cmd = 0;
	int fd = 0;
	double est;
	entropy_source_t *entcmd;
	struct stat buf;

	f = fopen(cmdfilename, "r");

	/** if we can't find the file under /usr/local/kn/libkn
      * create a temp file with entropy commands
      */
	if (!f)
	{
		if ((tmpfname == NULL) || (stat(tmpfname, &buf) != 0)) 
		{
			tmpfname = tmpnam(NULL);
		    fd = open(tmpfname, O_RDWR|O_CREAT, 0600);
		    write(fd, __prngcmds, strlen(__prngcmds));
		    close(fd);
		}
		f = fopen(tmpfname, "r");
	}
		
	if (!f) 
	{
	
#if 0
		fatal("couldn't read entropy commands file %.100s: %.100s",
		    cmdfilename, strerror(errno));
#else
		errno = EBADF;
		return 0;
#endif
	}

	entcmd = (entropy_source_t *)xmalloc(num_cmds * sizeof(entropy_source_t));
	memset(entcmd, '\0', num_cmds * sizeof(entropy_source_t));

	/* Read in file */
	linenum = 0;
	while (fgets(line, sizeof(line), f)) {
		int arg;
		char *argv;

		linenum++;

		/* skip leading whitespace, test for blank line or comment */
		cp = line + strspn(line, WHITESPACE);
		if ((*cp == 0) || (*cp == '#'))
			continue; /* done with this line */

		/* First non-whitespace char should be double quote delimiting */
		/* commandline */
		if (*cp != '"') {
#ifdef DEBUG
			error("bad entropy command, %.100s line %d\n", cmdfilename,
			     linenum);
#endif
			continue;
		}

		/* first token, command args (incl. argv[0]) in double quotes */
		cp = strtok(cp, "\"");
		if (cp == NULL) {
#ifdef DEBUG
			error("missing or bad command string, %.100s line %d -- ignored\n",
			      cmdfilename, linenum);
#endif
			continue;
		}
		kn_strlcpy(cmd, cp, sizeof(cmd));

		/* second token, full command path */
		if ((cp = strtok(NULL, WHITESPACE)) == NULL) {
#ifdef DEBUG
			error("missing command path, %.100s line %d -- ignored\n",
			      cmdfilename, linenum);
#endif
			continue;
		}

		/* did configure mark this as dead? */
		if (strncmp("undef", cp, 5) == 0)
			continue;

		kn_strlcpy(path, cp, sizeof(path));

		/* third token, entropy rate estimate for this command */
		if ((cp = strtok(NULL, WHITESPACE)) == NULL) {
#ifdef DEBUG
			error("missing entropy estimate, %.100s line %d -- ignored\n",
			      cmdfilename, linenum);
#endif
			continue;
		}
		est = strtod(cp, &argv);

		/* end of line */
		if ((cp = strtok(NULL, WHITESPACE)) != NULL) {
#ifdef DEBUG
			error("garbage at end of line %d in %.100s -- ignored\n", linenum,
				cmdfilename);
#endif
			continue;
		}

		/* save the command for debug messages */
		entcmd[cur_cmd].cmdstring = xstrdup(cmd);

		/* split the command args */
		cp = strtok(cmd, WHITESPACE);
		arg = 0;
		argv = NULL;
		do {
			char *s = (char*)xmalloc(strlen(cp) + 1);
			strncpy(s, cp, strlen(cp) + 1);
			entcmd[cur_cmd].args[arg] = s;
			arg++;
		} while ((arg < 5) && (cp = strtok(NULL, WHITESPACE)));

#ifdef DEBUG
		if (strtok(NULL, WHITESPACE))
			error("ignored extra command elements (max 5), %.100s line %d\n",
			      cmdfilename, linenum);
#endif

		/* Copy the command path and rate estimate */
		entcmd[cur_cmd].path = xstrdup(path);
		entcmd[cur_cmd].rate = est;

		/* Initialise other values */
		entcmd[cur_cmd].sticky_badness = 1;

		cur_cmd++;

		/* If we've filled the array, reallocate it twice the size */
		/* Do this now because even if this we're on the last command,
		   we need another slot to mark the last entry */
		if (cur_cmd == num_cmds) {
			num_cmds *= 2;
			entcmd = xrealloc(entcmd, num_cmds * sizeof(entropy_source_t));
		}
	}

	/* zero the last entry */
	memset(&entcmd[cur_cmd], '\0', sizeof(entropy_source_t));

	/* trim to size */
	entropy_sources = xrealloc(entcmd, (cur_cmd+1) * sizeof(entropy_source_t));

#ifdef DEBUG_RAND
	debug("Loaded %d entropy commands from %.100s\n", cur_cmd, cmdfilename);
#endif

	errno = 0;
	fclose(f);
	return (cur_cmd >= MIN_ENTROPY_SOURCES);
}

static int
_get_timeval_msec_difference(struct timeval *t1, struct timeval *t2) {
	int secdiff, usecdiff;

	secdiff = t2->tv_sec - t1->tv_sec;
	usecdiff = (secdiff*1000000) + (t2->tv_usec - t1->tv_usec);
	return (int)(usecdiff / 1000);
}

static double
hash_output_from_command(entropy_source_t *src, char *hash)
{
	static int devnull = -1;
	int p[2];
	fd_set rdset;
	int cmd_eof = 0, error_abort = 0;
	struct timeval tv_start, tv_current;
	int msec_elapsed = 0;
	pid_t pid;
	int status;
	char buf[16384];
	int bytes_read;
	int total_bytes_read;
	SHA_CTX sha;

#ifdef DEBUG_RAND
	debug3("Reading output from \'%s\'\n", src->cmdstring);
#endif

	if (devnull == -1) {
		devnull = open("/dev/null", O_RDWR);
		if (devnull == -1) {
#if 0
			fatal("Couldn't open /dev/null: %s", strerror(errno));
#else
			return(0.0);
#endif
		}
	}

	if (pipe(p) == -1) {
#if 0
		fatal("Couldn't open pipe: %s", strerror(errno));
#else
		return(0.0);
#endif
	}

	(void)gettimeofday(&tv_start, NULL); /* record start time */

	switch (pid = fork()) {
		case -1: /* Error */
			close(p[0]);
			close(p[1]);
#if 0
			fatal("Couldn't fork: %s", strerror(errno));
#else
			return(0.0);
#endif
			/* NOTREACHED */
		case 0: /* Child */
			dup2(devnull, STDIN_FILENO);
			dup2(p[1], STDOUT_FILENO);
			dup2(p[1], STDERR_FILENO);
			close(p[0]);
			close(p[1]);
			close(devnull);

#if 0
			setuid(original_uid);
#endif

			execv(src->path, (char**)(src->args));
#ifdef DEBUG
			debug("(child) Couldn't exec '%s': %s\n", src->cmdstring,
			      strerror(errno));
#endif
			_exit(-1);
		default: /* Parent */
			break;
	}

	RAND_add(&pid, sizeof(&pid), 0.0);

	close(p[1]);

	/* Hash output from child */
	SHA1_Init(&sha);
	total_bytes_read = 0;

	while (!error_abort && !cmd_eof) {
		int ret;
		struct timeval tv;
		int msec_remaining;

		(void) gettimeofday(&tv_current, 0);
		msec_elapsed = _get_timeval_msec_difference(&tv_start, &tv_current);
		if (msec_elapsed >= entropy_timeout_current) {
			error_abort=1;
			continue;
		}
		msec_remaining = entropy_timeout_current - msec_elapsed;

		FD_ZERO(&rdset);
		FD_SET(p[0], &rdset);
		tv.tv_sec =  msec_remaining / 1000;
		tv.tv_usec = (msec_remaining % 1000) * 1000;

		ret = select(p[0]+1, &rdset, NULL, NULL, &tv);

		RAND_add(&tv, sizeof(tv), 0.0);

		switch (ret) {
		case 0:
			/* timer expired */
			error_abort = 1;
			break;
		case 1:
			/* command input */
			bytes_read = read(p[0], buf, sizeof(buf));
			RAND_add(&bytes_read, sizeof(&bytes_read), 0.0);
			if (bytes_read == -1) {
				error_abort = 1;
				break;
			} else if (bytes_read) {
				SHA1_Update(&sha, buf, bytes_read);
				total_bytes_read += bytes_read;
			} else {
				cmd_eof = 1;
			}
			break;
		case -1:
		default:
			/* error */
#ifdef DEBUG
			debug("Command '%s': select() failed: %s\n", src->cmdstring,
			      strerror(errno));
#endif
			error_abort = 1;
			break;
		}
	}

	SHA1_Final(hash, &sha);

	close(p[0]);

#ifdef DEBUG_RAND
	debug3("Time elapsed: %d msec\n", msec_elapsed);
#endif

	if (waitpid(pid, &status, 0) == -1) {
	       error("Couldn't wait for child '%s' completion: %s", src->cmdstring,
		     strerror(errno));
		return(0.0);
	}

	RAND_add(&status, sizeof(&status), 0.0);

	if (error_abort) {
		/* closing p[0] on timeout causes the entropy command to
		 * SIGPIPE. Take whatever output we got, and mark this command
		 * as slow */
#ifdef DEBUG_RAND
		debug2("Command '%s' timed out\n", src->cmdstring);
#endif
		src->sticky_badness *= 2;
		src->badness = src->sticky_badness;
		return(total_bytes_read);
	}

	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status)==0) {
			return(total_bytes_read);
		} else {
#ifdef DEBUG_RAND
			debug2("Command '%s' exit status was %d\n", src->cmdstring,
				WEXITSTATUS(status));
#endif
			src->badness = src->sticky_badness = 128;
			return (0.0);
		}
	} else if (WIFSIGNALED(status)) {
#ifdef DEBUG
		debug2("Command '%s' returned on uncaught signal %d !\n", src->cmdstring,
			status);
#endif
		src->badness = src->sticky_badness = 128;
		return(0.0);
	} else
		return(0.0);
}

static double
stir_gettimeofday(double entropy_estimate)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == -1) {
#if 0
		fatal("Couldn't gettimeofday: %s", strerror(errno));
#else
		return(0.0);
#endif
	}

	RAND_add(&tv, sizeof(tv), entropy_estimate);

	return(entropy_estimate);
}

static double
stir_clock(double entropy_estimate)
{
#ifdef HAVE_CLOCK
	clock_t c;

	c = clock();
	RAND_add(&c, sizeof(c), entropy_estimate);

	return(entropy_estimate);
#else /* _HAVE_CLOCK */
	return(0);
#endif /* _HAVE_CLOCK */
}

static double
stir_rusage(int who, double entropy_estimate)
{
#ifdef HAVE_GETRUSAGE
	struct rusage ru;

	if (getrusage(who, &ru) == -1)
		return(0);

	RAND_add(&ru, sizeof(ru), entropy_estimate);

	return(entropy_estimate);
#else /* _HAVE_GETRUSAGE */
	return(0);
#endif /* _HAVE_GETRUSAGE */
}

static double
stir_from_system(void)
{
	double total_entropy_estimate;
	long int i;

	total_entropy_estimate = 0;

	i = getpid();
	RAND_add(&i, sizeof(i), 0.5);
	total_entropy_estimate += 0.1;

	i = getppid();
	RAND_add(&i, sizeof(i), 0.5);
	total_entropy_estimate += 0.1;

	i = getuid();
	RAND_add(&i, sizeof(i), 0.0);
	i = getgid();
	RAND_add(&i, sizeof(i), 0.0);

	total_entropy_estimate += stir_gettimeofday(1.0);
	total_entropy_estimate += stir_clock(0.5);
	total_entropy_estimate += stir_rusage(RUSAGE_SELF, 2.0);

	return(total_entropy_estimate);
}

static double
stir_from_programs(void)
{
	int i;
	int c;
	double entropy_estimate;
	double total_entropy_estimate;
	char hash[SHA_DIGEST_LENGTH];

	total_entropy_estimate = 0;
	for(i = 0; i < NUM_ENTROPY_RUNS; i++) {
		c = 0;
		while (entropy_sources[c].path != NULL) {

			if (!entropy_sources[c].badness) {
				/* Hash output from command */
				entropy_estimate = hash_output_from_command(&entropy_sources[c], hash);

				/* Scale back entropy estimate according to command's rate */
				entropy_estimate *= entropy_sources[c].rate;

				/* Upper bound of entropy estimate is SHA_DIGEST_LENGTH */
				if (entropy_estimate > SHA_DIGEST_LENGTH)
					entropy_estimate = SHA_DIGEST_LENGTH;

				/* Scale back estimates for subsequent passes through list */
				entropy_estimate /= SCALE_PER_RUN * (i + 1.0);

				/* Stir it in */
				RAND_add(hash, sizeof(hash), entropy_estimate);

#ifdef DEBUG_RAND
				debug3("Got %0.2f bytes of entropy from '%s'\n", entropy_estimate,
					entropy_sources[c].cmdstring);
#endif

				total_entropy_estimate += entropy_estimate;

			/* Execution times should be a little unpredictable */
				total_entropy_estimate += stir_gettimeofday(0.05);
				total_entropy_estimate += stir_clock(0.05);
				total_entropy_estimate += stir_rusage(RUSAGE_SELF, 0.1);
				total_entropy_estimate += stir_rusage(RUSAGE_CHILDREN, 0.1);
			} else {
#ifdef DEBUG_RAND
				debug2("Command '%s' disabled (badness %d)\n",
					entropy_sources[c].cmdstring, entropy_sources[c].badness);
#endif

				if (entropy_sources[c].badness > 0)
					entropy_sources[c].badness--;
			}

			c++;
		}
	}

	return(total_entropy_estimate);
}
