require 'digest/md5'
require 'singleton'
require 'metasploit/aggregator/tlv/packet'
require 'metasploit/aggregator/tlv/packet_parser'
require 'metasploit/aggregator/tlv/uuid'

module Metasploit
  module Aggregator
    class SessionDetailService
      include Singleton

      def initialize
        @mutex = Mutex.new
        @tlv_queue = Queue.new
        @thread = Thread.new { process_tlv }
        # for now since all data is http no cipher is required on the parser
        @parser = Metasploit::Aggregator::Tlv::PacketParser.new
        @r, @w = IO.pipe
        @payloads_count = 0;
        @detail_cache = {}
      end

      def add_request(request, payload)
        @tlv_queue << [ request, payload ]
        if @detail_cache[payload] && @detail_cache[payload]['REMOTE_SOCKET'].nil?
          @detail_cache[payload]['REMOTE_SOCKET'] = "#{request.socket.peeraddr[3]}:#{request.socket.peeraddr[1]}"
          @detail_cache[payload]['LOCAL_SOCKET'] = "#{request.socket.addr[3]}:#{request.socket.addr[1]}"
        end
      end

      def session_details(payload)
        @detail_cache[payload]
      end

      def process_tlv
        while true
          begin
            request, payload = @tlv_queue.pop
            if request.body && request.body.length > 0
              # process body as tlv
              @w.write(request.body)
              packet = @parser.recv(@r)
              next unless packet
              unless @detail_cache[payload]
                @detail_cache[payload] = { 'ID' => (@payloads_count += 1) }
              end
              if packet.has_tlv?(Metasploit::Aggregator::Tlv::TLV_TYPE_UUID)
                args = { :raw => packet.get_tlv_value(Metasploit::Aggregator::Tlv::TLV_TYPE_UUID) }
                @detail_cache[payload]['UUID'] = Metasploit::Aggregator::Tlv::UUID.new(args)
              end
              if packet.has_tlv?(Metasploit::Aggregator::Tlv::TLV_TYPE_MACHINE_ID)
                @detail_cache[payload]['MachineID'] = Digest::MD5.hexdigest(packet.get_tlv_value(Metasploit::Aggregator::Tlv::TLV_TYPE_MACHINE_ID).downcase.strip)
              end
              if packet.has_tlv?(Metasploit::Aggregator::Tlv::TLV_TYPE_USER_NAME)
                @detail_cache[payload]['USER'] = packet.get_tlv_value(Metasploit::Aggregator::Tlv::TLV_TYPE_USER_NAME)
              end
              if packet.has_tlv?(Metasploit::Aggregator::Tlv::TLV_TYPE_COMPUTER_NAME)
                @detail_cache[payload]['HOSTNAME'] = packet.get_tlv_value(Metasploit::Aggregator::Tlv::TLV_TYPE_COMPUTER_NAME)
              end
              if packet.has_tlv?(Metasploit::Aggregator::Tlv::TLV_TYPE_OS_NAME)
                @detail_cache[payload]['OS'] = packet.get_tlv_value(Metasploit::Aggregator::Tlv::TLV_TYPE_OS_NAME)
              end
            end
          rescue
            Logger.log $!
          end
        end
      end
    end
  end
end