require 'socket'
require 'openssl'
require 'metasploit/aggregator/forwarder'
require 'metasploit/aggregator/http/request'
require 'metasploit/aggregator/http/ssl_responder'
require 'metasploit/aggregator/logger'
require 'metasploit/aggregator/router'

module Metasploit
  module Aggregator

    class HttpsForwarder < Forwarder

      def initialize
        super
      end

      def forward(connection)
        #forward input requests
        request_obj = Metasploit::Aggregator::Http::SslResponder.get_data(connection, false)
        uri = Metasploit::Aggregator::Http::Request.parse_uri(request_obj)
        @forwarder_mutex.synchronize do
          unless uri.nil?
            unless @response_queues[uri]
              uri_responder = Metasploit::Aggregator::Http::SslResponder.new(uri)
              uri_responder.log_messages = @log_messages
              @response_queues[uri] = uri_responder
            end
            @response_queues[uri].queue << request_obj
            @response_queues[uri].time = Time.now
          else
            connection.sync_close = true
            connection.close
          end
        end
      end
    end
  end
end
