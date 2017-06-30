module Metasploit
  module Aggregator
    class Forwarder
      CONNECTION_TIMEOUT = 60 # one minute

      attr_accessor :log_messages
      attr_reader :response_queues

      def initialize
        @log_messages = false
        @response_queues = {}
        @forwarder_mutex = Mutex.new
        @router = Router.instance
      end

      # stub for indexing
      def forward(connection)

      end

      def connections
        # TODO: for now before reporting connections flush stale ones
        flush_stale_sessions
        connections = {}
        @response_queues.each_pair do |connection, _queue|
          forward = 'parked'
          _send, _recv, console = @router.get_forward(connection)
          unless console.nil?
            forward = "#{console}"
          end
          connections[connection] = forward
        end
        connections
      end

      def connection_info(connection)
        info = {}
        info['TIME'] = @response_queues[connection].time unless @response_queues[connection].nil?
        info
      end

      def flush_stale_sessions
        @forwarder_mutex.synchronize do
          stale_sessions = []
          @response_queues.each_pair do |payload, queue|
            unless (queue.time + CONNECTION_TIMEOUT) > Time.now
              stale_sessions << payload
            end
          end
          stale_sessions.each do |payload|
            stale_queue = @response_queues.delete(payload)
            stale_queue.stop_processing unless stale_queue.nil?
          end
        end
      end

      private :flush_stale_sessions
    end
  end
end