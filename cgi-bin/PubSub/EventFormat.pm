package PubSub::EventFormat;

# Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.
#
# @KNOWNOW_LICENSE_START@
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.
# 
# 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
# be used to endorse or promote any product without prior written
# permission from KnowNow, Inc.
# 
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# @KNOWNOW_LICENSE_END@
#
# $Id: EventFormat.pm,v 1.2 2003/03/22 11:05:12 ifindkarma Exp $

# NAME
# PubSub::EventFormat - Library for PubSub's filesystem handling.

# SYNOPSIS
# # Lots of stuff not covered here.
# use PubSub::EventFormat 'write_event', 'read_event';
# my $id = PubSub::UUID::uuid;
# PubSub::EventFormat::write_event('/what/chat', $id, {foo => 'bar'});
# PubSub::EventFormat::read_event('/what/chat', $id);
# # returns {foo => 'bar'} with some other stuff thrown in

# DESCRIPTION
# EventFormat.pm is the library PubSub uses for event persistence.

# FIXME: This interface contains too many functions and does too many things.

use strict;
use Fcntl ':flock';
use Exporter;
use base 'Exporter';
use vars '@EXPORT_OK';
use Carp 'cluck';

@EXPORT_OK = qw(write_event write_file read_event read_file
                event_pool_path build_path ensure_event_loc_exists
                to_rfc_822_format from_rfc_822_format subtopics routes
                is_heartbeat_stale fresh_heartbeat remove_heartbeat 
                make_stale_heartbeat event_names is_journal 
                crack_last_path_component topic_recurse new_events_on_topic
                server_urls);

# Change this to point to your event pool directory.
# FIXME: this should be in a common configuration file

my $topic_root_dir = '../kn_events';

# Read a list of self_url equivalents from a file named
# .names in the event pool directory.

sub server_urls
{
    my ($server_url) = @_;
    my $fname = event_pool_path(".names");
    my @rv = ();
    local *EQCLASSES;
    if (open EQCLASSES, "< $fname")
    {
        flock(EQCLASSES, LOCK_SH);
        @rv = <EQCLASSES>;
        close EQCLASSES;
        chomp @rv;
    }
    my $found = 0;
    for my $equivalent_url (@rv)
    {
        last if ($found = ($server_url eq $equivalent_url));
    }
    if (not $found)
    {
        open EQCLASSES, ">> $fname" or die "Can't append to $fname: $!";
        flock(EQCLASSES, LOCK_EX);
        seek(EQCLASSES, 0, 2);
        print EQCLASSES "$server_url\n";
        close EQCLASSES;
    }
    return @rv;
}

# Write an event in RFC-822 format or something similar.
# Expects a filename and a hash reference; dies if file can't be written.

sub write_event
{
    my ($topic, $eventname, $event) = @_;
    return write_file (event_pool_path($topic), $eventname, $event);
}

sub write_file
{
    my ($dir, $file, $event) = @_;

    cluck "Undefined filename" if not defined $file;

    die "Can't have / in filenames to write" if $file =~ m|/|;
    # Prepend .tmp- to the basename to obtain a temporary filename.
    my $tmpfile = "$dir/.tmp-$file";

    local *EVENT;
    open EVENT, "> $tmpfile" or die "can't open $tmpfile: $!\n";
    binmode EVENT;
    print EVENT to_rfc_822_format($event);
    close EVENT;
    
    unlink $file;
    if (not (rename $tmpfile, "$dir/$file" ))
    {
        unlink $tmpfile;
        die "can't rename $tmpfile to $dir/$file: $!\n";
    }

    my $index = index_file($dir);
    local *INDEX;
    open INDEX, ">> $index" or die "can't append to $index: $!\n";
    flock(INDEX, LOCK_EX);
    binmode INDEX;
    seek(INDEX, 0, 2);
    print INDEX "$file\n";
    close INDEX;
}

# Convert an event to RFC-822 format.

