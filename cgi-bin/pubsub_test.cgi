#!/usr/bin/perl -w

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
# $Id: pubsub_test.cgi,v 1.3 2003/03/25 06:04:26 ifindkarma Exp $

use strict;
use PubSub::UUID;
use PubSub::Event;
use PubSub::WebHelper 'encode_form';
use PubSub::TopicState;
use CGI ':standard';
use LWP::UserAgent;
use LWP::Simple (); # Don't import anything! 
use HTTP::Request::Common 'POST';
use URI;
use IO::Socket;
use vars qw($url $current_test %tests @tests $session_id %serial $ua
            $usr $pw $jsoutput);

$url = param("url"); # This eventually ends up as the URL to post to.
if (defined $url)
{
    # make sure we convert a relative URL to an absolute one
    $url = "" . URI->new_abs($url, url);
}
$usr = param("user");
$pw = param("password");
$jsoutput = param('jsoutput');
my $base_url = $url; # This arguably shouldn't be used anywhere.

if ((not defined $url) or (not $url))
{
  $url = url;
  # We assume pubsub.cgi is in the same directory as pubsub_test.cgi
  $url =~ s:[^/]*$:pubsub.cgi:;
  $base_url = $url;
  if ($usr && $pw) { $url =~ s/(.*\/\/)(.*)/$1$usr:$pw\@$2/ ; }
}
PubSub::Event::kn_url_is $url;
PubSub::TopicState::kn_url_is $url;

$current_test = 'no_current_test';

%tests = ();
@tests = ();

# Define a named test, which must return true for success, false or
# die on failure.  Name should not exceed 40 chars.

sub make_test
{
    my ($name, $sub) = @_;
    $tests{$name} = $sub;
    push @tests, $name;
}

# We can't put inline classes in files to be run by mod_perl, so I'll
# use a closure instead.

sub make_output
{
    my ($name_is, $succeed, $fail) = @_;
    return sub {
        my $methodname = shift @_;
        if ($methodname eq 'name_is') {
            goto &$name_is;
        } elsif ($methodname eq 'succeed') {
            goto &$succeed;
        } elsif ($methodname eq 'fail') {
            goto &$fail;
        }
    };
}

# Some basic character-based output.

sub basic_output
{
    my ($output) = @_;
    return make_output (sub {  # name_is
        $output->(sprintf "%40.40s: ", shift);
    }, sub { # succeed
        $output->("OK\n");
    }, sub { # fail
        $output->("FAILED ($_[0])\n");
    });
}

sub js_script_frag
{
    my ($str) = @_;
    return <<end_of_script . (' ' x 4096);
<script type="text/javascript"><!--
$str
// -->
</script>
end_of_script
}

# Return a JavaScript representation of the string.

sub js_quote
{
    my ($str) = @_;
    for ($str)
    {
        s/\\/\\\\/g;
        s/([""])/\\$1/g;
        s/\n/\\n/g;
        s/\r/\\r/g;
        s/([\x00-\x1f\x7f-\xff])/sprintf('\\x%02x', ord($1))/ge;
    }
    return qq("$str");
}


# JavaScript output for the test suite.

sub js_output_formatter
{
    my ($output) = @_;
    return make_output(
        sub { # name_is
            my $name = js_quote($_[0]);
            $output->(js_script_frag(qq(kn_test_name=$name)));
        }, sub { # succeed
            $output->(
                js_script_frag('parent.kn_test_frame.subsucceed(kn_test_name);'));
        }, sub { # fail
            my $msg = js_quote($_[0]);
            $output->(
             js_script_frag(
             qq(parent.kn_test_frame.subfail({name: kn_test_name, msg: $msg})))
                     );
        });
}        

sub run_test 
{
    my ($name, $output) = @_;
    $output->('name_is', $name);
    my $sub = $tests{$name};
    $current_test = $name;
    $current_test =~ s/\W/_/g;
    my $test_success = eval { $sub->()};
    $current_test = "no_current_test_again";
    if ($@) 
    {
        $output->('fail', $@);
        return 0;
    }
    elsif ($test_success)
    {
        $output->('succeed');
        return 1;
    }
    else 
    {
        $output->('fail', "returned false");
        return 0;
    }
}


sub interactive_run_test
{
    my ($name) = @_;
    $| = 1;
    my $output_formatter = \&basic_output;
    if (defined $jsoutput)
    {
        $output_formatter = \&js_output_formatter;
    }
    if ($tests{$name})
    {
        return run_test($name, $output_formatter->(sub { 
                            print(@_) || die "stdout: $!" 
                        }));
    }
    else
    {
        print "Don't know test `$name' --- try one of ", 
        join (", ", @tests), "\n";
    }
}

sub fork_test
{
    my ($name) = @_;
    $| = 1;
    my $pid = fork;
    if (not defined $pid)
    {
        die "Failed to fork: $!\n";
    }
    elsif ($pid == 0) # child
    {
        my $output = "";
        run_test($name, basic_output sub { 
                     $output .= join '', @_ 
                 });
        print($output) || die "stdout: $!";
        exit;
    }
    else # parent
    {
        # Do nothing in particular.
    }
}

sub test_header
{
    my (@tests) = @_;
    return "Running " . @tests . " tests:\n";
}

