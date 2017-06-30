require "metasploit/aggregator/session_detail_service"
require "metasploit/aggregator/http/request"

module Metasploit
  module Aggregator
    module Http
      # a Responder acts a a gateway to convert data from a port to into a Request object
      # used in the aggregator. It also reverses this process as a gateway for sending Request object
      # back as responses to the original Request.
      class Responder

        attr_accessor :queue
        attr_accessor :time
        attr_accessor :log_messages
        attr_reader :uri

        def initialize(uri)
          @uri = uri
          @queue = Queue.new
          @thread = Thread.new { process_requests }
          @time = Time.now
          @router = Router.instance
          @session_service = SessionDetailService.instance
          @pending_request = nil
        end

        def process_requests

          while true do
            begin
              request_task = @queue.pop
              connection = request_task.socket
              request_task.headers

              send, recv = @router.get_forward(@uri)
              if send.nil?
                # when no forward found park the connection for now
                # in the future this may get smarter and return a 404 or something
                send_parked_response(connection)
                next
              end

              # response from get_forward will be a queue to push messages onto and a response queue to retrieve result from
              @session_service.add_request(request_task, @uri)
              send << request_task
              @pending_request = connection

              log 'queued to console'

              # now get the response once available and send back using this connection
              begin
                request_obj = recv.pop
                @session_service.add_request(request_task, @uri)
                @pending_request = nil
                request_obj.headers.each do |line|
                  connection.write line
                end
                unless request_obj.body.nil?
                  connection.write request_obj.body
                end
                connection.flush
                log 'message delivered from console'
              rescue
                log $!
              end
              close_connection(connection)
            rescue Exception => e
              log "an error occurred processing request from #{@uri}"
            end
          end

        end

        def stop_processing
          @thread.exit
          if @pending_request
            send_parked_response(@pending_request)
            close_connection(@pending_request)
          end
        end

        def send_parked_response(connection)
          address = connection.peeraddr[3]
          log "sending parked response to #{address}"
          Metasploit::Aggregator::Http::Request.parked.headers.each do |line|
            connection.puts line
          end
          close_connection(connection)
        end

        def self.get_data(connection, guaranteed_length)
          checked_first = has_length = guaranteed_length
          content_length = 0
          request_lines = []

          while (input = connection.gets)
            request_lines << input
            # break for body read
            break if (input.inspect.gsub /^"|"$/, '').eql? '\r\n'

            if !checked_first && !has_length
              has_length = input.include?('POST')
              checked_first = true
            end

            if has_length && input.include?('Content-Length')
              content_length = input[(input.index(':') + 1)..input.length].to_i
            end

          end
          body = ''
          if has_length
            while body.length < content_length
              body += connection.read(content_length - body.length)
            end
          end
          Request.new request_lines, body, connection
        end

        def get_connection(host, port)
          TCPSocket.new host, port
        end

        def close_connection(connection)
          connection.close
        end

        def log(message)
          Logger.log message if @log_messages
        end

        private :log
        private :send_parked_response
      end
    end
  end
end