require "metasploit/aggregator/http/request"
require "metasploit/aggregator/http/responder"

module Metasploit
  module Aggregator
    module Http
      class SslResponder < Responder
        def initialize(uri)
          super
        end

        def get_connection(host, port)
          tcp_client = TCPSocket.new host, port
          ssl_context = OpenSSL::SSL::SSLContext.new
          ssl_context.ssl_version = :TLSv1
          ssl_client = OpenSSL::SSL::SSLSocket.new tcp_client, ssl_context
          ssl_client.connect
          ssl_client
        end

        def close_connection(connection)
          connection.sync_close = true
          connection.close
        end
      end
    end
  end
end