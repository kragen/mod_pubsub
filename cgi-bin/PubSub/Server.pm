package PubSub::Server;

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
# $Id: Server.pm,v 1.4 2003/04/25 22:58:40 ifindkarma Exp $

use strict;

use Exporter;
use base 'Exporter';
use vars qw(@EXPORT_OK $cgi_url $mod_pubsub_dir $mod_pubsub_url);
use Carp 'cluck';

@EXPORT_OK = qw(set_cgi_url dispatch_request set_docroot);

use CGI ();
use URI ();
use LWP::UserAgent;
use HTTP::Request::Common 'POST';
use PubSub::EventFormat qw(build_path ensure_event_loc_exists
                       is_heartbeat_stale fresh_heartbeat routes
                       crack_last_path_component remove_heartbeat 
                       make_stale_heartbeat is_journal new_events_on_topic
                       server_urls);
use PubSub::UUID qw(uuid);
use PubSub::Topic;
use PubSub::Status;
use PubSub::Htmlsafe 'entify';
use PubSub::ReplayEvents 'begin_replaying_events';
use Carp 'cluck';

$mod_pubsub_dir = ".."; # mod_pubsub root on the filesystem

$mod_pubsub_url = ".."; # mod_pubsub root in URL-space

sub set_docroot
{
    ($mod_pubsub_dir, $mod_pubsub_url) = @_;
}

# Check a topic name for forbidden characters.
# Expects a local topic name and a description for use in error messages;
# returns an error if forbidden characters are used in the topic name,
# or undef if it's OK.
# FIXME: this makes Ben uncomfortable, because it's far too restrictive.

sub is_bad_topic_name
{
    my ($topic) = @_;
    # FIXME: currently we only allow -, /, _ and alphanumeric characters
    # and, occasionally, .
    # (we also allow all characters with the eighth bit set)
    if ($topic =~ m|([^-/_a-zA-Z0-9.\200-\377])|)
    {
        return "contains bad char '$1'";
    }
    if ($topic =~ m:(^|/)\.: or $topic =~ m:\.($|/):)
    {
        return "contains . in bad place: $topic";
    }
    return undef;
}

# Prepends / to relative topic names, leaves others untouched.
# Expects an absolute URI or a topic name;
# returns absolute URIs unchanged, and topic names prefixed by / .

sub canonicalize_topic
{
    my ($topic) = @_;
    
    if (is_relative_topic_name($topic) and $topic !~ m|^/|)
    {
        return "/$topic";
    }
    else
    {
        return $topic;
    }
}

sub is_javascript_dest
{
    my ($dest) = @_;
    return ($dest =~ m|^javascript:|i);
}

# Need to recognize https: urls, too

