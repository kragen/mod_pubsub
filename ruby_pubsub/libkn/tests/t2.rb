#!/usr/local/bin/ruby
# -*- ruby -*-

# $Id: t2.rb,v 1.1 2003/03/15 03:57:32 ifindkarma Exp $

# http://mod-pubsub.sf.net/
require '../libkn'

host, port, basetopic = 'localhost', 80, '/kn'
client = PubSub::Client.new(host, port)
#client.debug(true)

# publish before a tunnel exists, expect status right away
client.publish("/biff",
               PubSub::Event.new({"displayname" => "pub",
                                  "kn_payload"  => "1st publication" })) {|e|
  puts "1st publish status:"; e.dump_s("\t") }

# route before a tunnel exists, expect status right away
client.route("/biff/from", "/biff/to",
            {"via_some_filter" => "route",
            "with_some_param"  => "1st route" }) {|e|
  puts "1st route status:"; e.dump_s("\t") }

# now, subscribe and get a status on that subscription
i1 = 0
begin
  sub1 = client.subscribe("/biff") { |e|
    puts "[#i1]event delivered to 1st sub:"
    e.dump_s("\t")
    i1 += 1}
rescue PubSub::ServerUnavailableException => e
  puts "#{$0}: Unable to locate or connect to a PubSub Server on " + e
  exit
end

i2 = 0
begin
  sub2 = client.subscribe("/biff", proc{ |e|
                        puts "[#{i2}]event delivered to 2nd sub:"
                        e.dump_s("\t")
                        i2 += 1},
                     {'do_max_n' => '1'})
rescue PubSub::ServerUnavailableException => e
  puts "#{$0}: Unable to locate or connect to a PubSub Server on " + e
  exit
end

i3 = 0
begin
  sub3 = client.subscribe("/biff", proc{ |e|
                        puts "[#{i3}]event delivered to 3rd sub:"
                        e.dump_s("\t")
                        i3 += 1 },
                     {'do_max_n' => '1'}) { |e|
    puts "3rd subscription status:"; e.dump_s("\t")}
rescue PubSub::ServerUnavailableException => e
  puts "#{$0}: Unable to locate or connect to a PubSub Server on " + e
  exit
end
sleep 1
$stdout.flush
puts "subscriptions? = #{client.subscriptions?}"
client.unsubscribe(sub1)
puts "subscriptions? = #{client.subscriptions?}"
client.unsubscribe(sub2)
puts "subscriptions? = #{client.subscriptions?}"

# publish, expect status later
client.publish("/biff",
           {"displayname" => "pub",
            "kn_payload"  => "2nd publiction" }) {|e|
  puts "2nd publish status:"; e.dump_s("\t") }

client.unsubscribe(sub3)
puts "subscriptions? = #{client.subscriptions?}"

# publish, expect status right away not via tunnel
# which should be closed now
client.publish("/biff",
           {"displayname" => "pub",
            "kn_payload"  => "3rd publiction" }) {|e|
  puts "3rd publish status:"; e.dump_s("\t") }

$stdout.flush
client.thread.join # this should happen soon, after the tunnel is closed
puts "the client has closed the tunnel"
$stdout.flush
exit
