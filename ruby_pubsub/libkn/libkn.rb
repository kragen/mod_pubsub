# -*- ruby -*-

=begin

== NAME

libkn.rb - Ruby PubSub Client Library for use with a PubSub Server
Originally written by Gregory Burd <gburd@ossus.com>

$Id: libkn.rb,v 1.2 2003/05/06 00:45:31 ifindkarma Exp $

Copyright (c) 2001-2003 KnowNow, Inc.  All rights reserved.

@KNOWNOW_LICENSE_START@

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.

3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
be used to endorse or promote any product without prior written
permission from KnowNow, Inc.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@KNOWNOW_LICENSE_END@


== EXAMPLE

=== ESTABLISH CONNECTION TO PUBSUB SERVER

  require 'libkn'
  
=== ESTABLISH CONNECTION VIA PROXY TO PUBSUB SERVER

  server = PubSub::Client.new(hostname, port)

=== PUBLISH EVENT

  server.publish(topic, PubSub::Event.new( { "displayname" => nick,
	                                 "kn_payload"  => msg })) { |e|
    puts "status event: #{e}"}

=== SUBSCRIBE TOPIC TO BLOCK

==== SUBSCRIBE BASICS

   Simply,
      server.subscribe("/what/chat") { |e| puts e }
   or your could do,
      server.subscribe("/what/chat", proc{ |e| puts e })
   and get the same results.
   To add subscription time qualifiers simply,
      server.subscribe("/what/chat", {"do_max_n" => "100"}) {|e| puts e}
   or you could do,
      server.subscribe("/what/chat", proc{|e| puts e}, {"do_max_n" => "100"})
   and again get the same results.

==== SUBSCRIBE BUT IGNORE THE STATUS EVENT

  server.subscribe("/what/chat", { "do_max_age" => since.to_s }) {
    |e| puts "%s: %s" % e['displayname'], e['kn_payload'] }

==== SUBSCRIBE AND GET THE STATUS EVENT

  server.subscribe("/what/chat", proc{|e|
    |e| puts "%s: %s" % e['displayname'], e['kn_payload'] },
    { "do_max_age" => since.to_s }) { |e| puts "status event #{e}" }

=== ROUTE FROM TOPIC TO TOPIC

  server.route("/what/foo", "/what/bar", {'kn_expires' => '+100'}) { |e|
    puts "status event: #{e}"}

=end

RUBY_VERSION >= "1.6.4" or
  raise "libkn.rb requires ruby lib version 1.6.4 or higher, you are using (#{RUBY_VERSION})"

require 'net/http'
require 'socket'
require 'thread'

