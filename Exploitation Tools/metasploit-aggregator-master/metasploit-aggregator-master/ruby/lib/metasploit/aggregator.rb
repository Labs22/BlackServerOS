require 'socket'
require 'openssl'
require 'thread'
require 'securerandom'

require 'metasploit/aggregator/error'
require 'metasploit/aggregator/messages_pb'
require 'metasploit/aggregator/aggregator_services_pb'
require 'metasploit/aggregator/version'
require 'metasploit/aggregator/cable'
require 'metasploit/aggregator/connection_manager'
require 'metasploit/aggregator/https_forwarder'
require 'metasploit/aggregator/http'
require 'metasploit/aggregator/logger'

module Metasploit
  module Aggregator

    class Service
      # return availability status of the service
      def available?
        # index for impl
      end

      # return the current service version found
      def version
        Metasploit::Aggregator::VERSION
      end

      # returns map of sessions available from the service
      def sessions
        # index for impl
      end

      def cables
        # index for impl
      end

      # sets forwarding for a specific session to promote
      # that session for local use, obtained sessions are
      # not reported in getSessions
      def obtain_session(payload, uuid)
        # index for impl
      end

      # parks a session and makes it available in the getSessions
      def release_session(payload)
        # index for impl
      end

      # return any extended details for the payload requested
      def session_details(payload)

      end

      # start a listening port maintained on the service
      # connections are forwarded to any registered default
      # TODO: may want to require a type here for future proof of api
      def add_cable(type, host, port, certificate = nil)
        # index for impl
      end

      def remove_cable(host, port)
        # index for impl
      end

      def register_default(uuid, payload_list)
        # index for impl
      end

      def default
        # index for impl
      end

      # returns list of IP addressed available to the service
      # TODO: consider also reporting "used" ports (may not be needed)
      def available_addresses
        # index for impl
      end

      # register the object to pass request from cables to
      def register_response_channel(requester)

      end
    end

    class ServerProxy < Service
      attr_reader :uuid
      @exit_lock = Mutex.new
      @host = @port = @socket = nil
      @no_params = nil
      @response_queue = nil
      @listening_thread = nil
      @cleanup_list = nil

      def initialize(host, port)
        @host = host
        @port = port
        @client = Metasploit::Aggregator::Pb::Stub.new("#{@host}:#{@port}", :this_channel_is_insecure)
        # TODO: add arg{ :channel_override => Core::Channel } to control connection
        @uuid = SecureRandom.uuid
        @no_params = Metasploit::Aggregator::Message::No_params.new
        # server_version = pb_to_array(@client.version(@no_params).value)[0]
        # raise CompatibilityError("server version mis-match found #{server_version}") unless server_version == version
      end

      def available?
        @client.available(@no_params).answer
      end

      def sessions
        pb_to_map(@client.sessions(@no_params).map)
      end

      def cables
        pb_to_array(@client.cables(@no_params).value)
      end


      def obtain_session(payload, uuid)
        args = Metasploit::Aggregator::Message::String_array.new( value: [payload, uuid] )
        @client.obtain_session(args).answer
      end

      def release_session(payload)
        args = Metasploit::Aggregator::Message::String_array.new( value: [payload] )
        @client.release_session(args).answer
      end

      def session_details(payload)
        args = Metasploit::Aggregator::Message::String_array.new( value: [payload] )
        pb_to_map(@client.session_details(args).map)
      end

      def add_cable(type, host, port, certificate = nil)
        args = nil
        if certificate.nil?
          args = Metasploit::Aggregator::Message::Cable_def.new( type: type, host: host, port: port.to_i )
        else
          args = Metasploit::Aggregator::Message::Cable_def.new( type: type, host: host, port: port.to_i, pem: certificate )
        end
        @client.add_cable(args).answer
      end

      def remove_cable(host, port)
        args = Metasploit::Aggregator::Message::String_array.new( value: [host, port] )
        @client.remove_cable(args).answer
      end

      def register_default(uuid, payload_list)
        uuid = "" if uuid.nil?
        payloads = []
        payloads = payload + payload_list unless payload_list.nil?
        args = Metasploit::Aggregator::Message::Register.new( uuid: uuid, payloads: payloads )
        @client.register_default(args).answer
      end

      def default
        pb_to_array(@client.default(@no_params).value)[0]
      end

      def available_addresses
        pb_to_array(@client.available_addresses(@no_params).value)
      end

      def stop(force = false)
        # end the response queue
        ServerProxy.unregister_for_cleanup(self) unless force
        @response_queue.push(self) unless @response_queue.nil?

        @listening_thread.join if @listening_thread
        @listening_thread = nil
        @client = nil
      end

      def register_response_channel(requester)
        unless requester.kind_of? Metasploit::Aggregator::Http::Requester
          raise ArgumentError("response channel class invalid")
        end
        @response_io = requester
        process
      end

      protected

      def self.register_for_cleanup(connection)
        @exit_lock.synchronize do
          unless @cleanup_list
            @cleanup_list = ::Set.new
            at_exit { ServerProxy.run_cleanup }
          end
          @cleanup_list.add connection
        end
      end

      def self.unregister_for_cleanup(connection)
        @exit_lock.synchronize do
          @cleanup_list.delete connection if @cleanup_list
        end
      end

      def self.run_cleanup
        @exit_lock.synchronize do
          if @cleanup_list
            @cleanup_list.each do |connection|
              connection.stop(true)
            end
          end
        end
      end

      private

      def pb_to_map(map)
        result = {}
        map.each do |key , value|
          result[key] = value
        end
        result
      end

      def pb_to_array(array)
        result = []
        array.each do |value|
          result << value
        end
        result
      end

      def process
        @response_queue = EnumeratorQueue.new(self)
        requests = @client.process(@response_queue.each_item)

        # add initial key response with only local uuid
        initial_response = Metasploit::Aggregator::Message::Response.new( uuid: @uuid )
        @response_queue.push(initial_response)

        @listening_thread = Thread.new do
          requests.each do |pb_request|
            request = Metasploit::Aggregator::Http::Request.new(pb_to_array(pb_request.headers), pb_request.body, nil)
            response = @response_io.process_request(request)
            session_id = Metasploit::Aggregator::Http::Request.parse_uri(request)
            pb_request = Metasploit::Aggregator::Message::Request.new( headers: response.headers, body: response.body )
            pb_response = Metasploit::Aggregator::Message::Response.new( uuid: session_id, response: pb_request)
            @response_queue.push(pb_response)
          end
        end
        ServerProxy.register_for_cleanup self
      end
    end

    # A EnumeratorQueue wraps a Queue to yield the items added to it.
    class EnumeratorQueue
      extend Forwardable
      def_delegators :@q, :push

      def initialize(sentinel)
        @q = Queue.new
        @sentinel = sentinel
      end

      def each_item
        return enum_for(:each_item) unless block_given?
        loop do
          r = @q.pop
          break if r.equal?(@sentinel)
          fail r if r.is_a? Exception
          yield r
        end
      end
    end

    class ServerImpl < Metasploit::Aggregator::Pb::Service

      def initialize
        super
        @local_server = Server.new
        @requestThreads = {}
        @listeners = []
      end

      def available(_no_params, _unused_call)
        Metasploit::Aggregator::Message::Result.new( answer: @local_server.available? )
      end

      def version(_no_params, _unused_call)
        Metasploit::Aggregator::Message::String_array.new( value: [ @local_server.version ] )
      end

      def sessions(_no_parms, _unused_call)
        Metasploit::Aggregator::Message::Result_map.new( map: @local_server.sessions() )
      end

      def cables(_no_parms, _unused_call)
        Metasploit::Aggregator::Message::String_array.new( value: @local_server.cables() )
      end

      def obtain_session(args, _unused_call)
        payload, uuid = args.value
        Metasploit::Aggregator::Message::Result.new( answer: @local_server.obtain_session(payload, uuid) )
      end

      def release_session(args, _unused_call)
        payload = args.value.shift
        Metasploit::Aggregator::Message::Result.new( answer: @local_server.release_session(payload) )
      end

      def session_details(args, _unused_call)
        payload = args.value.shift
        Metasploit::Aggregator::Message::Result_map.new( map: @local_server.session_details(payload) )
      end

      def add_cable(cable, _unused_call)
        pem = nil
        pem = cable.pem unless cable.pem.empty?
        result = @local_server.add_cable(cable.type, cable.host, cable.port, pem)
        Metasploit::Aggregator::Message::Result.new( answer: result )
      end

      def remove_cable(args, _unused_call)
        host, port = args.value
        result = @local_server.remove_cable(host, port)
        Metasploit::Aggregator::Message::Result.new( answer: result )
      end

      def register_default(register, _unused_call)
        payloads = nil
        payloads = register.payloads unless register.payloads.empty?
        result = @local_server.register_default(register.uuid, payloads)
        Metasploit::Aggregator::Message::Result.new( answer: result )
      end

      def default(_no_params, _unused_call)
        uuid = @local_server.default
        return Metasploit::Aggregator::Message::String_array.new( value: [ uuid ] ) unless uuid.nil?
        Metasploit::Aggregator::Message::String_array.new()
      end

      def available_addresses(_no_params, _unused_call)
        addresses = @local_server.available_addresses
        Metasploit::Aggregator::Message::String_array.new( value: addresses )
      end

      def process(responses)
        requests = EnumeratorQueue.new(self)
        uuid = nil

        requestingThread = Thread.new do
          loop do
            sleep 0.1 # outer loop only occurs until uuid is set
            break unless uuid.nil?
          end
          while true
            request = @local_server.request(uuid)
            # TODO: with this in place we can just get the request queue and pop each item to process and forward
            unless request.nil?
              body = ""
              body = request.body unless request.body.nil?
              pb_request = Metasploit::Aggregator::Message::Request.new( headers: request.headers, body: body )
              requests.push(pb_request)
            end
          end
        end

        Thread.new do
          responses.each do |response|
            uuid = response.uuid if uuid.nil?
            next if response.response.nil?
            request_pb = response.response
            request = Metasploit::Aggregator::Http::Request.new(request_pb.headers, request_pb.body, nil)
            @local_server.respond(response.uuid, request)
          end
          requestingThread.exit
          requestingThread.join
          requests.push(self)
        end

        requests.each_item
      end
    end

    class GrpcServer
      @exit_lock = Mutex.new

      def initialize(host, port)
        @host = host
        @port = port

        # TODO: investigate using Core::Channel to secure this communication
        # server = TCPServer.new(@host, @port)
        # sslContext = OpenSSL::SSL::SSLContext.new
        # sslContext.key, sslContext.cert = Metasploit::Aggregator::ConnectionManager.ssl_generate_certificate
        # sslServer = OpenSSL::SSL::SSLServer.new(server, sslContext)

        @svr = GRPC::RpcServer.new
        @svr.add_http2_port("#{@host}:#{@port}", :this_port_is_insecure)
        @svr.handle(ServerImpl)

        @exec = Thread.new do
          GrpcServer.register_for_cleanup(self)
          @svr.run_till_terminated
        end
      end

      def stop(force = false)
        GrpcServer.unregister_for_cleanup(self) unless force
        @svr.stop if @svr.running?
      end

      protected

      def self.register_for_cleanup(connection)
        @exit_lock.synchronize do
          unless @cleanup_list
            @cleanup_list = ::Set.new
            at_exit { GrpcServer.run_cleanup }
          end
          @cleanup_list.add connection
        end
      end

      def self.unregister_for_cleanup(connection)
        @exit_lock.synchronize do
          @cleanup_list.delete connection if @cleanup_list
        end
      end

      def self.run_cleanup
        @exit_lock.synchronize do
          if @cleanup_list
            @cleanup_list.each do |connection|
              connection.stop(true)
            end
          end
        end
      end

    end

    class Server < Service

      def initialize
        @router = Router.instance
        @manager = ConnectionManager.instance
      end

      def available?
        !@manager.nil?
      end

      def sessions
        @manager.connections
      end

      def cables
        @manager.cables
      end

      def obtain_session(payload, uuid)
        # return session object details or UUID/uri
        # forwarding will cause new session creation on the console
        # TODO: check and set lock on payload requested see note below in register_default
        @manager.register_forward(uuid, [ payload ])
        true # update later to return if lock obtained
      end

      def release_session(payload)
        @manager.park(payload)
        true # return always return success for now
      end

      def session_details(payload)
        @manager.connection_details(payload)
      end

      def add_cable(type, host, port, certificate = nil)
        unless @manager.nil?
          case type
            when Cable::HTTPS
              # TODO: check if already listening on that port
              @manager.add_cable_https(host, port, certificate)
            when Cable::HTTP
              @manager.add_cable_http(host, port)
            else
              Logger.log("#{type} cables are not supported.")
          end
        end
        true
      end

      def remove_cable(host, port)
        unless @manager.nil?
          @manager.remove_cable(host, port)
        end
      end

      def register_default(uuid, payload_list)
        # add this payload list to each forwarder for this remote console
        # TODO: consider adding boolean param to ConnectionManager.register_forward to 'lock'
        @manager.register_forward(uuid, payload_list)
        true
      end

      def default
        _send, _recv, console = @router.get_forward('default')
        console
      end

      def available_addresses
        addr_list = Socket.ip_address_list
        addresses = []
        addr_list.each do |addr|
          addresses << addr.ip_address
        end
        addresses
      end

      def stop
        unless @manager.nil?
          @manager.stop
        end
        @manager = nil
        true
      end

      def request(uuid)
        # return requests here
        send, _recv = @router.reverse_route(uuid)
        if send.length > 0
          result = send.pop
        end
        result
      end

      def respond(uuid, data)
        _send, recv = @router.get_forward(uuid)
        recv << data unless recv.nil?
        true
      end

      def register_response_channel(io)
        # not implemented "client only method"
        response = "register_response_channel not implemented on server"
        Logger.log response
        response
      end
    end # class Server
  end
end
