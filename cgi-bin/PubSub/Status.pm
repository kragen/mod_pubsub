package PubSub::Status;

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
# $Id: Status.pm,v 1.1 2002/11/07 07:08:00 troutgirl Exp $

use strict;
use PubSub::UUID;

# Our output system works by sending events.
# This module generates the events we use to report request status.

sub new
{
    my ($class, $kn_status_from) = @_;
    my $kn_route_location = (defined $kn_status_from) ? $kn_status_from : "";
    return bless {
                  'kn_route_location' => $kn_route_location,
                  'status' => '200 OK',
                  'kn_payload' => '',
                  'kn_id' => PubSub::UUID::uuid()
                 }, $class;
}

sub status
{
    my ($self, $newstatus) = @_;
    $self->{'status'} = $newstatus;
}

sub send_to
{
    my ($self, $logger) = @_;
    $logger->send({%$self});
}

sub log
{
    my ($self, $msg) = @_;
    $self->{'kn_payload'} .= $msg;
}

1;

# End of Status.pm
