package PubSub::ModPubSub;

# Sample use with Apache 1.3 and mod_perl 1.0:

# LoadModule perl_module        libexec/httpd/libperl.so
# AddModule mod_perl.c
# <Perl>
# use lib "/var/www/mod_pubsub/cgi-bin";
# use PubSub::ModPubSub 'mod_pubsub';
# mod_pubsub("/var/www/mod_pubsub", "/mod_pubsub", "/kn",
#            \%Location, \%Directory);
# </Perl>

# Sample use with Apache 2.0 and mod_perl 2.0 (same as above except
# for the first three lines):

# LoadModule perl_module modules/mod_perl.so
# PerlModule Apache2
# <Perl >
# use lib "/var/www/mod_pubsub/cgi-bin";
# use PubSub::ModPubSub 'mod_pubsub';
# mod_pubsub("/var/www/mod_pubsub", "/mod_pubsub", "/kn",
#            \%Location, \%Directory);
# </Perl>

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
# $Id: ModPubSub.pm,v 1.2 2003/04/28 23:25:54 ifindkarma Exp $


use strict;

use Exporter;
use base 'Exporter';
use vars qw(@EXPORT_OK $cgi_url);
use Carp 'cluck';

@EXPORT_OK = qw(mod_pubsub handler);

# when in ModPerl2, use ModPerl1 compatibility layer
BEGIN {
    eval {
        require Apache::compat;
        import Apache::compat;
    };
}
use Apache::Constants qw(OK);

use CGI ();
use PubSub::Server qw(set_cgi_url set_docroot dispatch_request);
use PubSub::EventFormat 'set_topic_root_dir';

# initialize mod_pubsub configuration
sub mod_pubsub
{
    my ($mod_pubsub_dir, $mod_pubsub_url, $topic_root,
	$Location, $Directory) = @_;
    set_docroot $mod_pubsub_dir, $mod_pubsub_url;
    set_topic_root_dir $mod_pubsub_dir . "/kn_events";

    $Location->{$topic_root} =
    {
        SetHandler => "perl-script",
        PerlHandler => "PubSub::ModPubSub",
        Options => "ExecCGI",
        PerlSendHeader => "On",
    };

    $Directory->{$mod_pubsub_dir . "/cgi-bin"} =
    {
        Files =>
        {
            "pubsub.cgi" => $Location->{$topic_root},
            "*.cgi" =>
            {
                SetHandler => "perl-script",
                PerlHandler => "Apache::Registry",
                Options => "+ExecCGI",
                PerlSendHeader => "On",
            }
        }
    };

    $Directory->{$mod_pubsub_dir} =
    {
	AddType => '"text/html; charset=utf-8" .html'
    };
}

# handler for mod_pubsub requests
sub handler
{
    my $r = shift;
    my $now = scalar localtime;
    my $server_name = $r->server->server_hostname;

    # Main program logic starts here

    my $q = new CGI;
    set_cgi_url($q->url());

    dispatch_request($q);

    return OK;
}

1; # modules must return true