sub run_all_tests 
{
    my $start = time();
    print(header('text/html; charset=utf-8'),
          start_html(-title => "PubSub test results from " . localtime($start),
                     -text => "black",
                     -bgcolor => "white"), 
          h1(test_header @tests), "<ol>\n");

    if (defined $jsoutput) {
        # We add 1 because of the fake subtest at the end reporting runtime.
        print js_script_frag('top.kn_test_frame.expect_subtests(' . 
                             (@tests + 1) . ')');
    }
    
    my $successes = 0;
    for my $test (@tests) 
    {
        print "<li>";
        interactive_run_test($test) and $successes ++;
        print "</li>";
    }
    print "</ol>";
    my $end = time();
    my $pct = 100.0 * $successes / @tests;
    my $time = $end - $start;
    if (defined $jsoutput) {
        print js_script_frag(sprintf
                             'top.kn_test_frame.subsucceed("server-side ' .
                             'tests completed; took %d seconds");',
                             $time);
    } else {
        print "<p>Tests completed at " . localtime($end), ".</p>\n";
        printf "<p>%3.2f%% successful; took %d seconds</p>\n", $pct, $time;
    }
    if ($successes == @tests)
    {
        print <<end_of_success;
<script type="text/javascript"><!--
top.kn_test_frame && top.kn_test_frame.succeed();
// -->
</script>
end_of_success
    }
    else
    {
        print <<end_of_failure;
<p><big><big>CODE BROKEN</big></big></p>
<script type="text/javascript"><!--
top.kn_test_frame && top.kn_test_frame.fail("see above");
// -->
</script>
end_of_failure
    }
    print end_html;
}

# FIXME: Not currently up to snuff --- produces plain-text output
# instead of HTML, doesn't produce statistics, and doesn't produce
# JavaScript callbacks.

sub fork_all_tests
{
    print header('text/plain'), test_header @tests;
    my $start = time();
    
    my @remaining_tests = @tests;

    # This number is the number of tests to run in parallel.  At
    # present, 4, 8, 16, and 32 all take 67-68 seconds; 64 takes 71
    # seconds.  So I chose 4, as it makes the software work mostly
    # deterministically without sacrificing too much performance.

    for (1..4)
    {
        fork_test(pop @remaining_tests) if @remaining_tests;
    }
    while (wait != -1) 
    {
        fork_test(pop @remaining_tests) if @remaining_tests;
    }
    my $done = time();
    my $len = $done - $start;
    print "\nComplete, $len seconds.";
}

$session_id = uuid;
%serial = ();
sub get_topic
{
    my $name = $current_test;
    $serial{$name} ||= 0;
    $serial{$name}++;
    return "/what/apps/pubsub_test.cgi/$session_id/$name/$serial{$name}";
}

sub get_journal
{
    my ($name) = @_;
    return get_topic . "/kn_journal";
}

sub really_check_events
{
    my ($equalp, $topic_state, @expected_events) = @_;
    my @caller = caller;
    my @new_events = $topic_state->new_events;
    my $topic = $topic_state->topic;
    
    for my $new_event (@new_events)
    {
        for my $expected_event (@expected_events)
        {
            next if not defined $expected_event;
            if ($new_event and $equalp->($new_event, $expected_event))
            {
                # Remove them both from their respective lists.
                $new_event = undef;
                $expected_event = undef;
                last;
            }
        }
    }
    
    for my $new_event (@new_events)
    {
        if (defined $new_event)
        {
            die "$topic: Unexpected new event, kn_id=" . $new_event->kn_id . " at $caller[1] line $caller[2]\n";
        }
    }
    
    for my $expected_event (@expected_events)
    {
        if (defined $expected_event)
        {
            die "$topic: Didn't get expected event, kn_id=" . 
                ($expected_event->kn_id || "(null)") . " at $caller[1] line $caller[2]\n";
        }
    }
}

sub event_equality
{
    my ($a, $b) = @_;
    return $a->kn_id eq $b->kn_id and $a->kn_payload eq $b->kn_payload;
}

sub expect_events 
{
    unshift @_, \&event_equality;
    goto &really_check_events;
}

$ua = new LWP::UserAgent;
sub route 
{
    my ($src, $dest, %parameters) = @_;
    my $req = POST $url, [ do_method => 'route', 
                           kn_from => $src, 
                           kn_to => $dest, 
                           %parameters ];
    # print "req is " . $req->as_string;
    my $result = $ua->request($req);    
    die "bad result routing on ${url} $src -> $dest: " . 
      $result->as_string
        unless $result->code =~ /^2/;
    warn $result->as_string if $parameters{"kn_debug"};
}

sub delete_route
{
    my ($src, $rid, %parameters) = @_;
    route($src, '', kn_id => $rid, %parameters);
}

sub unique_event
{
    return new PubSub::Event ("kn_id" => uuid, "kn_payload" => uuid);
}

make_test "test", sub { 1; };

make_test "server self test", sub
{
    my $response = $ua->request(POST $url, [do_method => "test"]);
    if ($response->code ne '200')
    {
    die "Couldn't successfully call PubSub Self-Test: " 
        . $response->as_string;
    }
    elsif ($response->content =~ /FAIL/)
    {
    die "PubSub Self-Test failed: " . $response->content;
    }
    return 1;
};

make_test "don't post", sub
{
    my $topic = get_topic;
    my $topic_state = new PubSub::TopicState($topic);
    sleep 1;
    expect_events($topic_state, ());
    return 1;
};

make_test "post", sub
{
    my $topic = get_topic;
    my $topic_state = new PubSub::TopicState($topic);
    my $event = unique_event;
    $event->post($topic);
    expect_events($topic_state, ($event));
    return 1;
};

make_test "post absolute url", sub
{
    my $topic = get_topic;
    my $topic_state = new PubSub::TopicState($topic);
    my $event = unique_event;
    $event->post($url . $topic);
    expect_events($topic_state, ($event));
    return 1;
};

make_test "unsupported do_method", sub
{
    my $response = $ua->request(POST $url, [do_method => "unsupported"]);
    if ($response->code =~ /^2/)
    {
        die "PubSub failed to return error code: " 
            . blockquote($response->as_string);
    }
    return 1;
};

make_test "simple route", sub
{
    my @topics = (get_topic, get_topic);
    route($topics[0], $topics[1]);
    my $topic_state = new PubSub::TopicState($topics[1]);
    my $event = unique_event;
    $event->post($topics[0]);
    expect_events($topic_state, ($event));
    return 1;
};

