#!/usr/bin/perl -w

# Copyright 2000-2004 KnowNow, Inc.  All rights reserved.
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
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 
# 3. Neither the name of the KnowNow, Inc., nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# @KNOWNOW_LICENSE_END@
# 
#
# $Id: rss.plx,v 1.3 2004/04/19 05:39:14 bsittler Exp $

# To run the RSS feed:
#   rss.plx -url ... [-rsslist ...] [-log ...] [-user ... -password ...]

# Where:
#   1. url is the mod_pubsub instance where new stories are POSTed
#   2. rsslist has the RSS sources polled every 5 minutes (default: rss.rsslist)
#   3. log is the logfile (default: rss.log)
#   4. user and password are used to authenticate POSTing to the URL

# Example:
#   rss.plx -url http://dogfood.example.com/cgi-bin/pubsub.cgi -user kncgi -password kncgi -log rss.log -rsslist rss.rsslist

# This program sends structured data:
#    RSS data in the headers, and a summary thereof in kn_payload.

# This program goes through the rsslist once.  Run it as a cron job
# for best results...

use strict;
use LWP::Simple;
use LWP::UserAgent;
use RSSLite;
use HTTP::Request::Common;
use IO::Handle;
use Time::localtime;

my %params = @ARGV;

my $version = '$Id: rss.plx,v 1.3 2004/04/19 05:39:14 bsittler Exp $';
my $postURL = $params{-url} or die "No URL specified with -url.\n";
my $rssList = $params{-rsslist} || "rss.rsslist";
my $logfile = $params{-log} || "rss.log";
my $user = $params{-user};
my $password = $params{-password};

print "RSS Sensor $version \n\n";
open(LOG, ">> $logfile") or die "Couldn't open $logfile for writing: $!\n";
LOG->autoflush(1);
print "Appending to log: $logfile\n";
log_msg("\n\nRSS Sensor $version \n");
print "Using RSS list in: $rssList\n";
log_msg("Using RSS list in: $rssList");

if ($user && $password) {
    print "Using user $user and password $password\n\n";
    log_msg("Using user $user and password $password\n");
    $postURL =~ s/(.*\/\/)(.*)/$1$user:$password\@$2/ ; 

}

my $ua = new LWP::UserAgent;

# Goes through the rsslist once.  Run as a cron job.

open(RSSLIST, "< $rssList") or die "Can't open $rssList: $!\n";
while (<RSSLIST>) 
{
    chomp;
    next if /^\#/;   # Skip lines that start with '#'
    next if /^\s*$/; # and blank ones too
    my ($feedURL, $topic, @other_stuff) = split;
    $topic ||= calculate_topic($feedURL);
    process_rss($feedURL, $topic, $ua);
}
close(RSSLIST);


# Create a route from the specified topic to /what/rss/all idempotently.

my %created_route;
sub create_route
{
    my ($ua, $dest, $topic) = @_;
    return if $created_route{$topic};

    my $id = unpack("%C*", $topic);
    log_msg("Creating route for $topic with id $id");
    my $result = $ua->request(GET "$dest?do_method=route&kn_to=$postURL/what/rss/all&kn_id=$id&kn_debug=1");
    log_msg("Got result: " .  $result->as_string) unless $result->code =~ /^2/;

    $created_route{$topic} = 1;
}

# Give the event in xml format for use in kn_payload.

sub rss_summary 
{
    my ($source, $item) = @_;

    my $xfrag = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n".
	        "<rss version=\"0.91\">\n".
	        "<channel>\n".
                ($source ? "  <title>$source</title>\n" : "").
	        "  <item>\n".
                ($item->{title} ? "    <title>$item->{title}</title>\n" : "").
                ($item->{link} ? "    <link>$item->{link}</link>\n" : "").
                ($item->{description} ? "    <description>$item->{description}</description>\n" : "").
	        "  </item>\n".
                "</channel>\n".
					"</rss>\n";

    return $xfrag;
}

# Return the event, parsed from RSS format.

