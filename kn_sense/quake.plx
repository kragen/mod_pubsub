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
# $Id: quake.plx,v 1.2 2004/04/19 05:39:14 bsittler Exp $

# http://localhost/kn_apps/chat/?kn_topic=/what/quake
# http://wwwneic.cr.usgs.gov/neis/bulletin/bulletin.html

use strict;
use Net::Finger;
use lib '../cgi-bin';
use PubSub::Client; # PubSub Perl library.


## GLOBALS

my $server_uri = shift @ARGV || 'http://localhost/kn/';
my $topic = '/what/quake';
my $fingername = 'quake@gldfs.cr.usgs.gov';
my %old;
my $serv = new PubSub::Client($server_uri);


## FUNCTIONS

sub dump_hash {
    my (%e) = @_;
    my @ekeys = keys %e;
    for my $key (@ekeys) {
        print "$key => $e{$key}\n";
    }
}

sub dump_quake_info {
    my (@i) = @_;

    print "$i[0] $i[1]\n";
    print "\tlat\t\t$i[2]\n";
    print "\tlon\t\t$i[3]\n";
    print "\tdepth\t\t$i[4]\n";
    print "\tmagnitude\t$i[5]\n";
    print "\tq\t\t$i[6]\n";
    print "\tcomments\t$i[7]\n";
}

sub quake_infoToString {
    my (@i) = @_;
    return "M $i[5] EQ in $i[7] Z=$i[4]km $i[1] UTC $i[2] $i[3]";
}


## MAIN

while(1) {

  # You can put the response in a scalar...

  my @lines = finger($fingername, 1);
  unless (@lines) {
      warn "Finger problem: $Net::Finger::error contacting $fingername";
      sleep(5 * 60);
      next;
  }

  for my $line (@lines) {
      $_ = $line;
      if (m/^[0-9][0-9]\//) {
          my @info = m/([^ ]+)[ ]+([^ ]+)[ ]+([^ ]+)[ ]+([^ ]+)[ ]+([^ ]+)[ ]+([^ ]+)[ ]+([^ ]+)[ ]+(.*)/;
          $info[5] =~ s/M//;
          my @db = split /\//, $info[0];
          $info[0] = "$db[1]/$db[2]/20$db[0]";
          my $uid = unpack("%32V*","$line");
          if (not defined $old{$uid}) {
              $old{$uid} = $line;
              my %e = (
                    kn_id => $uid,
                    kn_expires => "+9600",
                    kn_payload => quake_infoToString(@info),
                    displayname => "$info[0] $info[1](UTC)",
                    date => "$info[0]",
                    time => "$info[1]",
                    lat => "$info[2]",
                    lon => "$info[3]",
                    depth => "$info[4]",
                    magnitude => "$info[5]",
                    q => "$info[6]\n",
                    comment => "$info[7]",
                   );
              $serv->publish($topic, \%e);

              print "$info[0] $info[1]UTC ($info[2]/$info[3] $info[4]km $info[5]) $info[7]\n";
#              dump_quake_info(@info);
          }
      }
  }
  sleep(3 * 60);
}

# End of quake.plx
