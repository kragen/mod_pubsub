#!/usr/local/bin/ruby
# -*- ruby -*-

# $Id: t3.rb,v 1.2 2003/05/06 00:45:31 ifindkarma Exp $

# http://mod-pubsub.sf.net/
require '../libkn'

host, port, basetopic = 'localhost', 80, '/kn'
client = PubSub::Client.new(host, port)
#client.debug(true)

# publish before a tunnel exists, expect status right away
client.publish("/biff",
               PubSub::Event.new({"displayname" => "pub",
                                  "kn_payload"  => "test from foo.rb" })) {|e|
  puts "1st publish status:"; e.dump_s("\t") }

# route before a tunnel exists, expect status right away
client.route("/biff/from", "/biff/to",
            {"displayname" => "pub",
             "kn_payload"  => "test from foo.rb" }) {|e|
  puts "1st route status:"; e.dump_s("\t") }

# now, subscribe and get a status on that subscription
begin
  sub1 = client.subscribe("/biff") { |e|
    puts "event delivered to 1st sub:"; e.dump_s("\t") }
rescue PubSub::ServerUnavailableException => e
  puts "#{$0}: Unable to locate or connect to a PubSub Server on " + e
  exit
end

begin
  sub2 = client.subscribe("/biff", proc{ |e|
                          puts "event delivered to 2nd sub:"; e.dump_s("\t")},
                         {'do_max_n' => '1'})
rescue PubSub::ServerUnavailableException => e
  puts "#{$0}: Unable to locate or connect to a PubSub Server on " + e
  exit
end

begin
  sub3 = client.subscribe("/biff", proc{ |e|
                          puts "event delivered to 3rd sub:"; e.dump_s("\t")},
                         {'do_max_n' => '1'}) { |e|
    puts "3rd subscription status:"; e.dump_s("\t")}
rescue PubSub::ServerUnavailableException => e
  puts "#{$0}: Unable to locate or connect to a PubSub Server on " + e
  exit
end
puts "ahhhhhhhhhhhhhhh"
puts "subscriptions? = #{client.subscriptions?}"
client.unsubscribe(sub1)
puts "subscriptions? = #{client.subscriptions?}"
client.unsubscribe(sub2)
puts "subscriptions? = #{client.subscriptions?}"

# publish, expect status later
client.publish("/biff",
              {"displayname" => "pub",
               "kn_payload"  => "test from foo.rb" }) {|e|
  puts "2nd publish status:"; e.dump_s("\t") }

client.thread.join
exit
