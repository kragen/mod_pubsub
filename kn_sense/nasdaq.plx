#!/usr/bin/perl -w

# Copyright 2000-2002 KnowNow, Inc.  All Rights Reserved.
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
# $Id: nasdaq.plx,v 1.1 2002/11/07 07:09:27 troutgirl Exp $

# NOTE: If you run this Nasdaq sensor for a while from any domain,
# Nasdaq will block you.  This script is provided here to showcase
# functionality, not to serve as your gateway to free stock quotes.

use strict;
use LWP::UserAgent;
use LWP::Simple 'get';
use HTTP::Request::Common 'POST';
#use HTTP::Request::Common 'GET';
use lib '../cgi-bin';
use PubSub::Client; # PubSub Perl library.

my $serveruri = shift (@ARGV) || 'http://localhost/kn/';
my $desttopic = '/what/nasdaq';

my $interval = 0;  # seconds
my $intrastockwait = 0;  # seconds

# Stock symbols to monitor for changes.

my @watch_list = sort();
@watch_list = @ARGV if @ARGV;

my $ua = LWP::UserAgent->new;
my $next_time = time + $interval;
my %old_data;
my @datum = ();
my $evcount = 0;

# Give me N elements starting at index S.

sub ashift {
    my ($s, $n, @a) = @_;
    my @result = ();
    for (my $i = 0; ($i < $s) and (defined $a[0]); $i++) {
        shift @a;
    }
    for (my $i = 0; ($i < $n) and (defined $a[0]); $i++) {
        push @result, shift @a;
    }
    return @result;
}

# Array rm, remove all instances of a string from an array.

sub arm {
    my ($what, @a) = @_;
    my @result = ();
    for my $i (@a) {
        if ($i ne $what) {
            @result = (@result, $i);
        }
    }
    return @result;
}

sub post_datum {
    my ($sym, $tradetime, $value, $tagname, $topname) = @_;

    my $ssym = $sym;
    $ssym =~ tr[A-Z][a-z];

    if ((not defined $old_data{$sym}{$tagname} or
	$old_data{$sym}{$tagname} ne $value) and $value ne "0") {
	printf("%5s\t%16s\t%10s\n", $sym, $tagname, $value);
        my $topicid = unpack("%32V*","$sym $tagname");
        my $exp = time + (48 * 60 * 60); # Save events for 2 days.
	my $urlev = "do_method=notify&kn_to=$desttopic/$ssym/$topname&displayname=$ssym&symbol=$ssym&type=$topname&trade-datetime=$tradetime&kn_payload=$value&kn_id=$topicid&kn_expires=$exp";
	push @datum, $urlev;
	$old_data{$sym}{$tagname} = $value;
    }
}

sub post_all_datum {
    if (@datum > 0) {
	my $req = POST $serveruri, [ do_method => 'batch', 
				 kn_response_format => 'simple',
				 map { (kn_batch => $_) } @datum ];
	my $result = $ua->request($req);
	if ($result->code !~ /^[123]/) {
	    warn "Failed to post to $serveruri: " . $result->as_string;
	}
	$evcount += @datum;
    }
    @datum=();
}

my $last_time = time;
my $cycle_time = 0;

my $serv;

# Watch for stock requests.

sub setup_subscription {
    $serv  = new PubSub::Client($serveruri);
    $serv->subscribe("/what/nasdaq/kn_subtopics", sub {
	my ($event) = @_;
	add_to_watchlist($event->{kn_payload});
    }, { do_max_age => "infinity" });
}
setup_subscription();

sub add_to_watchlist {
    my ($sym) = @_;
    push @watch_list, $sym;
}

sub fetch_10syms {
    my (@watch_list) = @_;

    my $qsyms = "";
    for my $sym (@watch_list)
    {
	$qsyms = "$qsyms&symbol=$sym";
    }
    my $req = "http://quotes.nasdaq.com/quote.dll?page=custom&mode=stock$qsyms";
#    print "requesting: $req\n";
#    my $result = $ua->request(new HTTP::Request('GET', $req));
#    if ($result->code !~ /^[123]/) {
#        warn "NASDAQ.com not responding when requesting from quotes.nasdaq.com?quote.dll\n";
#        sleep(5);
#        next;
#    }

    my $result = get $req;
    if (! defined $result) {
	print "NASDAQ.com not responding when requesting from quotes.nasdaq.com?quote.dll\n";
        sleep(5);
	next;
    }

    # XXX: Actually it looks like quotes.dll will take
    # at most 10 symbols at once, bummer.
#    print "result: \"$result\"\n";
    my @result = split /\n/, $result;
    pop @result;
    return @result;
}

for (;;)
{
    $serv->handle_events() if defined $serv; # Let PubSub handle events.

    if (@watch_list == 0) {
        sleep (5);
	next;
    }
    my $offset = 0;
    my $count = @watch_list;

    if ($offset + 10 >= $count) {
        $offset = 0;
    }
    while ($offset < $count)
    {
        my @next10 = ashift($offset, 10, @watch_list);
        my @lines = fetch_10syms(@next10);
        my $linei = 0;
        for my $line (@lines) {
#           print "line: \"$line\"\n";
            my @info = split /\|/, $line;
            my $sym = $next10[$linei++];
            if (@info <= 1 || $info[1] eq "") {
                print "NASDAQ.com has no data for $sym.\n";
                @watch_list = arm($sym, @watch_list);
                # XXX poison /what/nasdaq/$sym (is that possible?)
                next;
            }

            my $symbol = $info[0];
            my $symbol_name = $info[1];
            my $tradetime = $info[2] . " " . $info[3];
            my @tmp = split /\*/, $info[4]; my $bid = $tmp[0];
            @tmp = split /\*/, $info[5]; my $ask = $tmp[0];
            ; # what is [6]?
            @tmp = split /\*/, $info[7]; my $previous_close = $tmp[0];
            @tmp = split /\*/, $info[8]; my $todays_high = $tmp[0];
            @tmp = split /\*/, $info[9]; my $todays_low = $tmp[0];
            @tmp = split /\*/, $info[10]; my $last = $tmp[0];
            my $price_change = $info[12];
            my $vol = $info[13];

            post_datum($sym, $tradetime, $last, 'last-sale-price', 'last');
            post_datum($sym, $tradetime, $bid, 'best-bid-price', 'bid');
            post_datum($sym, $tradetime, $ask, 'best-ask-price', 'ask');
            post_datum($sym, $tradetime, $vol, 'share-volume-qty', 'vol');
            post_datum($sym, $tradetime, $previous_close, 'previous-close', 'close');
            post_datum($sym, $tradetime, $price_change, 'price-change', 'change');

#            print "$symbol last: $last bid: $bid ask: $ask vol: $vol close: $previous_close ($tradetime)\n";
        }
        post_all_datum();
        sleep ($intrastockwait);
        $offset += 10;
    }
    my $now = time;
    my $sleep_amount = $next_time - $now;
    if ($now < $next_time) {
        sleep ($next_time - $now);
    }
    $next_time = time + $interval;
    $cycle_time = ($now - $last_time) - $sleep_amount - ($intrastockwait * @watch_list);
    $last_time = $now;
    print "------------------------------ (" . $cycle_time . " sec to update " . @watch_list . " symbols) [" . $evcount . "]\n";
}

# End of nasdaq.plx
