#!/usr/local/bin/ruby
# -*- ruby -*-

# $Id: nasdaq.rb,v 1.1 2003/03/15 03:57:32 ifindkarma Exp $

# http://mod-pubsub.sf.net/
require '../libkn'

host  = 'localhost'
port  = '80'
topic = '/what/nasdaq'

client = PubSub::Client.new(host, port.to_i)
begin
  client.subscribe("#{topic}/kn_subtopics", {'do_max_age' => 'infinity' }) {
    |e|
    ['last', 'bid', 'ask', 'vol'].each do |comp|
      client.subscribe("#{topic}/#{e['kn_payload']}/#{comp}") {
        |e|
        plfmt = "%0.3f"
        plfmt = "%0.0f" if comp == 'vol'
        printf("NASDAQ:%0.4s\t#{comp}\t#{plfmt}\n",
               e['symbol'], e['kn_payload'].to_s.to_f) unless
          e['symbol'].nil? or e['kn_payload'].nil?
      }
      puts "monitoring #{topic}/#{e['kn_payload']}/#{comp}"
    end unless e['kn_payload'].nil? or e['kn_payload'].to_s.index('kn_') == 0

  }
rescue PubSub::ServerUnavailableException => e
  puts "#{$0}: Unable to locate or connect to a PubSub Server on " + e
  exit 0
end

begin
  client.thread.join
rescue PubSub::ServerUnavailableException => e
  puts "#{$0}: Unable to locate or connect to a PubSub Server on " + e
end
exit 0