sub to_rfc_822_format
{
    my ($event) = @_;
    my $rv = '';

    for my $header (keys %$event)
    {
        next if ($header eq "kn_payload");
        my $value = $event->{$header};
        # escape dangerous characters in names and values
        for ($value, $header)
        {
            s/([\000-\037\177-\237:=]|^\s|\s$)/"=".sprintf("%2.2X",ord($1))/ge;
        }
        $rv .= "$header: " . $value . "\n";
    }
    my $value = $event->{"kn_payload"};
    $rv .= "\n" . $value;
    return $rv;
}

# Read an RFC-822 file into an event.
# Expects a filename;
# returns a hash reference on success, undef otherwise.
# FIXME: dies if the file is corrupt

sub read_event
{
    my ($topic, $eventname, $even_if_expired) = @_;
    return read_file(event_pool_path($topic), $eventname, $even_if_expired);
}

sub read_file
{
    my ($dir, $fname, $even_if_expired) = @_;

    cluck "Undefined filename" if not defined $fname;

    die "Can't have / in filenames to read" if $fname =~ m|/|;

    local *FILE;
    open FILE, "< $dir/$fname" or return undef;
    binmode FILE;

    my $content;
    # Maximum event file size is two megabytes.
    read FILE, $content, 1024 * 1024 * 2;

    # if the data appears to be corrupted, reread it in CR-LF mode
    if ($content =~ m|\A[^\n]*\r\n|)
    {
        close FILE;
        open FILE, "< $dir/$fname" or return undef;
        read FILE, $content, 1024 * 1024 * 2;
    }

    my $dummy;
    if (read FILE, $dummy, 1)
    {
        # File is too big!
        close FILE;
        return undef;
    }

    my $rv = from_rfc_822_format($content);

    # If we don't do this, the ->{} ||= below will autovivify $rv.
    return $rv if not defined $rv;
    
    my @stat = stat FILE;
    $rv->{kn_time_t} ||= $stat[9];

    close FILE;

    my $now = time();
    if (defined($rv->{'kn_expires'}) and
        $rv->{'kn_expires'} ne 'infinity' and
        $rv->{'kn_expires'} < $now and 
        not is_journal($dir))
    {
        # don't delete events that are less than thirty seconds old
        unlink "$dir/$fname" if $stat[9] < $now - 30;
        return undef unless $even_if_expired;
    }

    return $rv;
}

sub from_rfc_822_format
{
    my ($string) = @_;
    return undef unless defined $string;
    my ($headers, $body) = split /\n\n/, $string, 2;
    return undef unless defined $body;
    my %rv;

    for my $hdrline (split /\n/, $headers)
    {
        # Header names are separated from their values by colon.
        my ($name, $value) = split /:/, $hdrline, 2;
        die "invalid file format" if not defined $value;
        chomp $value;
        $value =~ s/^\s+//; # strip leading whitespace from values
        # Unescape names and values.
        for ($name, $value)
        {
            s/=([0-9A-F]{2})/pack("c", hex($1))/ge;
        }
        #$rv{lc $name} = $value;
        $rv{$name} = $value;
    }

    $rv{"kn_payload"} = $body;
    return \%rv;
}

# Concatenates /-separated path segments.
# Expects a list of one or more path segments;
# returns the concatenated path, reducing strings of /s at segment
# boundaries to a single / each.

sub build_path
{
    my ($first, $next, @rest) = @_;
    return $first if not defined $next;
    $first =~ s|/+$||; # remove trailing /s from the first segment
    $next =~ s|^/+||;  # and leading /s from the second segment
    return build_path ("$first/$next", @rest);
}

# Expects topic segments and optionally an event filename;
# returns a corresponding absolute pathname.

sub event_pool_path
{
    my $dir = build_path $topic_root_dir, @_;
    $dir =~ s|/+$||; # remove trailing /s
    return $dir;
}

# Builds the directory hierarchy for a given topic.
# Probably should be called ensure_topic_exists.