sub build_event
{
    my ($source, $item) = @_;

    for my $key (keys %$item)
    {
        $item->{$key} =~ s/&lt;/</g;
        $item->{$key} =~ s/&gt;/>/g;
        $item->{$key} =~ s/&amp;/&/g;
    }

    return {
        ('kn_id' => unpack("%C*", $source.($item->{link} || "").($item->{title} || "") )),
        ('rss_source' => $source),
        ($item->{title} ? ('rss_title' => $item->{title}) : ()),
        ($item->{description} ? ('rss_description' => $item->{description}) : ()),
        ('content-type' => 'text/xml'),
        ('kn_payload' => rss_summary($source, $item)),
        ('kn_expires' => '+300'),
        ($item->{link} ? ('rss_link' => $item->{link}) : ()),
        ('displayname' => 'RSS')
    };
}

# Log time and message.

sub log_msg 
{
    my ($s) = @_;

    # The current time formatted like: 2000-09-01 15:56:21.

    my $tm = localtime;
    my $now = sprintf("%04d-%02d-%02d %02d:%02d:%02d", $tm->year+1900, 
             ($tm->mon)+1, $tm->mday, $tm->hour, $tm->min, $tm->sec);

    print LOG $now . ": $s\n";
}

# Given an RSS URL, calculate the topic to which stories from the URL
# should be posted (in addition to /what/rss-all).

sub calculate_topic
{
    my ($rssURL) = @_;

    if ($rssURL =~ m|^http://www\.egroups\.com/messages/(.*)\?rss=1|)
    {
        return "egroups.com/what/$1";
    }

    if ($rssURL =~ m|^http://my\.theinfo\.org/rss/nytimes\.tcl\?category=(.*)|)
    {
        return "theinfo.org/what/nytimes/$1";
    }

    if ($rssURL =~ m|^http://my\.theinfo\.org/rss/(.*)\.tcl$|)
    {
        return "theinfo.org/what/$1";
    }

    if ($rssURL =~ m|^http://weblogs\.userland\.com/(.*)/xml/rss.xml$|)
    {
        return "weblogs.userland.com/what/$1";
    }

    if ($rssURL =~ m|^http://channels\.epinions\.com/rss_channels/(.*)\.rdf$|)
    {
        return "epinions.com/what/$1";
    }

    if ($rssURL =~ m|^http://p\.moreover\.com/cgi-local/page\?index_(.*)\+rss$|)
    {
        return "moreover.com/what/$1";
    }

    ### General case, for things for which we don't have specific patterns.

    # Remove initial http: if possible.
    $rssURL =~ s|^http:/*||;

    # Remove chars not valid in topic names.
    $rssURL =~ s|[^A-Za-z0-9/]|_|g;

    # Collapse strings of slashes.
    $rssURL =~ s|//+|/|g;

    # Remove initial slashes; we'll append this to a string with a slash
    # at the end anyway.
    $rssURL =~ s|^/*||;

    return $rssURL;
}

# Retrieve the given RSS field.

sub process_rss 
{
    my ($feedURL, $topic, $ua) = @_;
    if ($feedURL =~ /\s/)
    {
        my $msg = "Got URL containing whitespace: '$feedURL', quitting";
        log_msg($msg);
        die $msg;
    }

    log_msg("Retrieving channel @ $feedURL, posting to $topic");
    my $dest = "$postURL/$topic";
    create_route $ua, $dest, $topic;

    my $content = get($feedURL) or return;
    my %result;

    eval { parseXML(\%result, \$content); };

    if ($@)
    {
        log_msg("Failed to parse $feedURL:\n$content");
        return;
    }

    log_msg("Processing channel \"$result{'title'}\"");

    my $item;
    foreach $item (@{$result{'items'}}) 
    {
        next if not defined ($item->{link});
        log_msg("$item->{link}");
	$item->{link} =~ s|^([a-zA-Z0-9_]+://[^/]+/)/+|$1|;
        my $story = build_event($result{title}, $item);
        my $result = $ua->request(POST $dest, [ %$story ]);
        log_msg("Failed to post to $dest: " . $result->as_string)
            if $result->code !~ /^2/;
    }
}

# End of rss.plx