make_test "delete route", sub
{
    my @topics = (get_topic, get_topic);
    my $rid = uuid;
    route($topics[0], $topics[1], 'kn_id' => $rid);
    my $topic_state = new PubSub::TopicState($topics[1]);
    my $event = unique_event;
    $event->post($topics[0]);
    expect_events($topic_state, ($event));
    delete_route($topics[0], $rid);
    $event = unique_event;
    $event->post($topics[0]);
    expect_events($topic_state, ());
    return 1;
};

make_test "custom header", sub
{
    my $topic = get_topic;
    my $header = uuid;
    my $event = new PubSub::Event ("kn_id" => uuid, 
                   "kn_payload" => uuid, 
                   $header => uuid);
    my $topic_state = new PubSub::TopicState($topic);
    $event->post($topic);
    my ($new_event) = $topic_state->new_events;
    die "Didn't get event" if not $new_event;
    die "New event $new_event wasn't event: " . ref $new_event
        if not $new_event->isa('PubSub::Event');
    die "Event had wrong kn_id" if $new_event->kn_id ne $event->kn_id;
    die "Event was missing custom header $header" 
        if not $new_event->get_header($header);
    die "Event custom header $header didn't match"
        if $new_event->get_header($header) ne $event->get_header($header);
    return 1;
};

# Duplicate event squashing test.

make_test "duplicate event squashing", sub
{
    my $topic0 = get_topic;
    my $topic_state = new PubSub::TopicState($topic0);
    
    my $event = unique_event;
    $event->post($topic0);
    expect_events($topic_state, ($event));

    # FIXME: This is to avoid a race condition.
    # We use mtime, with 1sec resolution!
    sleep 1;
    
    my $event2 = new PubSub::Event ("kn_id" => $event->kn_id, "kn_payload" => uuid);
    $event2->post($topic0);
    expect_events($topic_state, ($event2));
    
    sleep 1;
    $event2->post($topic0);

    # Expect no events.
    expect_events($topic_state, ());
    return 1;
};

# In pubsub.cgi, events are stored next to subtopics; this is to ensure
# they don't get confused with each other.

make_test "subtopics aren't events", sub
{
    my $topic = get_topic;
    my $ts = new PubSub::TopicState($topic);
    my $subtopic = "$topic/subtopic";
    unique_event->post($subtopic);
    my $event = unique_event;
    $event->post($topic);
    expect_events($ts, ($event));
    return 1;
};

# When someone changed the duplicate suppression logic to take more
# than the kn_id and kn_payload into account, they introduced a bug
# that caused spurious warnings and didn't work correctly when the old
# event differed from the new only by having keys the new event
# didn't.  This test case verifies that that bug is still gone.  This
# was one of our longer-lived bugs --- it lived from 2000-09-13 to
# 2001-01-25.

make_test "missing keys in new event", sub
{
    my $topic = get_topic;
    my $topic_state = new PubSub::TopicState($topic);
    my $id = uuid();
    my $event = PubSub::Event->new('kn_id' => $id, 'trash' => 37);
    $event->post($topic);
    $event = PubSub::Event->new('kn_id' => $id);
    $event->post($topic);
    my @new_events = $topic_state->new_events();
    die "Wrong number of new events: " . @new_events if @new_events != 1;
    die "Didn't take out the trash" if defined $new_events[0]->get_header('trash');
    return 1;
};

# Journal test.

make_test "kn_journal", sub
{
    my $topic1 = get_journal;
    my $topic_state = new PubSub::TopicState($topic1);
    my $event = unique_event;
    $event->post($topic1);
    expect_events($topic_state, ($event));
    $event->post($topic1);
    expect_events($topic_state, ($event));
    # Close our tunnel.
    $topic_state->{'tunnel'}->close();
    return 1;
};

make_test "alt kn_journal", sub
{
    my $topic1 = get_journal;
    my $topic_state = new PubSub::TopicState($topic1);
    my $event = unique_event;
    $event->post($topic1);
    expect_events($topic_state, ($event));
    my $event2 = new PubSub::Event ("kn_id" => $event->kn_id, "kn_payload" => uuid);
    $event2->post($topic1);
    $event2->post($topic1);
    expect_events($topic_state, ($event2, $event2));
    # Close our tunnel.
    $topic_state->{'tunnel'}->close();
    return 1;
};

# Loop test.

make_test "simple loop", sub
{
    my @topics = (get_topic, get_topic);
    route($topics[0], $topics[1]);
    route($topics[1], $topics[0]);
    my $topic_state = new PubSub::TopicState($topics[1]);
    my $event = unique_event;
    $event->post($topics[0]);
    # If we get here, we're OK; infinite posting runs forever. :)
    expect_events($topic_state, ($event));
    return 1;
};

# The previous test doesn't verify that we aren't using headers that
# change on every hop in duplicate suppression.  This test verifies
# that, at least if every route causes those headers to change.

make_test "hard loop", sub
{
    my @topics = (get_topic, get_topic, get_topic);
    route($topics[0], $topics[1]);
    route($topics[1], $topics[0]);
    route($topics[1], $topics[2]);
    route($topics[2], $topics[1]);
    route($topics[2], $topics[0]);
    route($topics[0], $topics[2]);
    my $topic_state = new PubSub::TopicState($topics[0]);
    my $event = unique_event;
    $event->post($topics[0]);
    expect_events($topic_state, ($event));
    return 1;
};

# This one used to loop forever, causing pubsub.cgi to swell to huge
# proportions and then exit with an error.  Now it is OK.