sub ensure_event_loc_exists
{
    my ($path, $new_dir_proc) = @_;
    my @dirs = split m|/|, $path;
    
    mkdir $topic_root_dir, 0777;
    index_file($topic_root_dir);
    for my $i (0..$#dirs)
    {
        my $topic = build_path @dirs[0..$i];
        my $dir = event_pool_path $topic;
        # FIXME: Be nice to check this for failure from, say, EPERM, 
        #        wouldn't it?
        #        If the directories already exist, the failure is nothing to 
        #        worry about --- it's just not creating a new topic.  But if
        #        they don't exist and we can't create them, it's an error.
        if (mkdir $dir, 0777)
        {
            index_file($dir);
            $new_dir_proc->($topic) if $new_dir_proc;
        }
    }
}

sub subtopics
{
    my ($topic) = @_;
    my $dir = event_pool_path $topic;
    local *DIR;
    opendir DIR, $dir or return ();
    my @files = readdir DIR;
    closedir DIR;

    # .dirs and "kn_routes" are not "subtopics".
    return sort grep { not /^\./ and -d "$dir/$_" and $_ ne "kn_routes" }
           @files;
}

# Route walk into cave. Or is it in to cave? Grog not know.

## (NOTE: the above message was found on the flagstone original of
## this program. A modern translation has been attempted below.)

# Expects as input a topic name.  Returns a list of events whose
# payloads contain absolute URIs that topic routes to.

sub routes
{
    my ($topic) = @_;
    my @routes;
    my $routedir = event_pool_path($topic, "kn_routes");
    local *ROUTES;
    (opendir ROUTES, $routedir) || return ();
    local $_;
    while (defined ($_ = readdir ROUTES))
    {
        my $route = PubSub::EventFormat::read_event("$topic/kn_routes", $_);
        next unless $route;     # skip directories and corrupted files 
        next if $route->{'kn_payload'} eq ""; # skip deleted routes
        push @routes, $route;
    }
    closedir ROUTES;
    return @routes;
}

# This is nasty, but turns out to be the most convenient interface.

sub event_names_and_dates
{
    my ($topic) = @_;
    my @files = event_names $topic;

    my $dir = event_pool_path $topic;
    return map { { name => $_, date => (stat "$dir/$_")[9] } } @files;
}

sub event_names
{
    my ($topic) = @_;
    my $dir = event_pool_path $topic;
    local *DIR;
    opendir DIR, $dir or return ();
    my @files = sort grep { not /^\./ and not -d "$dir/$_" } readdir DIR;
    closedir DIR;
    return @files;
}

# Index stuff

sub index_file 
{
    my ($dir) = @_;
    my $index = "$dir/.index";
    if (! stat $index) # this is for compatibility with old event pools
    {
        local $_;
        local *DIR;
        opendir DIR, $dir or die "can't opendir $dir: $!\n";
        # Hide .files, including . and ..
        my @files = grep !(/^\./ || -d "$dir/$_"), readdir DIR;
        closedir DIR;
        # Import the existing directory into the namelist
        local *INDEX;
        open INDEX, ">> $index" or die "can't initialize $index: $!\n";
        flock(INDEX, LOCK_EX);
        binmode INDEX;
        seek(INDEX, 0, 2);
        if (! tell INDEX) # make sure another index initializer didn't already write here
        {
            for (@files)
            {
                print INDEX "$_\n";
            }
        }
        close INDEX;
    }
    return $index;
}

# Return a list of the new or modified files on $topic.
# FIXME: This only has 1-second resolution on Unix (even worse elsewhere?).
# FIXME: This directory scanner should be moved into a separate module.
# FIXME: Use Date: header instead?

sub new_events_on_topic
{
    my ($topic, $state, $newest_date) = @_;
    my $dir = event_pool_path($topic);
    my @rv = ();
    local $_;
    local $/;  # slurp!
    if (! defined $state->{"buffer"})
    {
        my $index = index_file(event_pool_path($topic));
        $state->{"buffer"} = "";
        my @oldfiles = ();
        $state->{"files"} = \@oldfiles;
        $state->{"INDEX"} = \ (do { local *INDEX; });
        open($state->{"INDEX"}, "< $index") or die "can't read $index: $!\n";
        binmode($state->{"INDEX"});
    }
    my $curpos;
    $curpos = tell($state->{"INDEX"});
    $state->{"buffer"} .= readline(*{$state->{"INDEX"}});
    $curpos = tell($state->{"INDEX"});
    seek($state->{"INDEX"}, $curpos, 0);
    my $length = rindex $state->{"buffer"}, "\n";
    my @files = ();
    my @rest = ();
    if ($length > -1)
    {
        @files = split "\n", substr($state->{"buffer"}, 0, $length);
        $state->{"buffer"} = substr($state->{"buffer"}, $length + 1);
    }
    my $found = 0;
    if (defined $newest_date)
    {
        $found ||= $newest_date eq "-infinity";
    }
    my %indeces = ();
    for (@{$state->{"files"}}, @files)
    {
        my $date = (stat "$dir/$_")[9]; # or die "stat: $dir/$_: $!\n";
        if (defined $date)
        {
            if (defined $newest_date)
            {
                $found ||= $date > $newest_date;
            }
            if ($found)
            {
                push @rest, $_;
            }
            else
            {
                if (defined $indeces{$_})
                {
                    undef $rv[$indeces{$_}];
                }
                push @rv, $_;
                $indeces{$_} = $#rv;
            }
        }
    }
    @{$state->{"files"}} = @rest;
    return grep defined($_), @rv;
}

# Heartbeat stuff.

sub heartbeat_file_name 
{
    my ($topic) = @_;
    my $dir = event_pool_path $topic;
    return "$dir/.heartbeat";
}

# Given a topic or event name, return the parent topic and the event
# id or last segment of topic name.

sub crack_last_path_component
{
    my ($topic) = @_;
    for ($topic)
    {
        # Strip out strings of more than one slash.
        s|//+|/|g;
        # Strip out leading slash, if present.
        s|^/||;
        # Strip out trailing slash, if present.
        s|/\Z||;
    }
    if ($topic eq "")
    {
        # Sorry, can't crack last path component from zero-segment path!
        return ();  
    }
    elsif ($topic =~ m|^(.+)/(.+)\Z|)
    {
        return ($1, $2);
    }
    # So if the path consists of only non-slash chars (like, say, "chat").
    elsif ($topic =~ m|^[^/]+\Z|)
    {
        return ("", $topic);  # child of root topic
    }
    else
    {
        die "crack_last_path_component can't grok $topic, giving up";
    }
}

# Is the topic a journal?

sub is_journal
{
    my ($topic) = @_;
    my ($head, $tail) = crack_last_path_component ($topic);
    return defined($tail) ? ($tail eq "kn_journal") : 0;
}

# Check the heartbeat for a topic.

sub is_heartbeat_stale
{
    my ($topic) = @_;
    return undef unless is_journal $topic;
    my @stat = stat heartbeat_file_name $topic;

    return 1 unless @stat;

    my $age = time() - $stat[9];

    return $age > 100;
}

# Create a heartbeat with a recent mtime.

sub fresh_heartbeat
{
    my ($topic) = @_;
    if ($topic !~ /\/kn_journal$/)
    {
        die "Can't put a heartbeat into a non-kn_journal topic";
    }
    my $name = heartbeat_file_name($topic);
    local *HEARTBEAT;
    open HEARTBEAT, "> $name"
        or die "Can't create $name: $!";
    close HEARTBEAT;
}

# Ensure that there is no heartbeat, stale or otherwise --- for testing.

sub remove_heartbeat
{
    my ($topic) = @_;
    unlink heartbeat_file_name $topic;
}

# Make a stale heartbeat --- for testing.  Returns true on success,
# returns false or dies on failure.

sub make_stale_heartbeat
{
    my ($topic) = @_;
    fresh_heartbeat $topic;
    my $time = time() - 600;  # ten minutes ago
    my $name = heartbeat_file_name($topic);
    return utime $time, $time, $name;
}

sub topic_recurse
{
    my ($topic, $func) = @_;
    $func->($topic);
    my @subtopics = subtopics $topic;
    topic_recurse (build_path($topic, $_), $func) for @subtopics;
}

1;

# End of EventFormat.pm
