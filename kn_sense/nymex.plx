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
# $Id: nymex.plx,v 1.2 2004/04/19 05:39:14 bsittler Exp $

use strict;
use IO::Socket;
use URI::URL;
use LWP::Simple 'get';
use LWP::UserAgent;
use HTTP::Request::Common 'POST';
use lib '../cgi-bin';
use PubSub::Client; # PubSub Perl library.

my $server_uri = shift (@ARGV) || 'http://localhost/kn/';
my $serv = new PubSub::Client($server_uri);
my $desttopic = '/what/nymex';

my $interval = 0;  # seconds
my $intrastockwait = 0;  # seconds
my $ua = LWP::UserAgent->new;
my $next_time = time + $interval;
my @datum = ();
my $evcount = 0;
my $routes_need_updating = 0;
my $debug = 1;
my %old;

# NYMEX markets to monitor for changes.

my %markets;
%markets = (
               "SC" => "Brent Crude Oil",
               "WC" => "COB Electricity",
               "QL" => "Central Appalachian Coal",
               "CN" => "Cinergy Electricity",
               "NT" => "Entergy Electricity",
               "HO" => "Heating Oil",
               "NG" => "Henry Hub Natural Gas",
               "CL" => "Light Sweet Crude Oil",
               "HU" => "New York Harbor Unleaded Gasoline",
               "QJ" => "PJM Electricity",
               "WV" => "Palo Verde Electricity",
               "PL" => "Propane" );

my @market_keys = sort(keys %markets);

my $last_time = time;
my $cycle_time = 0;

## FUNCTIONS

sub dump_market_info {
    my (@i) = @_;
    print "$i[0] $i[1] high: $i[2] low: $i[3] last: $i[4] change: $i[5] date: $i[6] in $markets{$i[0]}\n";
}

sub post_market_data {
    my ($market, $contract, $open, $high, $low, $last, $change, $time) = @_;

    if (not defined $old{$market}{$contract}{'open'} or
        $old{$market}{$contract}{'open'} ne $open) {
        $old{$market}{$contract}{'open'} = $open;
        print "\t" . $market . $contract . "\topen\t$open\n";
        post_market_datum('open', $market, $contract, $open, $time);
    }
    if (not defined $old{$market}{$contract}{'high'} or
        $old{$market}{$contract}{'high'} ne $high) {
        $old{$market}{$contract}{'high'} = $high;
        print "\t" . $market . $contract . "\thigh\t$high\n";
        post_market_datum('high', $market, $contract, $high, $time);
    }
    if (not defined $old{$market}{$contract}{'low'} or
        $old{$market}{$contract}{'low'} ne $low) {
        $old{$market}{$contract}{'low'} = $low;
        print "\t" . $market . $contract . "\tlow\t$low\n";
        post_market_datum('low', $market, $contract, $low, $time);
    }
    if (not defined $old{$market}{$contract}{'last'} or
        $old{$market}{$contract}{'last'} ne $last) {
        $old{$market}{$contract}{'last'} = $last;
        print "\t" . $market . $contract . "\tlast\t$last\n";
        post_market_datum('last', $market, $contract, $last, $time);
    }
    if (not defined $old{$market}{$contract}{'change'} or
        $old{$market}{$contract}{'change'} ne $change) {
        $old{$market}{$contract}{'change'} = $change;
        print "\t" . $market . $contract . "\tchange\t$change\n";
        post_market_datum('change', $market, $contract, $change, $time);
    }
}

sub post_market_datum {
    my ($what, $market, $contract, $info, $time) = @_;

    my $topic = "$desttopic/$market/$contract/$what";
    my $uid = unpack("%32V*","$topic");
    my %e = (
             kn_id => $uid,
             kn_expires => "+9600",
             kn_payload => $info,
             displayname => "$markets{$market}",
             tradetime => "$time",
             market => "$market",
             contract => "$contract",
             type => "$what"
            );
    $serv->publish($topic, \%e);
}


## MAIN

for (;;)
{

  for my $market (@market_keys) {
      print "============= $markets{$market}\n";
      my $m_url = "http://quotes.ino.com/exchanges/?r=NYMEX_$market";

      my $html_result = get $m_url;

      if (! defined $html_result) {
          print "ino.com not responding when requesting $market\n";
          sleep(5);
          next;
      }

      my $contract = "";
      my $date = "";
      my $open = "";
      my $high = "";
      my $low = "";
      my $last = "";
      my $change = "";
      my $time = "";

      print "result: \"$html_result\"\n" if $debug > 10;

      my $state = 0;
      my @lines = split /\n/, $html_result;
      for my $line (@lines) {
          $_ = $line;
          if ($state == 0) {
              if (/NYMEX_$market/) {
                  if (/[A-Z][A-Z]([A-Z][0-9])/) {
                      $contract = $1;
                  }
                  $state = 7;
                  next;
              }
          }
          if ($state == 7) {
              $line =~ /">(.*)<\/A>/; #"
              $date = $1;
              $state = 6;
              next;
          }
          if ($state == 6) {
              $line =~ />([0-9]+.[0-9]+)</;
              $open = $1 if (defined $1);
              $state = 5;
              next;
          }
          if ($state == 5) {
              $line =~ />([0-9]+.[0-9]+)</;
              $high = $1 if (defined $1);
              $state = 4;
              next;
          }
          if ($state == 4) {
              $line =~ />([0-9]+.[0-9]+)</;
              $low = $1 if (defined $1);
              $state = 3;
              next;
          }
          if ($state == 3) {
              $line =~ />([0-9]+.[0-9]+)</;
              $last = $1 if (defined $1);
              $state = 2;
              next;
          }
          if ($state == 2) {
              $line =~ /2>(.*)<\/FONT>/;
              $change = $1 if (defined $1);
              $state = 1;
              next;
          }
          if ($state == 1) {
              $line =~ /1>(.*)<\/FONT>/;
              $time = $1 if (defined $1);
              $state = 0;
              post_market_data($market, $contract, $open, $high, $low, $last, $change, $time);
#              dump_market_info($market, $contract, $open, $high, $low, $last, $change, $time);
              next;
          }
      }
  }
}

# End of nymex.plx