make_test "journal loop", sub
{
    my @topics = (get_journal, get_journal);
    # Pretend we're watching the journals so they're POSTable.
    my @topic_states = (new PubSub::TopicState($topics[0]),
                        new PubSub::TopicState($topics[1]));
    # These next two calls to route should normally fail,
    # because the server will return an error.
    eval { route($topics[0], $topics[1]); };
    eval { route($topics[1], $topics[0]); };
    unique_event->post($topics[0]);
    # Close our tunnels.
    $topic_states[0]->{'tunnel'}->close();
    $topic_states[1]->{'tunnel'}->close();
    return 1;
};

make_test "journal loop with trailing slash", sub
{
    my @topics = (get_journal, get_journal);
    # Pretend we're watching the journals so they're POSTable.
    my @topic_states = (new PubSub::TopicState($topics[0]),
                        new PubSub::TopicState($topics[1]));
    # These next two calls to route should normally fail,
    # because the server will return an error.
    eval { route($topics[0].'/', $topics[1]); };
    eval { route($topics[1].'/', $topics[0]); };
    unique_event->post($topics[0]);
    # Close our tunnels.
    $topic_states[0]->{'tunnel'}->close();
    $topic_states[1]->{'tunnel'}->close();
    return 1;
};

make_test "multipath", sub
{
    my @topics = map { get_topic } (1..3);
    route($topics[0], $topics[1]);
    route($topics[1], $topics[2]);
    route($topics[0], $topics[2]); # Two paths for events to get from 0 to 2.
    
    my $topic_state = new PubSub::TopicState($topics[2]);
    my $event = unique_event;
    $event->post($topics[0]);
    expect_events($topic_state, ($event));
    return 1;
};

make_test "multipath journal", sub
{
  my @topics = (get_topic, get_topic, 
        get_journal);
  route($topics[0], $topics[1]);
  route($topics[1], $topics[2]);
  route($topics[0], $topics[2]);

  my $topic_state = new PubSub::TopicState($topics[2]);
  my $event = unique_event;
  $event->post($topics[0]);
  expect_events($topic_state, ($event, $event));
  # Close our tunnel.
  $topic_state->{'tunnel'}->close();
  return 1;
};

make_test "empty body", sub
{
    my $topic = get_topic;
    my $event = new PubSub::Event ( "kn_id" => uuid, "kn_payload" => "" );
    my $topic_state = new PubSub::TopicState($topic);
    $event->post($topic);
    expect_events($topic_state, ($event));
    return 1;
};

# Now this is obviated by routing all routes this way.

make_test "loopback HTTP POST", sub
{
    my @topics = (get_topic, get_topic);
    my $post_url = "$url$topics[1]"; # we trust $topics[1] begins with /
    route($topics[0], $post_url);
    my $topic_state = new PubSub::TopicState($topics[1]);
    my $event = unique_event;
    $event->post($topics[0]);
    expect_events($topic_state, ($event));
    return 1;
};

# There should be content.

make_test "notify reply content", sub
{
    my $result = $ua->request(POST $url, 
                  [ kn_payload => uuid,
                    kn_to => get_topic]);
    die "Bad result from notify with debug: " . $result->as_string
        if $result->code ne '200' or not $result->content;
    return 1;
};

# Likewise for route.

make_test "route reply content", sub
{
    my $result = $ua->request(POST $url,
                  [ do_method => 'route',
                    kn_from => get_topic,
                    kn_to => get_topic]);
    die "Bad result from route with content: " . $result->as_string
    if $result->code ne '200' or not $result->content;
    return 1;
};

# XXX: this fails if $topic is a journal, because the event gets
# posted to the server, but there isn't time for us to receive it
# before we expect_events on it.

make_test "big message", sub
{
    my $topic = get_topic;
    my $event = new PubSub::Event (kn_id => uuid, 
                   kn_payload => uuid . 'x' x 1048576);
    my $topic_state = new PubSub::TopicState($topic);
    $event->post($topic);
    expect_events($topic_state, ($event));
    return 1;
};

sub expect_header 
{
    my ($event, $header, $value) = @_;
    my $real_value = $event->get_header($header);
    die "No header $header" if not $real_value;
    die "Wrong header $header: $real_value ne $value" 
    if $real_value ne $value;
};

make_test "kn_routed_from route deletion", sub
{
    my @topics = (get_topic, get_journal);
    route($topics[0], $topics[1]);
    my $event = unique_event;
    my $topic_state = new PubSub::TopicState($topics[1]);
    $event->post($topics[0]);
    my ($new_event) = $topic_state->new_events;
    die "Received no event" if not $new_event;
    my $route_topic = $new_event->get_header('kn_routed_from');
    die "No kn_routed_from" if not $route_topic;
	$route_topic =~ s|://|://$usr:$pw@| if defined $usr;
    my $route_id = $new_event->get_header('kn_route_id');
    die "No kn_route_id" if not $route_id;
    my $result = $ua->request(POST $route_topic, [ 'do_method' => 'route', 
                                                   'kn_id' => $route_id,
                                                   'kn_expires' => '+5',
                                                   'kn_to' => '' ]);
    die "Attempted deletion failed: " . $result->as_string() 
		if $result->code() !~ /^[123]/;
    $event = unique_event;
    $event->post($topics[0]);
    expect_events($topic_state, ());
    # Close our tunnel.
    $topic_state->{'tunnel'}->close();
    return 1;
};

make_test "route content filter", sub
{
    my @topics = (get_topic, get_topic);
    route($topics[0], $topics[1],
      "kn_content_filter" => "(?i)foo");
    my $event = new PubSub::Event ('kn_payload' => 'bar',
                   'kn_id' => uuid );
    my $topic_state = new PubSub::TopicState($topics[1]);
    $event->post($topics[0]);
    expect_events($topic_state, ());
    $event = new PubSub::Event ('kn_payload' => 'aFooB',
                'kn_id' => uuid );
    $event->post($topics[0]);
    expect_events($topic_state, $event);
    return 1;
};

