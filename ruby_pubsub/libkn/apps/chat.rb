#!/usr/local/bin/ruby
# -*- ruby -*-

# $Id: chat.rb,v 1.1 2003/03/15 03:57:32 ifindkarma Exp $

# http://mod-pubsub.sf.net/
require '../libkn'

opts = {'hostname' => 'localhost',
        'port'     => '80',
        'topic'    => '/what/chat',
        'since'    => "#{30*60}",
        'nick'     => 'Anonymous User',
        'listen'   => false}
     
client = PubSub::Client.new(opts['hostname'], opts['port'].to_i)

begin
  client.subscribe(opts['topic'], { "do_max_age" => opts['since'].to_s }) {
    |e| 
    if not e['displayname'].nil?
      puts "#{e['displayname']}: #{e['kn_payload']}"
    end }
rescue PubSub::ServerUnavailableException => e
  puts "#{$0}: Unable to locate or connect to a PubSub Server on " + e
  exit
end

puts "Connecting to #{opts['topic']} on #{client.url} as #{opts['nick']}"
t = Thread.new {
  if not opts['listen']
    while true
      # This gets is locking something in the I/O somewhere...
      msg = gets
      client.publish(opts['topic'],
                 PubSub::Event.new( {"displayname" => opts['nick'],
                                     "kn_payload"  => msg.chomp! }))
    end
  else
    client.thread.join
  end
}
t.join