module PubSub

  class ServerUnavailableException < Exception
  end

  class Client < Object
    VERSION      = '0.0.1'
    RELEASE_DATE = '2002-04-12'
    VERSION_CODE = 001
    RELEASE_CODE = 20020412
    REVISION     = '$Id: libkn.rb,v 1.2 2003/05/06 00:45:31 ifindkarma Exp $'

    def initialize(address = nil, port = nil, basetopic = nil,
		   username = nil, password = nil)
      @address       = address   || 'localhost'
      @port          = port      || 80
      @basetopic     = basetopic || '/kn'
      @username      = username  || 'anonymous'
      @password      = password
      @subscriptions = {}
      @proxy_address = nil
      @proxy_port    = 8000
      @basetopic     = '/kn'
      @status_blocks = {}
      @tunnel        = Tunnel.new(@address, @port, @username, @password,
				  @basetopic)
    end

    def to_s
      super + " kn: " + url
    end

    def thread
      @tunnel.thread if @tunnel
    end

    def url
      return @url unless @url.nil?
      u = "http://"
      u += "#{@username}:#{@password}@@" if @username and @password
      u += "#{@address}"
      u += ":#{@port}"
      u += "#{@basetopic}"
      @url = u
      return @url
    end

    def proxy(address, port)
      @proxy_address = address
      @proxy_port = port || 80
    end

    def publish(topic, event, &block)
      topic = '/' unless topic
      if event.kind_of? PubSub::Event
	u = "#{@basetopic}?do_method=notify"
	event.keys.collect { |k| u +=";#{escape(k)}=#{escape(event[k])}" }
        u += ";kn_to=#{escape(topic)}" if event['kn_to'].nil? #FIXME url+topic?
        status_journaled = false
        if event['kn_status_to'].nil? and @tunnel.open?
          u += ";kn_status_to=#{escape(@tunnel.topic)}"
          status_journaled = true
        end
        if event['kn_status_from'].nil?
          status_uri = "javascript://#{Uuid.gen()}"
          u += ";kn_status_from=#{escape(status_uri)}"
          dputs "#{status_uri} -> #{block}"
          @status_blocks[status_uri] = block if block_given? 
        else
          dputs "#{event['kn_status_from']} -> #{block}"
          @status_blocks[event['kn_status_from']] = block if block_given? 
        end
	h = Net::HTTP.new(@address, @port)
	h.Proxy(@proxy_address, @proxy_port) if @proxy_address
	resp, data = h.get(u, nil) #FIXME rescue on failure
        if status_journaled
          if resp.code == '204' # "No Content" which is good.
            return true
          else
            return false
          end
        else
          if resp.message == "OK"
            if block_given?
              yield Event.new.parse_javascript(data)
            end
            return true
          else
            return false
          end
        end
      end
    end

    def subscribe(topic, callback = nil, qualifiers = {}, &block)
      if not callback.nil? and callback.kind_of? Hash
        qualifiers = callback 
        callback = nil
      end
      if not callback.nil? and block_given?
        evblock = callback
        seblock = block
      elsif not callback.nil? and not block_given?
        evblock = callback
        seblock = nil
      elsif callback.nil? and block_given?
        evblock = block
        seblock = nil
      else
        evblock = nil
        seblock = nil
      end
      qualifiers['kn_expires'] = 'infinity' unless
        qualifiers.has_key? 'kn_expires'
      qualifiers['kn_id'] = Uuid.gen() unless qualifiers['kn_id']
      qualifiers['kn_uri'] = "#{url}#{topic}/kn_routes/#{escape(qualifiers['kn_id'])}"
      rid = qualifiers['kn_uri']
      @subscriptions[rid] = Array.new() if @subscriptions[rid].nil?
      @subscriptions[rid].push(evblock) unless evblock.nil?
      dputs "#{rid} for #{topic} -> #{@tunnel.topic} for #{evblock}"
      if not @tunnel.open?
        @tunnel.open(qualifiers) { |e|
          routed_id = e['kn_route_location'] unless e['kn_route_location'].nil?
          if not routed_id.nil?
            acb = @subscriptions[routed_id]
            if not acb.nil? and acb.size > 0
              acb.each do |p|
                dputs "#{unescape(routed_id)} #{p} called"
                p.call e
              end
            else
              cb = @status_blocks[routed_id]
              if not cb.nil?
                dputs e.to_s + " -> " + cb.to_s
                dputs "#{e['kn_status_to']} #{cb} called"
                cb.call e
               @status_blocks.delete(routed_id)
              end
            end
          end
        }
      end
      dputs "routing #{topic} to #{@tunnel.topic}"
      if not seblock.nil?
        routeid = route(topic, @tunnel.topic, qualifiers) {|e| seblock.call e}
      else
        routeid = route(topic, @tunnel.topic, qualifiers)
      end
      return "#{rid}/#{evblock}"
    end

    def unsubscribe(id)
      rid, rfn = id.split('/')
      cba = Array.new
      @subscriptions[rid].each do |fn|
        cba.push(fn) unless "#{fn}" == "#{rfn}"
      end unless @subscriptions[rid].nil?
      @subscriptions[rid] = cba
      @tunnel.stop unless subscriptions? > 0
    end

    def subscriptions?
      i = 0
      @subscriptions.keys.each do |rid|
        @subscriptions[rid].each do |fn|
          i += 1
        end unless @subscriptions[rid].nil?
      end unless @subscriptions.nil?
      return i
    end

    def route(from, to, qualifiers = {}, &block)
      u = "#{@basetopic}?do_method=route"
      qualifiers.keys.collect {|k|
        u +=";#{escape(k.to_s)}=#{escape(qualifiers[k].to_s)}" }
      u += ";kn_to=#{escape(to)}" unless qualifiers['kn_to']
      u += ";kn_from=#{escape(from)}" unless qualifiers['kn_from']
      knid = (qualifiers['kn_id'].nil? ? Uuid.gen() : qualifiers['kn_id'])
      u += ";kn_id=#{knid}"
      u += ";kn_uri=" + "#{url}#{from}/kn_routes/#{escape(knid)}" if
        qualifiers['kn_uri'].nil?
      status_journaled = false
      if qualifiers['kn_status_to'].nil?
        if  @tunnel.open?
          u += ";kn_status_to=#{escape(@tunnel.topic)}"
          status_journaled = true
        end
      else
        if not qualifiers['kn_status_to'].to_s.index('kn_journal').nil?
          status_journaled = true
        end
      end
      if qualifiers['kn_status_from'].nil?
        status_uri = "javascript://#{Uuid.gen()}"
        u += ";kn_status_from=#{escape(status_uri)}"
        dputs "#{status_uri} -> #{block}"
        @status_blocks[status_uri] = block if block_given? 
      else
        dputs "#{qualifiers['kn_status_from']} -> #{block}"
        @status_blocks[qualifiers['kn_status_from']] = block if block_given? 
      end
      h = Net::HTTP.new(@address, @port)
      h.Proxy(@proxy_address, @proxy_port) if @proxy_address
      begin
        resp, data = h.get(u, nil) # FIXME rescue on failure
        if status_journaled
          if resp.code == '204' # "No Content" which is what we expected.
            return knid
          else
            return nil
          end
        else
          if resp.message == "OK"
            if block_given?
              yield Event.new.parse_javascript(data)
            end
            return knid
          else
            return nil
          end
        end
      rescue Net::ProtoRetriableError => error
        #FIXME retry? or at least try harder?
        raise ServerUnavailableException, "#{@address}:#{@port}", caller
      rescue
        raise ServerUnavailableException, "#{@address}:#{@port}", caller
      end
    end

    def debug(val)
      @debug = val
    end

    protected

    def dputs(*args)
      if @debug
        args.each { |x| $stdout.write(x) }
        $stdout.write("\n")
      end
    end

    # escape url encode
    def escape(str)
      return nil if str.nil?
      str.gsub(/[^a-zA-Z0-9_\-.]/n){ sprintf("%%%02X", $&.unpack("C")[0]) }
    end

    # unescape url encoded
    def unescape(str)
      return nil if str.nil?
      str.gsub(/\+/, ' ').gsub(/%([0-9a-fA-F]{2})/){ [$1.hex].pack("c") }
    end

  end

  class Uuid < Object
    @@seqno = 0
    def Uuid.gen
      @@seqno += 1
      return "#{Time.now.to_i}_#{Process.pid}_#{rand(10)}#{@@seqno}"
    end
  end

  class Tunnel < Object

    @dispatcher
    @shutdown    = false

    def initialize(address, port, username, password, basetopic = '/kn')
      @address   = address
      @port      = port
      @basetopic = basetopic
      @username  = username || 'anonymous'
      @password  = password
      @topic     = "/who/#{@username}/s/#{Uuid.gen()}/kn_journal"
      @retry     = 10
    end
    
    def to_s
      super + "#{@topic} on #{@socket}"
    end

    def topic
      return @topic
    end

    def thread
      return @dispatcher if @dispatcher
    end

    def open(qualifiers = {})
      @shutdown = false
      establish(qualifiers) if @socket.nil?
      @dispatcher = Thread.new { consume() { |x|
          if block_given?
            yield x
          end
          }
      } unless @dispatcher
    end

    def stop()
      @shutdown = true
    end

    def open?
      open = false
      open = (not @socket.closed?) if not @socket.nil?
      return open
    end
    
    def restart(checkpoint)
      #FIXME
    end

    def consume()
      while not @shutdown do
	establish({'do_since_checkpoint' => @checkpoint}) if
	  @socket.nil? or @socket.eof? or @socket.closed?
	len = @socket.readline.to_i
	data = @socket.read(len) if len > 0
        if  not (data.nil? or data == "")
          e = Event.new.parse_simple(data)
          len = 0; data = nil
          @checkpoint = e['kn_route_checkpoint']
          if block_given?
            yield e
          end
        end
      end
      @socket.close
      puts "going to shutdown the tunnel #{@socket} now..." if @shutdown
      $stdout.flush
    end

    def establish(qualifiers = {})
      msg = "GET #{@basetopic}?do_method=route"
      qualifiers.delete('kn_response_format') if
	qualifiers and qualifiers['kn_response_format'] 
      qualifiers.keys.collect { |key|
	msg +=";#{escape(key)}=#{escape(qualifiers[key])}" } if qualifiers
      msg += ";kn_from=#{topic}" unless qualifiers['kn_from'] 
      msg += ";kn_response_format=simple"
      msg += " HTTP/1.0\r\n\r\n"
      started = Time.now.to_i
      begin
        @socket = TCPSocket.new(@address, @port) #FIXME proxy?
        @socket.print(msg)
        headder = true
        buf = ""
        while headder do
          ch = @socket.read(1)
          buf += "#{ch}"
          headder = false if buf[buf.size - 4, buf.size] == "\r\n\r\n"
        end
        return buf
      rescue
        #puts "#{Time.now.to_i - started} #{@retry}" #FIXME
        if ((Time.now.to_i - started) <= @retry)
          #puts "com'on!" #FIXME
          retry
        else
          raise ServerUnavailableException, "#{@address}:#{@port}", caller
        end
      end
    end

    # escape url encode
    def escape(str)
      return nil if str.nil?
      str.gsub(/[^a-zA-Z0-9_\-.]/n){ sprintf("%%%02X", $&.unpack("C")[0]) }
    end

  end

  class Event < Object

    def initialize(t = nil)
      @table = t if (t != nil and t.kind_of? Hash)
      @table = Hash.new unless @table
    end

    def to_s
      res = super
      kvp = self.keys.collect {|k| "#{k}=\"#{self[k]}\" "}
      kvp.each { |x| res += x }
      res
    end

    def [](key)
      @table[key] if @table.key? key
    end

    def []=(key, value)
      @table[key] = value
    end

    def keys
      @table.keys
    end

    def dump_s(pre = '')
      self.keys.collect {|k| puts "#{pre}#{k}=\"#{self[k]}\""}
    end

    def parse_javascript(str)
      str.gsub(/\n|\r/,' ').
	scan(/name\s*:\s*"([^"]*)"\s*,\s*value\s*:\s*"([^"]*)"/) {
	|k, v|  self[k] = v.to_s }
      self
    end

    def parse_simple(str)
      str.scan(/([^:]+):\s*(.*)\n/) {|k, v| self[k] = unescape(v).to_s }
      payload = str.scan(/\n\n(.*)/)
      self['kn_payload'] = payload
      self
    end

    def unescape(str)
      return nil if str.nil?
      str.gsub(/=([0-9a-fA-F=]{2})/){ [$1.hex].pack("c") }
    end

  end

end