make_test "mail sensor", sub
{
    # This is my body.
    my $body = uuid . "\n";
    my $msg = join ('', 
            "Subject: ", uuid, "\n",
            "From: ", uuid, "\n",
            "\n",
            $body);
    
    my $topic = get_topic;
    my $topic_state = new PubSub::TopicState($topic);

    my $url = "$url$topic";
    open MAILPIPE, "| perl ../kn_sense/mailfrom.plx $url";
    print MAILPIPE $msg;
    close MAILPIPE or die "Couldn't pipe perl mailfrom $url: $!";
    sleep 5;
    my ($new_event) = $topic_state->new_events;
    expect_header($new_event, "kn_payload", $body);
    return 1;
};

make_test "malicious kn_id test", sub
{
    my $id = "../evil";
    my $parent_topic = get_topic;
    my $child_topic = "$parent_topic/child";
    my $event = new PubSub::Event('kn_id' => $id, 'kn_payload' => uuid);
    my $topic_state = new PubSub::TopicState($parent_topic);
    my $child_state = new PubSub::TopicState($child_topic);
    eval { $event->post($child_topic); };
    if ($@)
    {
      expect_events($topic_state, ());
      expect_events($child_state, ());
    }
    else
    {
      expect_events($topic_state, ());
      expect_events($child_state, ($event));
    }
    return 1;
};

# We think we saw a bug; route-id seems to be naming the first route
# it went over, not the last.

make_test "correct kn_route_location test", sub
{
    my @topics = (get_topic, get_topic, get_journal);
    route($topics[0], $topics[1]);
    route($topics[1], $topics[2]);
    my $topic_state = new PubSub::TopicState($topics[2]);
    my $event = unique_event;
    $event->post($topics[0]);
    my ($new_event) = $topic_state->new_events;
    my $route_location = $new_event->get_header("kn_route_location");
    die "route location $route_location doesn't contain $topics[1]"
    if $route_location !~ m/\Q$topics[1]/;
    # Close our tunnel.
    $topic_state->{'tunnel'}->close();
    return 1;
};

make_test "content transform", sub
{
    my @topics = (get_topic, get_topic);

    # FIXME: if the pubsub_test.cgi URL is HTTP or HTTP/S, we assume
    # xform_rot13.cgi is co-located with it in URL-space, otherwise we
    # assume it is co-located with the server in URL space. These
    # assumptions break horribly when (a) the server is referred to as
    # .../kn, and (b) pubsub_test.cgi is run from the command-line, or
    # from behind a firewall.
    my $rot13url = $url;
    my $pubsub_test_url_scheme = new URI(url)->scheme;
    if ($pubsub_test_url_scheme eq "http" or
        $pubsub_test_url_scheme eq "https")
    {
        $rot13url = url;
    }
    $rot13url =~ s:[^/]*$:xform_rot13.cgi:;

    route ($topics[0], $topics[1], "kn_content_transform" => $rot13url);
    my $event = new PubSub::Event(kn_id => uuid, kn_payload => "Kragen Sittler");
    my $topic_state = new PubSub::TopicState($topics[1]);
    $event->post($topics[0]);
    my ($new_event) = $topic_state->new_events;
    die "No event" if not $new_event;
    my $payload = $new_event->get_header("kn_payload");
    die "Event payload wrong: $payload" if $payload ne "Xentra Fvggyre";
    return 1;
};

make_test "path_info on route from", sub
{
    my @topics = (get_topic, get_topic);
    my $event = unique_event;
    my $topic_state = new PubSub::TopicState($topics[1]);
    my $result = LWP::Simple::get("$url$topics[0]?kn_to=$url$topics[1]" .
                                  ";do_method=route");
    $event->post($topics[0]);
    expect_events($topic_state, ($event));
    return 1;
};

make_test "initial route population with do_max_age", sub
{
    my @topics = (get_topic, get_topic);
    my $event = unique_event;
    my $topic_state = new PubSub::TopicState($topics[1]);
    $event->post($topics[0]);
    expect_events($topic_state, ());
    route ($topics[0], $topics[1], "do_max_age" => 600);
    expect_events($topic_state, ($event));
    return 1;
};

make_test "duplicate initial route population", sub
{
    my @topics = (get_topic, get_journal);
    my $event = unique_event;
    my $topic_state = new PubSub::TopicState($topics[1]);
	my $id = uuid;
    $event->post($topics[0]);
    expect_events($topic_state, ());
    route ($topics[0], $topics[1], ("do_max_age" => 600, "kn_id" => $id));
    expect_events($topic_state, ($event));
    # The second time around it better not re-populate w/same rID.
    route ($topics[0], $topics[1], ("do_max_age" => 600, "kn_id" => $id));
    expect_events($topic_state, ());
    # Close our tunnel.
    $topic_state->{'tunnel'}->close();
    return 1;
};

make_test "evil characters in route-to URL", sub
{
    my @topics = (get_topic, get_topic . "/../flurb");
    my $topic_state = new PubSub::TopicState($topics[1]);
    my $event = unique_event;
    route($topics[0], "$base_url$topics[1]"); # This route req should fail.
    $event->post($topics[0]);
    expect_events($topic_state, ()); # This verifies that it did.
    return 1;
};

# Certain optimizations we made created a bug in handling of our own
# dests: you could route to pubsub.cgifoo, and it would treat that as
# pubsub.cgi/foo.  This test verifies that this bug is not present.  The
# bug was important because it hid a bug that should have made the
# next test fail: in some cases these broken URLs were being created
# internal to pubsub.cgi .

