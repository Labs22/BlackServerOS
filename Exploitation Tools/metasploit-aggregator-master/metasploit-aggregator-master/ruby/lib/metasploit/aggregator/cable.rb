module Metasploit
  module Aggregator
    class Cable
      HTTPS = 'https'
      HTTP = 'http'

      attr_reader :forwarder
      attr_reader :server
      attr_reader :thread

      def initialize(thread, server, forwarder)
        @thread = thread
        @forwarder = forwarder
        @server = server
      end
    end
  end
end
