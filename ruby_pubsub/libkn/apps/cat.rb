#!/usr/local/bin/ruby
# -*- ruby -*-

# $Id: cat.rb,v 1.2 2003/05/06 00:45:31 ifindkarma Exp $

# http://mod-pubsub.sf.net/
require '../libkn'

host  = ARGV[0] or 'localhost'
port  = ARGV[1] or '80'
topic = ARGV[2] or '/'
key   = ARGV[3] or nil

client = PubSub::Client.new(host, port.to_i)

begin
  client.subscribe(topic) {
    |e|
    if not key.nil?
      puts "#{key} =\t\"#{e[key]}\"" unless e[key].nil?
    else
      e.keys.collect {|k| puts "#{k} =\t\"#{e[k]}\""}
    end
  }
rescue PubSub::ServerUnavailableException => e
  puts "#{$0}: Unable to locate or connect to a PubSub Server on " + e
  exit 0
end

client.thread.join
exit 0