make_test 'missing / between pubsub.cgi and topic', sub
{
    my $localurl = $url;
    $localurl =~ s|/+$||;
    my @topics = (get_topic, get_topic);
    my $bad_name = $topics[1];
    $bad_name =~ s|^/*||;
    my $topic_state = new PubSub::TopicState($topics[1]);
    route($topics[0], $localurl . $bad_name);
    unique_event->post($topics[0]);
    expect_events($topic_state, ());
    return 1;
};

make_test 'relative route dest without leading /', sub
{
    my @topics = (get_topic, get_topic);
    my $bad_name = $topics[1];
    $bad_name =~ s|^/*||;
    my $topic_state = new PubSub::TopicState($topics[1]);
    route($topics[0], $bad_name);
    my $event = unique_event;
    $event->post($topics[0]);
    expect_events($topic_state, ($event));
    return 1;
};

make_test 'do_max_n', sub
{
    my @topics = (get_topic, get_topic);
    my @events = (unique_event, unique_event);
    $events[0]->post($topics[0]);
    sleep(2);
    $events[1]->post($topics[0]);
    my $topic_state = new PubSub::TopicState($topics[1]);
    route($topics[0], $topics[1], do_max_n => 1);
    expect_events ($topic_state, $events[1]);
    return 1;
};

# The real test here is to ensure that the server doesn't
# produce error messages or crash.

make_test 'impossible do_max_n', sub
{
    my @topics = (get_topic, get_topic);
    unique_event->post($topics[0]) for (1, 2);
    my $topic_state = new PubSub::TopicState($topics[1]);
    route($topics[0], $topics[1], do_max_n => 3);
    my @forwarded_events = $topic_state->new_events;
    if (@forwarded_events != 2)
    {
        die "Expected both events with do_max_n of 3; instead got " .
            @forwarded_events;
    }
    return 1;
};

make_test 'expires', sub
{
    my @topics = (get_topic, get_topic);
    my $event = new PubSub::Event ("kn_id" => uuid, 
                   "kn_payload" => uuid, 
                   "kn_expires" => "+1");
    $event->post($topics[0]);
    sleep(2);
    my $topic_state = new PubSub::TopicState($topics[1]);
    route($topics[0], $topics[1], do_max_n => 1);
    sleep(2);
    expect_events ($topic_state, ());
    return 1;
};

make_test 'loop with kn_expires', sub
{
    my $topic = get_topic;
    route($topic, $topic);
    my $event = unique_event;
    $event->set_header('kn_expires', '+10');
    my $topic_state = new PubSub::TopicState($topic);
    $event->post($topic);
    expect_events($topic_state, $event);
    return 1;
};

# At one time, posting an event with a kn_expires in the past would
# break duplicate squashing, which could cause infinite loops.

make_test "past kn_expires loop", sub
{
    my @topics = (get_topic, get_topic);
    route($topics[0], $topics[1]);
    route($topics[1], $topics[0]);
    my $topicstate = new PubSub::TopicState($topics[0]);
    my $event = unique_event;
    $event->set_header(kn_expires => 1000);
    $event->post($topics[0]);
    # It is mostly assumed that if the event finished posting in a
    # finite amount of time, everything is OK.
    # This expect_events doesn't verify much.
    expect_events($topicstate, ());
    return 1;
};

sub payloads_are_equal
{
    my ($a, $b) = @_;
    return $a->kn_payload eq $b->kn_payload;
}

# Batching and new output layer possible tests:
# - batch of two events posts events and posts 3 status events
# - batch of event, followed by route, followed by event, forwards only
#   2nd event

make_test 'forwarding status event', sub
{
    my @topics = (get_topic, get_topic);
    my $event = unique_event;
    $event->set_header('kn_status_to' => $topics[1]);
    my @topic_state = map { new PubSub::TopicState($_) } @topics;
    my $result = $event->post($topics[0]);
    die "Got wrong result code " . $result->code() 
        unless $result->code() eq '204';
    expect_events($topic_state[0], ($event));
    my @status = $topic_state[1]->new_events();
    die "Got wrong number of events " . @status unless @status == 1;
    my $status = $status[0]->get_header('status');
    die "Status event had no status " unless $status;
    die "Status event's status was $status" unless $status =~ /^200/;
    return 1;
};

make_test 'forwarding status event to absolute URI', sub
{
    my @topics = (get_topic, get_topic);
    my $event = unique_event;
    $event->set_header('kn_status_to' => "$url$topics[1]");
    my $topic_state = new PubSub::TopicState($topics[1]);
    $event->post($topics[0]);
    my @status = $topic_state->new_events();
    die "Got wrong number of status events: " . @status unless @status == 1;
    # Assume that the event is correct, because it was in the last test.
    return 1;
};

make_test 'two-event batch', sub
{
    my $topic = get_topic;
    my $topic_state = new PubSub::TopicState($topic);
    my @events = (unique_event, unique_event);
    my @encevents = map { ('kn_batch' =>
        encode_form($_->as_form($topic), 'do_method' => 'notify'))
    } @events;
    my $request = POST $url, [ do_method => 'batch', @encevents ];
    my $result = $ua->request($request);
    if ($result->code !~ /^[123]/)
    {
        die "Bad HTTP result posting batch: " . $result->as_string;
    }
    expect_events($topic_state, @events);
    return 1;
};

make_test 'batch status', sub
{
    my $topic = get_topic;
    my $journal = get_journal;
    my $journal_state = new PubSub::TopicState($journal);
    my @events = (unique_event, unique_event);
    for (@events) {
        $_->set_header('kn_status_from', uuid);
    }
    my @encevents = map { ('kn_batch' =>
                           encode_form($_->as_form($topic), 
                                       'do_method' => 'notify'))
                        } @events;
    my $batch_status_from = uuid;
    my $result = $ua->request(POST $url, [ do_method => 'batch',
                                           'kn_status_from' => 
                                               $batch_status_from,
                                           @encevents, 
                                           'kn_status_to' => $journal ]);
    if ($result->code !~ /^[123]/)
    {
        die "Bad HTTP result posting batch: " . $result->as_string;
    }
    my @status = $journal_state->new_events();

    my @wanted_locations = ($batch_status_from, 
                            $events[0]->get_header('kn_status_from'), 
                            $events[1]->get_header('kn_status_from'));

    die "Wrong number of new events " . @status 
        if $#status != $#wanted_locations;

    for my $i (0..$#wanted_locations)
    {
        if ($status[$i]->get_header('kn_route_location') ne $wanted_locations[$i])
        {
            die "Status event $i has wrong kn_route_location: '" . 
                $status[$i]->get_header('kn_route_location') .
                "' instead of '$wanted_locations[$i]'";
        }
    }
    
    return 1;
};

make_test "subtopic routing", sub
{
    my @topics = (get_topic, get_topic);
    my @subtopics = (uuid, uuid, uuid);
    unique_event->post("$topics[0]/$subtopics[0]");
    unique_event->post("$topics[0]/$subtopics[1]");
    my $topic_state = new PubSub::TopicState($topics[1]);
    route("$topics[0]/kn_subtopics", $topics[1], do_max_age => 'infinity');
    really_check_events(\&payloads_are_equal, $topic_state,
                        (new PubSub::Event(kn_payload => $subtopics[0]),
                         new PubSub::Event(kn_payload => $subtopics[1])));
    unique_event->post("$topics[0]/$subtopics[2]");
    really_check_events(\&payloads_are_equal, $topic_state,
                        (new PubSub::Event(kn_payload => $subtopics[2])));
    return 1;
};

sub expect_unpostable
{
    my ($topic) = @_;
    my $topic_state = new PubSub::TopicState($topic);
    my $result = eval { unique_event->post($topic); };
    die "Successfully posted to $topic: " . $result->as_string unless $@;

    # Just because we got back failure doesn't mean it didn't POST...
    # so we check.
    expect_events($topic_state, ());
}

make_test "can't post to kn_subtopics", sub
{
    expect_unpostable(get_topic . "/kn_subtopics");
    return 1;
};

make_test "can't post to kn_routes", sub
{
    expect_unpostable(get_topic . "/kn_routes");
    return 1;
};

sub poison_journal
{
    my ($topic) = @_;
    # Open a temporary tunnel.
    my $oldtimeout = $ua->timeout(5); # This next request should last 1 sec.
    my $result  = $ua->request(POST "$url$topic", [ do_method => 'route',
                                                    kn_expires => '+1' ]);
    $ua->timeout($oldtimeout);
    if ($result->code() !~ /^[123]/)
    {
        die "Got bad result on tunnel: " . $result->as_string();
    }
}

make_test "post to expired kn_journal", sub
{
    my $topic = get_topic . "/kn_journal";
    poison_journal($topic);

    my $postresult = eval { unique_event->post($topic); };
    if (!$@)
    {
        die "Successfully posted to expired topic: " . 
            $postresult->as_string();
    }
                                           
    return 1;
};

# Return a hostname the server knows us by.  NAT breaks this.

sub get_my_hostname
{
    my $uri = new URI($url);
    my $hostport = $uri->host_port();
    my $sock = new IO::Socket::INET($hostport);
    if (not $sock)
    {
        die "Couldn't open $hostport: $!";
    }
    my $hostname = $sock->sockhost();
    close $sock;
    return $hostname;
}

make_test "kn_time_t on forwarded HTTP posts", sub
{
    my $topic = get_topic;
    my $hostname = get_my_hostname;
    my $sock = new IO::Socket::INET(Listen => 5, LocalAddr => $hostname);
    if (not $sock)
    {
        die "Couldn't open listening socket on $hostname: $!";
    }
    my $addr = "http://$hostname:" . $sock->sockport() . "/";
    route($topic, $addr);
    unique_event->send($topic);
    my $r = '';
    vec($r, fileno($sock), 1) = 1;
    select($r, undef, undef, 4);
    if (!vec($r, fileno($sock), 1))
    {
        $sock->close();
        die "Didn't get a connection";
    }
    my $conn = $sock->accept();

    # Crudely parse an HTTP request.
    local $_;
    my $reqline = <$conn>;
    my $len;
    while (<$conn>)
    {
        $len = $1 if /Content-Length:\s+(\d+)\r\n/;
        last if ($_ eq "\r\n");
    }
    die "Didn't get length" if not defined $len;
    my $postdata;
    read $conn, $postdata, $len;
    my $response = "Fine.\n";
    print $conn ("HTTP/1.0 200 OK\r\n",
                 "Content-Type: text/plain\r\n", 
                 "Content-Length: ", length($response), "\r\n",
                 "\r\n$response");
    my $params = new CGI($postdata);

    if (not $params->param('kn_payload')) 
    {
        die "Didn't even get kn_payload (" . (join ' ', $params->param()) . ")";
    }
    if (not $params->param('kn_time_t'))
    {
        die "Didn't get kn_time_t";
    }
    close $conn;
    close $sock;
    
    return 1;
};

# FIXME: need a test to verify that status events can't be sent to kn_subtopics or kn_routes
# FIXME: need a test to verify that routed events can't be sent to them either

make_test "route from kn_routes", sub
{
    my @topics = (get_topic, get_topic);
    my $route_topic = "$topics[0]/kn_routes";
    my $routes = new PubSub::TopicState($route_topic);
    route($topics[0], $topics[1]);
    my @events = $routes->new_events;

    die "Got wrong number of routes " . @events if @events != 1;
    my $payload = $events[0]->kn_payload;
    die "Route dest wrong: $payload" if $payload !~ /\Q$topics[1]/;

    return 1;
};

make_test "route poisoning", sub
{
    my @topics = (get_topic, get_journal);
    route($topics[0], $topics[1]);
    my $route_topic = "$topics[0]/kn_routes";
    my $routes = new PubSub::TopicState($route_topic);
    poison_journal($topics[1]);
    unique_event->post($topics[0]);
    my @events = $routes->new_events;
    die "Got wrong number of routes " . @events if @events != 1;
    my $expiry = $events[0]->get_header('kn_expires');
    die "No expiry on poisoned route" if not defined $expiry;
    die "Dest on poisoned route: " . $events[0]->kn_payload 
        if $events[0]->kn_payload ne '';
    return 1;
};

# /foo/ and /foo are the same topic; python_pubsub could have had
# a bug where it didn't think they were.

make_test "trailing slash", sub
{
    my $topic = get_topic;
    my $topicstate = new PubSub::TopicState("$topic/");
    my $event = unique_event;
    $event->post($topic);
    expect_events($topicstate, $event);
    return 1;
};

sub expect_event_sequence
{
    my ($expected, @got) = @_;
    my @expected = @$expected;
    my %expectids = map { ( $expected[$_]->kn_id => $_ ) } (0..$#expected);
    for my $i (0..$#got)
    {
        my $id = $got[$i]->kn_id;
        die "Got completely unexpected event $id"
            if not exists $expectids{$id};
        die "Got $id as ${i}th when it should have been $expectids{$id}th"
            if $expectids{$id} != $i;
    }
    die "Got wrong number of events: " . @got . " instead of " . @expected
        if @got != @expected;
}

sub dont_make_test {}

# This test is currently too slow to check in and use.

dont_make_test "event sequencing", sub
{
    my $topic = get_topic;
    my $topicstate = new PubSub::TopicState($topic);
    my $topicstate2 = new PubSub::TopicState($topic);
    my @firstevents = map { unique_event } (1..10);
    for my $i (0..4)
    {
        $firstevents[$i]->set_header('kn_expires', '+5');
    }
    for my $event (@firstevents)
    {
        $event->post($topic);
    }
    # Updating an event should move it to the end of the line.
    $firstevents[6]->set_header('kn_payload', uuid);
    $firstevents[6]->post($topic);
    # Get past pubsub.cgi's 30-second grace period within
    # which it doesn't forward.
    sleep 40;
    # This reads the events on the topic, which (for pubsub.cgi)
    # triggers the delete-from-filesystem magic.
    expect_events($topicstate2, @firstevents[5..9]);
    my @moreevents = map { unique_event } (1..10);
    for my $event (@moreevents)
    {
        $event->post($topic);
    }
    expect_event_sequence([$firstevents[5], @firstevents[7..$#firstevents], 
                           $firstevents[6], @moreevents],
                          $topicstate->new_events);
    return 1;
};


## Insert new tests above here.

# The replay-of-events tests take more than 30 seconds to complete,
# so all new tests should be added before them, instead of after.

make_test "replay of events", sub
{
    my @topics = (get_topic, get_journal);
    my $topic_state = new PubSub::TopicState($topics[1]);
    my $route_id = uuid;
    route($topics[0], $topics[1], kn_id => $route_id);
    my @events = (unique_event, unique_event, unique_event, unique_event);
    
    $events[0]->post($topics[0]);
    sleep 3;
    $events[1]->post($topics[0]);
    sleep 7;
    $events[2]->post($topics[0]);
    # Delete route.
    route($topics[0], "", kn_id => $route_id, kn_expires => '+30');

    $events[3]->post($topics[0]);

    expect_events($topic_state, @events[0, 1, 2]);

    my $replay_state = new PubSub::TopicState($topics[0]);

    my $result = $ua->request(POST "$url$topics[1]", [ do_method => 'replay',
                                                       user => $usr, 
                                                       password => $pw ]);
    if ($result->code !~ /^[123]/)
    {
        die "Got bad do_method=replay result " . $result->as_string;
    }
    sleep 2;
    really_check_events(\&payloads_are_equal, $replay_state, $events[0]);
    sleep 5;
    really_check_events(\&payloads_are_equal, $replay_state, $events[1]);
    sleep 10;
    really_check_events(\&payloads_are_equal, $replay_state, $events[2]);
    sleep 1;
    expect_events($replay_state, ());

    # Close our tunnel.
    $topic_state->{'tunnel'}->close();
    return 1;
};

# FIXME: Perhaps this should be merged into the previous test?

make_test "replay of events with warp factor", sub
{
    my @topics = (get_topic, get_journal);
    my $topic_state = new PubSub::TopicState($topics[1]);
    my $route_id = uuid;
    route($topics[0], $topics[1], kn_id => $route_id);
    my @events = (unique_event, unique_event);

    $events[0]->post($topics[0]);
    sleep 4;
    $events[1]->post($topics[0]);

    # Delete route.
    route($topics[0], "", kn_id => $route_id, kn_expires => '+30');

    my $replay_state = new PubSub::TopicState($topics[0]);

    my $result = $ua->request(POST "$url$topics[1]", [ do_method => 'replay', 
                                                       user => $usr, 
                                                       password => $pw, 
                                                       warp => 3]);
    if ($result->code !~ /^[123]/)
    {
        die "Got bad do_method=replay result " . $result->as_string;
    }

    sleep 3;
    really_check_events(\&payloads_are_equal, $replay_state, @events);

    # Close our tunnel.
    $topic_state->{'tunnel'}->close();
    return 1;
};

# FIXME: We need a topic/eventname collision test.

if (param('test')) 
{
    print(header('text/html; charset=utf-8'),
          start_html(-title => "PubSub test results from " . localtime,
                     -text => "black",
                     -bgcolor => "white"), 
          h1(test_header param('test')), "<ol>\n");
    
    foreach my $test (param('test'))
    {
        print "<li>";
        interactive_run_test $test;
        print "</li>";
    }
    print "</ol>";
    print "<p>Tests completed at " . localtime, ".</p>\n";
    print end_html;
}
else
{
    run_all_tests; # Needed for primitive Win32 systems without fork.
    # fork_all_tests;  # 70 seconds instead of 130
}

# End of pubsub_test.cgi
