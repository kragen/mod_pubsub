#!/usr/local/bin/ruby
# -*- ruby -*-

# $Id: stocks.rb,v 1.1 2003/03/15 03:57:32 ifindkarma Exp $

require 'net/http'
# http://mod-pubsub.sf.net/
require '../libkn'

host    = 'localhost'
port    = '80'
topic   = '/what/nasdaq'
threads = []
events  = 0
elapsed = 0
old     = {}

client = PubSub::Client.new(host, port.to_i)
begin
  client.subscribe("#{topic}/kn_subtopics", {'do_max_age' => 'infinity' }) {
    |e|
    sym = e['kn_payload'].to_s
    if not sym.nil? and not sym.index('kn_') == 0
      t = Thread.new {
        while true
          Net::HTTP.start( 'finance.yahoo.com', 80 ) {|http|
            q = "/d/quotes.csv?s=#{sym}&f=sl1d1t1c1ohgvba&e=.csv"
            begin
              response, body = http.get(q)
            rescue
              retry
            end
            if response.message == "OK"
              symbol,last,date,time,change,open,high,low,vol,bid,ask =
                body.split(',')
              symbol = symbol.gsub('"', '').swapcase
              tradetime = "#{date.strip} #{time.strip}".gsub('"', '')
#              puts symbol
              ['last','bid','ask','vol','change','high','low','open'].each do
                |x|
                topic = "/what/nasdaq/#{symbol}/#{x}"
#                puts topic
                val = self.instance_eval(x).chomp
                val = (val =~ 'N/A' ? nil : val)
                if not val.nil?
                  if old[topic].nil? or old[topic] != val
                    old[topic] = val
                    e = {
                      'kn_id'       => topic.unpack("32V").to_s,
                      'symbol'      => symbol,
                      'type'        => x,
                      'trade_time'  => tradetime,
                      'kn_expires'  => '+3600',
                      'kn_payload'  => val,
                      'displayname' => "#{symbol}:\t#{x} = #{val}"
                    }
#                    e.keys.collect {|k| puts "\t#{k}=#{e[k]}"}
#                    started = Time.now.to_f
                    client.publish(topic, PubSub::Event.new(e))
#                    elapsed += Time.now.to_f - started
                    printf "NASDAQ:%4s %7s\t#{val}\n", symbol, x
                    events += 1
                  end
                end
              end
            else
              break
            end
          }
#          evpersec = events / elapsed
#          printf "%0.3f ev/sec, total delivered #{events}\n", evpersec if
#            (events % 100) == 0
        end
      }
      threads.push(t)
    end
  }
end

threads.each do |t| t.join; end
client.thread.join
