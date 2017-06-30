require 'singleton'

module Metasploit
  module Aggregator
    class Router
      include Singleton

      def initialize
        @mutex = Mutex.new
        @forward_routes = {}
        @queue_by_uuid = {}
      end

      def add_route(uuid, payload)
        # for now always replace in future may check if same route to avoid request loss
        forward = uuid.nil? ? [] : [Queue.new, Queue.new, uuid]
        @mutex.synchronize do
          if payload.nil?
            @forward_routes['default'] = forward
            return
          end
          @forward_routes[payload] = forward
        end
      end

      def remove_route(payload)
        unless payload.nil?
          @mutex.synchronize do
            @forward_routes.delete(payload)
          end
        end
      end

      def get_forward(payload)
        unless @forward_routes[payload].nil?
          @forward_routes[payload]
        else
          @forward_routes['default']
        end
      end

      def reverse_route(uuid)
        # this method is not ready for prime time yet as will not
        # support multiple sessions properly, either need unique Queue
        # for each session which can be identified on reverse or need
        # pipeline request order to ensure all results are posted in order only
        # this could create deadlocks while one session waits for a response
        # if another fails to respond.

        # try returning a uuid queue filled an aggregate of queues for all requests
        # for a reverse uuid, and refill that on each reverse, this puts logic
        # here that I do not like but accomplishes the requirement for now
        unless @queue_by_uuid[uuid]
          @queue_by_uuid[uuid] = Queue.new
        end
        @forward_routes.each_pair do |_key, val|
          request, _response, remote_uuid = val
          next unless remote_uuid == uuid
          while !request.empty?
            @queue_by_uuid[uuid] << request.pop
          end
        end
        [@queue_by_uuid[uuid], nil, uuid]
      end
    end
  end
end