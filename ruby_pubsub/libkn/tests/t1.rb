#!/usr/local/bin/ruby
# -*- ruby -*-

# $Id: t1.rb,v 1.1 2003/03/15 03:57:32 ifindkarma Exp $

# http://mod-pubsub.sf.net/
require '../libkn'

host, port, basetopic = 'localhost', 80, '/kn'
#host, port, basetopic = '127.0.0.1', 80, '/kn'
#host, port, basetopic = 'localhost', 80, '/~gburd/kn/cgi-bin/kn-cgi'

rtopic = "/t.rb/#{$$}"
n = 1
require 'profile' if false

tests = {
  "#{n}-pub"   => true,
  "#{n}-sub"   => false,
  "#{n}-route" => true
}

client = PubSub::Client.new(host, port)
client.debug(true)

if tests["#{n}-pub"]
  n.times do
    client.publish(rtopic + "#{n}-pub",
               PubSub::Event.new({"displayname" => "ruby?",
                                  "kn_payload"  => "(.st + .py) % .self" })) {|e|
      puts "publish status: #{e.to_s}" }
  end
end

if tests["#{n}-sub"]
  n.times do
    begin
      sub = client.subscribe(rtopic + "/#{n}-sub", {'do_max_age' => '1'}) { |e| }
      client.unsubscribe(sub)
    rescue PubSub::ServerUnavailableException => e
      puts "#{$0}: Unable to locate or connect to a PubSub Server on " + e
      exit
    end
  end
end

if tests["#{n}-route"]
  n.times do
    begin
      rid = client.route(rtopic + "/#{n}-route/from/#{n}",
                     rtopic + "/#{n}-route/to/#{n}",
                     {'kn_expires' => '+10'}) { |e| puts e.to_s }
    end
  end
end

client.thread.join unless client.thread.nil?
exit


#client.debug(true)
puts client.url

#### test pub
begin
sub1 = client.subscribe("/rbclient-tests/chat", { "do_max_age" => "#{30*60}" }) { |e| 
    if not e['displayname'].nil?
      puts "#{e['displayname']}: #{e['kn_payload']}"
    end }
rescue PubSub::ServerUnavailableException => e
  puts "#{$0}: Unable to locate or connect to a PubSub Server on " + e
  exit
end

begin
sub2 = client.subscribe("/what/chat", { "do_max_age" => "#{30*60}" }) { |e| 
    if not e['displayname'].nil?
      puts "#{e['displayname']}: #{e['kn_payload']}"
    end }
rescue PubSub::ServerUnavailableException => e
  puts "#{$0}: Unable to locate or connect to a PubSub Server on " + e
  exit
end

client.unsubscribe(sub1)
client.unsubscribe(sub2)

puts "pub failed" unless
  client.publish("/what/chat", PubSub::Event.new(
		 { "displayname" => "ruby",
                   "kn_payload"  => "rules" })) #{ |e| puts e } 
puts "-- published"
puts "pub failed" unless
  client.publish("/spud", PubSub::Event.new(
	      { "displayname" => "ruby",
		"kn_payload"  => "kicks a**" })) #{ |e| puts e } 
puts "-- published"
exit 0

t = Thread.new { 
  while true
    if false
    end
  end
}
t.join

begin
  client.subscribe("/what/chat") { |e|
    puts e unless e.nil?
  }
rescue PubSub::ServerUnavailableException => e
  puts "PubSub::ServerUnavailableException on " + e
  exit
end

begin
  client.subscribe("/what/chat", { "do_max_n" => "1" }) {
    |e| puts e}
#    puts 'got /spud event ' + 
#      e['kn_payload'] }# if not e['kn_payload'].nil? }
rescue PubSub::ServerUnavailableException => e
  puts "PubSub::ServerUnavailableException on " + e
  exit
end
puts "-- subscribed"
#begin
#  client.subscribe("/what/chat", { "do_max_n" => "1" }) {
#    |e| puts e }
#    puts 'got /what/chat event ' + 
#      e['kn_payload'] } #if not e['kn_payload'].nil? }
#rescue PubSub::ServerUnavailableException => e
#  puts "PubSub::ServerUnavailableException on " + e
#  exit
#end
#puts "-- subscribed"