sub is_relative_topic_name
{
    my ($dest) = @_;
    return ($dest !~ m|^http.?://|i and !is_javascript_dest($dest));
}

sub is_bad_dest
{
    my ($dest) = @_;
    # rfc2068 section 3.2 says essentially that the part of a URL
    # after the scheme can contain anything at all except for space,
    # control characters, and ["#<>].
    # FIXME: This makes Kragen uncomfortable.
    if ($dest =~ m|([\x00-\x20\"\#<>])|)
    {
        return "remote destination URL contains bad char ".entify($1);
    }
    else
    {
        return undef;
    }
}


# Stuff for posting new events.

# In this context, an "event" is a hash reference. the "kn_payload" key
# corresponds with the message body, and other keys (including "kn_id")
# correspond to RFC-822 headers.

# Is this event a duplicate? returns undef for journals unless UUIDs aren't.
# Currently, duplicate events must be identical in every field except
# kn_route_*, kn_routed_from.

sub is_duplicate
{
    my ($event, $topic) = @_;
    my $old_event = PubSub::EventFormat::read_event($topic, $event->{'kn_id'}, 
                                                'even_if_expired');
    return undef unless $old_event;

    # less efficient than it could be
    for my $key (keys %$event, keys %$old_event) {
        die "key is undef" if not defined $key;
        if (not ($key =~ m|\Akn_route_|) and
            not in_list($key, qw(kn_routed_from kn_time_t)))
        {
            return 0 if not defined $event->{$key};
            return 0 if not defined $old_event->{$key};
            return 0 if $event->{$key} ne $old_event->{$key};
        }
    }
    return 1;
}

# Given an absolute URL, if it names a topic this server controls, turn
# it into a topic name.  If it doesn't, return undef.
# Given a relative topic name, simply return it.

sub local_url_to_topic
{
    my ($own_url, $topic_url) = @_;
    if ($topic_url =~ /^\w+:/)
    {
        for my $url (server_urls $own_url)
        {
            $url .= "/";
            # topic = subtract own $url from the beginning of $topic_url
            # FIXME: this should only compare domain names case-insensitively
            if ($topic_url =~ s/^\Q$url\E//i) 
            {
                return "/$topic_url";
            }
        }
        return undef;
    }
    else
    {
        return $topic_url;
    }
}

# Post an event to an HTTP URI.

# Expects a logger with which to perform output, a URI to post to, an
# event to post to it, a source topic that is forwarding the event to the
# URI, and the event ID of the route that is causing this forwarding.

# FIXME: too many parameters!
# FIXME: there should be separate routines for different URI schemes

sub post_remote_event
{
    my ($q, $logger, $url, $event) = @_;

    # Short-circuit posting off-host by checking if we can instead
    # recurse -- saves spinning up a new CGI
    
    my $topic = local_url_to_topic $cgi_url, $url;
    if ($topic)
    {
        return undef if is_bad_topic_name $topic;
        return undef if is_heartbeat_stale $topic;
        post_and_forward($q, $topic, $logger, $event);
        return 1;
    }
    else
    {
        my $ua = LWP::UserAgent->new;
        # currently only HTTP URLs are supported
        # we use %{{stuff}} to get rid of duplicated keys; specifically, to
        # remove any kn_route_location from %$event.
        $url =~ s/^https:/http:/i;
        my $result = $ua->request(POST $url, [ %$event ]);
        $logger->log("$cgi_url posted to $url; response: ".
                     $result->as_string."\n");
        return $result->code !~ /^4/;
    }
}

sub make_new_topic_sensor
{
    my ($q, $logger) = @_;
    return sub 
    {
        my ($topic) = @_;
        my ($basetopic, $subtopic) = crack_last_path_component $topic;
        return if not $subtopic;
        return if $subtopic eq "kn_subtopics";
        post_and_forward($q, "$basetopic/kn_subtopics", $logger, 
                         { kn_id => uuid, kn_payload => $subtopic });
    };
}

sub post_event 
{
    my ($q, $logger, $topic, $event) = @_;

    ensure_event_loc_exists $topic, make_new_topic_sensor($q, $logger);
    
    my $filename;
    if (is_journal $topic)
    {
        $filename = uuid();
        cluck "uuid() returned undef" if not defined $filename;
    } 
    else                        # it's a queue
    {
        $filename = $event->{'kn_id'};
        cluck "event has no kn_id" if not defined $filename;
    }
    
    return undef if is_duplicate($event, $topic);
    
    PubSub::EventFormat::write_event($topic, $filename, $event);
    $logger->log("Wrote to '$topic' ($filename)\n");
    return 'success';
}


sub in_list
{
    my ($word, @list) = @_;
    return ${{(map { $_ => 1 } @list)}}{$word};
}

# Returns a hash of the first value for each CGI parameter by value.

sub get_extra_keywords
{
    my ($cgi_obj) = @_;
    my %rv = ();
    for my $key ($cgi_obj->param())
    {
        if (!in_list($key, qw(do_method kn_from kn_to
                              kn_debug
                              kn_status_from kn_status_to
                              kn_response_format)))
        {
            $rv{$key} = $cgi_obj->param($key);
        }
    }
    return %rv;
}

sub use_filter
{
    my ($filter, $event) = @_;
    my ($uri, $keyword) = split / /, $filter, 2;
    $keyword ||= "kn_payload";
    my $result = LWP::UserAgent->new->request(POST $uri, 
                                              [$keyword => 
                                               $event->{'kn_payload'}]);
    if ($result->code eq '200')
    {
        return {%$event, "kn_payload" => $result->content()};
    }
    else
    {
        return undef;
    }
}

sub absolute_kn_expires
{
    my ($expiry) = @_;
    return ($expiry =~ s/^\+(\d+)/$1/) ? $expiry + time() : $expiry;
}

# Posts events onto topics with forwarding.

sub post_and_forward 
{
    my ($q, $topic, $logger, $event) = @_;

    cluck "trying to post undef event" if not defined $event;
    cluck "trying to post event without id" if not defined $event->{'kn_id'};
    # should we possibly move kn_id defaulting into this function?
    $event->{'kn_expires'} = absolute_kn_expires($event->{'kn_expires'}) 
        if (exists $event->{'kn_expires'} and $event->{'kn_expires'} ne 'infinity');
    $event->{'kn_time_t'} = time if not exists $event->{'kn_time_t'};
    $event->{'kn_route_checkpoint'} = uuid();
    
    post_event($q, $logger, $topic, $event) or return 0;

    my @routes = routes($topic);
    my $route;
    foreach $route (@routes)
    {
        forward($q, $topic, $event, $logger, $route);
    }
    return 1;
}

# We have been notified of an incoming event.

sub notify
{
    $| = 1;
    my ($q, $status_topic, $status) = @_;
    my $topic = $q->param('kn_to');
    $topic = $q->path_info() if not defined $topic;
    if (not defined $topic)
    {
        $status->status("400 No param 'kn_to' for notify request");
        $status->log("Notify requests require either path info\n".
                     "or a 'kn_to' parameter indicating the topic\n".
                     "on which to post the event.  The notify request\n".
                     "did not contain this parameter or any path info.");
        $status->send_to($status_topic);
        return;
    }
    # In case we've been handed an absolute URL for kn_to.
    $topic = local_url_to_topic($cgi_url, $topic);
    if (not defined $topic)
    {
        $status->status("403 Proxying not supported by this server");
        $status->log("The route request specified a URL on another server," .
                     " '$topic', as the route source.  At present,".
                     " creating routes on other servers is not supported.");
        $status->send_to($status_topic);
        return;
    }

    $topic = canonicalize_topic($topic);

    my $topic_name_badness = is_bad_topic_name($topic);
    if ($topic_name_badness)
    {
        $status->status('400 bad posting destination topic name');
        $status->log("Bad posting topic name '$topic': $topic_name_badness");
        $status->send_to($status_topic);
        return;
    }
    my %event = get_extra_keywords($q);
    $event{'kn_payload'} = "" if not defined $event{'kn_payload'};

    if (is_heartbeat_stale $topic)
    {
        $status->status("404 Expired");
        $status->log("Expired: $topic");
        $status->send_to($status_topic);
        return;
    }
    else
    {
        my ($head, $tail) = crack_last_path_component $topic;

        if (in_list($tail, 'kn_subtopics', 'kn_routes'))
        {
            $status->status("400 Can't post there");
            $status->log("You can't post to a $tail topic");
            $status->send_to($status_topic);
            return;
        }
        else
        {
            $event{'kn_id'} = uuid() unless defined($event{'kn_id'});
            post_and_forward($q, $topic, $status, \%event);
        }
    }
    $status->status('200 Notified');
    $status->send_to($status_topic);
}

# Handles a persistent connection to a "hidden" frame with JavaScript
# callbacks; does pretty-printing of the hidden frame contents.

sub route_to_javascript
{
    my ($q, $status_topic, $status, $src, $attributes) = @_;

    my $max_n = $attributes->{'do_max_n'};
    my $max_age = $attributes->{'do_max_age'};
    my $since_checkpoint = $attributes->{'do_since_checkpoint'};

    my $need_irp = (defined $max_n or defined $max_age or defined $since_checkpoint);
    if ($need_irp)
    {
        $max_age = 'infinity' unless defined $max_age;
    }
    else
    {
        $max_age = '0';
    }

    $| = 1;

    my $expires;
    if (exists $attributes->{'kn_expires'} and
	$attributes->{'kn_expires'} ne 'infinity')
    {
        $expires = absolute_kn_expires($attributes->{'kn_expires'});
    }

    ensure_event_loc_exists($src, 
                            make_new_topic_sensor($q, $status));
    fresh_heartbeat $src;

    $status_topic->is_watching();
    $status->status("200 Watching topic");
    $status->log("Watching for events in topic '$src'.");
    $status->send_to($status_topic);

    $status_topic->tickle_renderer();
    
    my %state = ();
    
    # Check once before anything happens.
    new_events_on_topic($src, \%state,
                        (($max_age eq "0")
                         ? undef
                         : (($max_age eq "infinity")
                            ? "-infinity"
                            : (time() - $max_age))));
    
    # We print a space every 0.5s if nothing else happens.
    # This counts the waits.
    my $nwait = 0;
    for (;;)
    {
        if (defined $expires and time() > $expires)
        {
            make_stale_heartbeat $src;
            $status_topic->close();
            return;
        }
        my @files = new_events_on_topic $src, \%state, undef;
        if ($need_irp)
        {
            $need_irp = 0;
            if (defined $max_n)
            {
                if (@files > $max_n)
                {
                    @files = @files[($#files - $max_n + 1)..
                                    $#files ];
                }
            }
        }
        my @checkpointed;
        for my $file (@files)
        {
            my $contents = PubSub::EventFormat::read_event $src, $file;
            next unless $contents;
            if (defined $since_checkpoint)
            {
                push @checkpointed, $contents;
                if ($contents->{'kn_route_checkpoint'} eq $since_checkpoint)
                {
                    $since_checkpoint = undef;
                }
                next;
            }
            $status_topic->send($contents);
        }
        if (defined $since_checkpoint)
        {
            $since_checkpoint = undef;
            for my $contents (@checkpointed)
            {
                $status_topic->send($contents);
            }
        }
        $status_topic->tickle_renderer() if @files;
        # This is to avoid eating our server's CPU time
        my $polling_interval = 0.05; # seconds
        select(undef, undef, undef, $polling_interval);
        # keep-alive:
        # This is here to ensure that the script exits soon after the browser
        # closes the connection.  It's supposed to run every 30 seconds.
        $nwait = ($nwait + 1) % (0.5 / $polling_interval);
        if ($nwait == 0)
        {
            $status_topic->verify_aliveness() or return;
            fresh_heartbeat $src;
        }
    }
}

# Forward an event $event from a topic $src along route $route, logging to 
# logger $logger.

sub forward
{
    my ($q, $src, $event, $logger, $route) = @_;
    my $filter = $route->{'kn_content_filter'};
    my $transform = $route->{'kn_content_transform'};
    
    return if $filter and not ($event->{'kn_payload'} =~ $filter);
    
    my $new_event = $transform ? use_filter($transform, $event) : $event;
    return unless $new_event;

    # this because sometimes this function gets called with noncanonical topics.
    $src = canonicalize_topic($src);
    # FIXME: here we may be modifying the input event.  This is bad.
    # FIXME: kn_uri should be required, according to Kragen
    $new_event->{'kn_route_location'} = $route->{'kn_uri'} ||
        ("$cgi_url$src/kn_routes/$route->{'kn_id'}");
    $new_event->{'kn_route_id'} = $route->{'kn_id'};
    $new_event->{'kn_routed_from'} = "$cgi_url$src";

    # try to post the event along this route, and delete the route
    # if that fails ("route poisoning")
    post_remote_event($q, $logger, $route->{'kn_payload'}, $new_event)
        ||
            post_and_forward($q, build_path($src, "kn_routes"),
                             $logger,
                             ( {
                                 "kn_id" => $route->{'kn_id'},
                                 "kn_payload" => "",
                                 # this next number is intended to make it
                                 # stick around only long enough to be 
                                 # guaranteed to be delivered to interested
                                 # web browsers
                                 "kn_expires" => "+300"
                                 }
                               )
                             );
}


# Add a static route to another KN server.

sub add_static_route
{
    my ($q, $status_topic, $status, $src, $dest, %route_attributes) = @_;

    my $max_n = $route_attributes{'do_max_n'};
    my $max_age = $route_attributes{'do_max_age'};
    my $since_checkpoint = $route_attributes{'do_since_checkpoint'};

    if (defined $since_checkpoint)
    {
        die "do_since_checkpoint is not supported for static routes";
    }

    my $need_irp = (defined $max_n or defined $max_age);
    if ($need_irp)
    {
        $max_age = 'infinity' unless defined $max_age;
    }
    else
    {
        $max_age = '0';
    }

    if (is_journal $src)
    {
        $status->status("403 Unsupported Route Request");
        $status->log("Unsupported Route Request\n" .
                     "You can't create a static route from a kn_journal\n" .
                     "topic.");
        $status->send_to($status_topic);
        return;
    }
    
    # FIXME: move this up to &route; possibly pass a hash or hash ref
    my $id = $q->param('kn_id') || uuid();

    my $route = {
        "kn_id" => $id,
        "kn_payload" => $dest,
        %route_attributes
        };

    if (not post_and_forward($q, build_path($src, "kn_routes"), 
                             $status, $route))
    {
        $status->status('200 Ignored route');
        $status->log("was a duplicate\n");
        $status->send_to($status_topic);
        return;
    }

    # Initial route population
    if ($need_irp)
    {
        my %state = ();

        # Check once before anything happens.
        new_events_on_topic($src, \%state,
                            (($max_age eq "0")
                             ? undef
                             : (($max_age eq "infinity")
                                ? "-infinity"
                                : (time() - $max_age))));

        my @files = new_events_on_topic $src, \%state, undef;

        if (defined $max_n)
        {
            if (@files > $max_n)
            {
                @files = @files[($#files - $max_n + 1)..
                                $#files ];
            }
        }
        for my $file (@files)
        {
            my $event = PubSub::EventFormat::read_event $src, $file;
            next unless $event;

            forward($q, $src, $event, $status, $route);
        }
    }
    $status->status('200 Routed');
    $status->send_to($status_topic);
}

sub route
{
    my ($q, $status_topic, $status) = @_;
    my $src = $q->param('kn_from');
    $src = $q->path_info() if not defined $src;
    if (not defined $src)
    {
        $status->status("400 No param 'kn_from' for route request");
        $status->log("Route requests require either path info\n".
                     "or a 'kn_from' parameter indicating the topic\n".
                     "on which to create the route.  The route request\n".
                     "did not contain this parameter or any path info.");
        $status->send_to($status_topic);
        return;
    }

    my $dest = $q->param('kn_to');

    my $local_topic_src = local_url_to_topic($cgi_url, $src);
    if (not defined $local_topic_src)
    {
        $status->status("403 Proxying not supported by this server");
        $status->log("The route request specified a URL on another server," .
                     " '$local_topic_src', as the route source.  At present,".
                     " creating routes on other servers is not supported.");
        $status->send_to($status_topic);
        return;
    }

    my %route_attributes = get_extra_keywords($q);
    
    $local_topic_src = canonicalize_topic($local_topic_src);

    my $bad_source = is_bad_topic_name($local_topic_src);
    if ($bad_source)
    {
        $status->status("400 Bad source topic name");
        $status->log("Bad source topic name for route ".
                     "request '$src': $bad_source");
        $status->send_to($status_topic);
        return;
    }

    if (not defined $dest or is_javascript_dest($dest))
    {
        route_to_javascript($q, $status_topic, $status, 
                            $local_topic_src, \%route_attributes);
    }
    else
    {
        my $bad_dest = is_bad_dest($dest);
        if ($bad_dest)
        {
            $status->status("400 Bad destination topic name");
            $status->log("Bad destination topic URI for route ".
                         "request '$dest': $bad_dest");
            $status->send_to($status_topic);
            return;
        }

        # This code makes the topic name absolute.
        if (is_relative_topic_name($dest) and $dest ne "")
        {               
            $dest = build_path($cgi_url, $dest);
        }
        
        add_static_route($q, $status_topic, $status, 
                         $local_topic_src, $dest, %route_attributes);
    }
}

sub help
{
    my ($q) = @_;
    my $relurl = URI->new("$mod_pubsub_url/kn_docs/");
    my $absurl = $relurl->abs($cgi_url);
    print $q->header(-status=>"302 Moved",
                     -location=>$absurl,
                     -uri=>$absurl,
                     -url=>$absurl,
                     -type=>'text/html; charset=utf-8');
    print $q->start_html(-head=><<HEAD, -title=><<TITLE), $q->end_html;
<p>This document has moved to <a href="$absurl">$absurl</a>.</p>
HEAD
Moved
TITLE
}

# FIXME: do_method=replay is currently broken.

sub replay
{
    my ($q, $status_topic, $status_event) = @_;
    my $topic = $q->param('kn_from') || $q->path_info;
    my $warp = $q->param('warp') || 1;
    my $user = $q->param('user');
    my $password = $q->param('password');
    begin_replaying_events($topic, $warp, $user, $password, $mod_pubsub_dir);
    $status_event->log("Replaying... ");
    $status_event->send_to($status_topic);
}

sub blank
{
    my ($q) = @_;
    print $q->header(-type=>'text/html; charset=utf-8',
                     -expires=>'+1y');
    print $q->start_html(-head=><<HEAD, -title=><<TITLE), $q->end_html;
<script type="text/javascript">
<!--
if (parent.kn_redrawCallback) { parent.kn_redrawCallback(window); }
else { document.write('<body bgcolor="white"></body>'); }
// -->
</script>
HEAD
This Space Intentionally Left Blank
TITLE
}

# FIXME: One day lib should be parameterized to give the app developer
# the option of autoloading modules from kn_apps/kn_lib, such as
# cookie_auth.js for authentication.

sub lib
{
    my ($q) = @_;
    whoami($q);
    print "\n";
    open(LIB , "$mod_pubsub_dir/kn_apps/kn_lib/pubsub.js")
        or print "alert('Error: No PubSub Library Available');";
    print <LIB>;
    close(LIB);
}

sub libform
{
    my ($q) = @_;
    lib($q);
    print "\n";
    open(LIBFORM , "$mod_pubsub_dir/kn_apps/kn_lib/form.js")
        or print "alert('Error: No PubSub Form Library Available');";
    print <LIBFORM>;
    close(LIBFORM);
}

sub lib2form
{
    my ($q) = @_;
    lib($q);
    print "\n";
    print "window.kn__form2way = true;\n";
    open(LIBFORM , "$mod_pubsub_dir/kn_apps/kn_lib/form.js")
        or print "alert('Error: No PubSub Form Library Available');";
    print <LIBFORM>;
    close(LIBFORM);
}

# Some apps don't need authentication, so autoloading the cookie_auth.js
# from whoami would be intrusive.

sub whoami
{
    my ($q) = @_;
    print $q->header(-type=>'application/x-javascript',
                     -expires=>'+1d');
    my $kn_userid = $q->remote_user();
    my $kn_displayname = $q->user_name();
    my $kn_server = $q->script_name();
    if (!$kn_userid) 
    {
        $kn_userid = 'anonymous'; 
        $kn_displayname = 'Anonymous User'; 
    }
    if (!$kn_displayname) 
    {
        $kn_displayname = $kn_userid;
    }
    for ($kn_userid, $kn_displayname, $kn_server)
    {
        s/([\\""])/\\$1/g;
        s/\f/\\f/g;
        s/\n/\\n/g;
        s/\r/\\r/g;
        s/\t/\\t/g;
        s/(<|[^ -~])/sprintf("\\x%2.2x",ord($1))/ge;
    }
    print("window.kn_userid = '$kn_userid' ;",
          "window.kn_displayname = '$kn_displayname' ;",
          "window.kn_server = '$kn_server' ;");
}

# Unpack several requests.

sub batch
{
    my ($q, $status_topic, $status) = @_;
    $status->status('200 Processing batch');
    $status->send_to($status_topic);
    for my $req ($q->param('kn_batch'))
    {
        my $req_q = new CGI($req);
        my $req_status = PubSub::Status->new($req_q->param('kn_status_from'));
        my $method = $req_q->param('do_method');
        eval {
            if ($method eq 'notify')
            {
                notify($req_q, $status_topic, $req_status);
            }
            elsif ($method eq 'route')
            {
                route($req_q, $status_topic, $req_status);
            }
            elsif ($method eq 'replay')
            {
                replay($req_q, $status_topic, $req_status);
            }
            else
            {
                $req_status->status("400 can't batch that");
                $req_status->log("You can't batch '$method' requests");
                $req_status->send_to($status_topic);
            }
        };
        if ($@)
        {
            $req_status->status("500 Internal Server Error, Uncaught Exception");
            $req_status->log("\nUncaught exception for $method: $@\n");
            print STDERR "Uncaught exception for $method: $@\n";
            $req_status->send_to($status_topic);
        }
    }
}

# Handle a request of unknown do_method.

sub unknown
{
    my ($q, $status_topic, $status) = @_;
    my $method = $q->param('do_method');
    $status->status('403 Unknown do_method');
    $status->log("Unknown do_method '$method'");
    $status->send_to($status_topic);
}

# The next several functions are self-test code run when 'do_method' is 'test'.

sub run_rfc822_test
{
    my ($q) = @_;
    my $event =
    {
        "header1" => "value1",
        "headertwo" => "two",
        "anotherheader" => "anothervalue",
        "anotherHeader" => "anotherValue",
        "  a SPACED\nheader " => " a spaced-out:\n\tvalue\0 ",
        "special=20header" => "special=3Dvalue",
        "queer: header" => "queer: value",
        "kn_payload" => "testpayload",
        "pid" => $$
        };
    my $topic = '/what/apps/pubsub.cgi/rfc822_test';
    ensure_event_loc_exists($topic, 
                            make_new_topic_sensor($q, new PubSub::Status()));
    my $id = uuid();
    PubSub::EventFormat::write_event($topic, $id, $event);
    
    my $new_event = PubSub::EventFormat::read_event($topic, $id);
    
    if (not defined $new_event)
    {
        print "<p> FAILURE: new event doesn't exist\n";
        return;
    }
    
    for my $key (keys %$event)
    {
        if (not exists ($new_event->{$key}))
        {
            print "<p> FAILURE: new event doesn't contain key $key\n";
            return;
        }
        elsif ($new_event->{$key} ne $event->{$key} and $key ne 'kn_time_t')
        {
            print "<p> FAILURE: new event for `$key' doesn't match \n",
            "original value: `$new_event->{$key}' ne `$event->{$key}'\n";
            return;
        }
    }
    
    for my $key (keys %$new_event)
    {
        if (not exists ($event->{$key}) and $key ne 'kn_time_t')
        {
            print "<p> FAILURE: new event contains new key $key\n";
            return;
        }
    }
    
    print "<p> Success.\n";
}

sub run_rfc822_read_nonexistent_file_test
{
    if (defined (PubSub::EventFormat::read_event("/what/apps/pubsub.cgi/rfc822_test", uuid)))
    {
        print "<p> FAILURE: PubSub::EventFormat::read_event doesn't notice " .
            "when asked to read a nonexistent file.\n";
    }
    else
    {
        print "<p> Success.\n";
    }
}

sub run_entify_test
{
    # The UTF-8 sequence here spells "kosme" in Greek.
    my $teststring =
        "<foo&bar>\"\xCE\xBA\xE1\xBD\xB9\xCF\x83\xCE\xBC\xCE\xB5\"";
    my $modified_teststring = entify $teststring;
    if ($modified_teststring ne
        '&lt;foo&amp;bar&gt;&quot;&#954;&#8057;&#963;&#956;&#949;&quot;')
    {
        print "<p> FAILURE: entified(\"$teststring\") is " .
            "\"$modified_teststring\"\n";
    }
    else
    {
        print "<p> Success.\n";
    }
}

# Verify that a particular topic is not a bad topic.

sub test_good_topic
{
    my ($topic) = @_;
    if (is_bad_topic_name($topic))
    {
        print "<p> FAILURE: $topic is not a valid topic\n";
        return 0;
    }
    else
    {
        return 1;
    }
}

# Verify that a particular topic is a bad topic.

sub test_bad_topic
{
    my ($topic) = @_;
    if (is_bad_topic_name($topic))
    {
        return 1;
    }
    else
    {
        print "<p> FAILURE: $topic is a valid topic\n";
        return 0;
    }
}

sub test_topic_validity
{
    # All of these are legal topic names.
    for my $topic ("/", "eatme", "/eat/me", "fu.schnicken", "1_2_3.4", 
                   "GOGOGOGO", "dashed-name", "www.example.com",
                   "\xCE\xBA\xE1\xBD\xB9\xCF\x83\xCE\xBC\xCE\xB5")
    {
        if (!test_good_topic $topic)
        {
            return;
        }
    }
    
    # All of these are illegal topic names; unfortunately com1: is not
    # protected by checking for the colon, because omitting the colon
    # still leaves a destructive filename.
    for my $topic ("..", "foo.", "com1:", ".foobity",
                   "/foo/bar/../../../../etc/passwd", "...",
                   "My Topic")
    {
        if (!test_bad_topic $topic)
        {
            return;
        }
    }
    print "<p> Success.\n";
}

sub test_heartbeat
{
    eval 
    {
        my ($q) = @_;
        my $hbname = "/what/apps/pubsub.cgi/heartbeat_from/kn_journal";
        ensure_event_loc_exists($hbname);
        my $hburl = "$cgi_url/$hbname";
        
        my $logger = PubSub::Status->new();
        
        remove_heartbeat $hbname;
        my $success = post_remote_event($q, $logger, $hburl, 
                                        ( { kn_id => uuid(), kn_payload => 'some stuff' } ));
        if ($success)
        {
            print "<p> FAILURE: posting event succeeded when no heartbeat\n";
            return;
        }
        
        fresh_heartbeat $hbname;
        $success = post_remote_event($q, $logger, $hburl, 
                                     ( { kn_id => uuid(), 
                                         kn_payload => 'some more stuff' } ));
        if (not $success)
        {
            print "<p> FAILURE: posting remote event with heartbeat failed\n";
            return;
        }
        
        make_stale_heartbeat $hbname or do {
            print "<p> FAILURE: couldn't make stale $hbname heartbeat: $!\n";
            return;
        };
        
        $success = post_remote_event($q, $logger, $hburl, 
                                     ( { kn_id => uuid(), kn_payload => 'failing stuff' } ));
        if ($success)
        {
            print "<p> FAILURE: posted successfully to stale topic\n";
            return;
        }
        
        print "<p> Success.\n";
    };
    if ($@)
    {
        print "<p> FAILURE: died: $@\n";
    }
}

sub test_crack_last_path_component
{
    my ($x, $y) = crack_last_path_component "a/b/c/d";
    if ($x ne "a/b/c" or $y ne "d")
    {
        print "<p> FAILURE: cracking a/b/c/d gave $x and $y";
        return;
    }

    ($x, $y) = crack_last_path_component "/a/b/c/d/";
    if ($x ne "a/b/c" or $y ne "d")
    {
        print "<p> FAILURE: cracking /a/b/c/d/ gave $x and $y";
        return;
    }
    
    ($x, $y) = crack_last_path_component "fooby";
    if ($x ne "" or $y ne "fooby")
    {
        print "<p> FAILURE: cracking fooby gave $x and $y";
        return;
    }

    ($x, $y) = crack_last_path_component "/";
    if (defined $x or defined $y)
    {
        print "<p> FAILURE: cracking / gave $x and $y";
        return;
    }

    ($x, $y) = crack_last_path_component "///////";
    if (defined $x or defined $y)
    {
        print "<p> FAILURE: cracking lotsa slashes gave $x and $y";
        return;
    }

    ($x, $y) = crack_last_path_component "/////x/////";
    if ($x ne "" or $y ne "x")
    {
        print "<p> FAILURE: cracking /////x///// gave $x and $y";
        return;
    }

    ($x, $y) = crack_last_path_component "///a/////b////c";
    if ($x ne "a/b" or $y ne "c")
    {
        print "<p> FAILURE: cracking ///a/////b////c gave $x and $y";
        return;
    }

    print "<p> Success.\n";
}


sub run_tests
{
    my ($q) = @_;
    print $q->header('text/html; charset=utf-8'),
    $q->start_html(-title=>"test results for pubsub.cgi");
    print $q->h1('RFC822 test');
    run_rfc822_test($q);
    print $q->h1('RFC822 nonexistent file test');
    run_rfc822_read_nonexistent_file_test();
    print $q->h1('entify test');
    run_entify_test();
    print $q->h1('topic validity test');
    test_topic_validity();
    print $q->h1('heartbeat test');
    test_heartbeat($q);
    print $q->h1('crack_last_path_component test');
    test_crack_last_path_component();
}


sub set_cgi_url
{
    ($cgi_url) = @_;
}

sub dispatch_request
{
    my ($q) = @_;

    # we support only GET and POST
    my $http_method = $q->request_method();
    if (($http_method ne 'GET') and
        ($http_method ne 'POST'))
    {
        my $error = "501 Not Implemented";
        my $desc = "The method you requested is unimplemented for this URI.";
        print($q->header(-type => 'text/html; charset=utf-8',
                         -status => $error));
        if ($http_method ne "HEAD")
        {
            print($q->start_html(-title => $error),
                  $q->h1($error),
                  $q->p($desc),
                  $q->end_html());
        }
        return;
    }
    if (not $q->param('do_method'))
    {
        if (!$q->param()) 
        {
            $q->param('do_method', 'help');
        }
        else
        {
            if ($http_method eq "GET")
            {
                $q->param('do_method', 'route');
            }
            else
            {
                $q->param('do_method', 'notify');
            }
        }
    }
    # Dispatch things that don't use status_topics
    my $method = ({
        'whoami' => \&whoami,
        'lib' => \&lib,
        'libform' => \&libform,
        'lib2form' => \&lib2form,
        'blank' => \&blank,
        'help' => \&help,
        'test'  => \&run_tests,
    }->{$q->param('do_method')});
    if ($method)
    {
        $method->($q);
    }
    else
    {
        # Then there are things that do use status_topics
        my $method = ({
            'route' => \&route,
            'batch' => \&batch,
            'notify' => \&notify,
            'replay' => \&replay
            }->{$q->param('do_method')});
        $method ||= \&unknown;
        
        my $status_topic;
        $status_topic = new PubSub::Topic($q, \&post_remote_event);
        my $event = $status_topic->status_event();
        # This is how Perl spells "try".  This bothers some people.
        eval 
        {
            # Make sure this server name is known to our event pool
            server_urls($cgi_url);

            $method->($q, $status_topic, $event);
        };
        # ... and this is how Perl spells "catch", which is even worse.
        if ($@)
        {
            $event->status("500 Internal Server Error, Uncaught Exception");
            $event->log("\nUncaught exception: $@");
            print STDERR "Uncaught exception: $@\n";
            $event->send_to($status_topic);
        }
        $status_topic->close();
    }
}

1;
# End of PubSub::Server
