require 'spec_helper'

describe Metasploit::Aggregator do
  it "has a version number" do
    expect(Metasploit::Aggregator::VERSION).not_to be nil
  end

  it "provides all classes for interaction" do
    is_expected.to satisfy { defined? Metasploit::Aggregator::Service }
    is_expected.to satisfy { defined? Metasploit::Aggregator::Server }
    is_expected.to satisfy { defined? Metasploit::Aggregator::ServerProxy }
    is_expected.to satisfy { defined? Metasploit::Aggregator::GrpcServer }
  end

  describe "GrpcServer" do

    context "a GrpcServer is created" do
      ip = '127.0.0.1'
      port = 2447
      context "given the localhost ip '#{ip}' and port '#{port}'" do
        subject do
          Metasploit::Aggregator::GrpcServer.new(ip, port)
        end

        context "a ServerProxy is created" do
          context "given the localhost ip '#{ip}' and port '#{port}'" do

            subject do
              Metasploit::Aggregator::ServerProxy.new(ip, port)
            end

            it { is_expected.to be_a Metasploit::Aggregator::Service }
            it { is_expected.to respond_to(:available?) }
            it { is_expected.to respond_to(:sessions) }
            it { is_expected.to respond_to(:obtain_session) }
            it { is_expected.to respond_to(:release_session) }
            it { is_expected.to respond_to(:cables) }
            it { is_expected.to respond_to(:add_cable) }
            it { is_expected.to respond_to(:remove_cable) }
            it { is_expected.to respond_to(:register_default) }
            it { is_expected.to respond_to(:available_addresses) }

            after do
              subject.stop
            end
          end
        end

        after do
          subject.stop
        end

        it { is_expected.to respond_to(:stop) }
      end
    end
  end

  describe "Service" do
    subject do
      Metasploit::Aggregator::Service.new
    end

    it { is_expected.to respond_to(:available?) }
    it { is_expected.to respond_to(:sessions) }
    it { is_expected.to respond_to(:obtain_session) }
    it { is_expected.to respond_to(:release_session) }
    it { is_expected.to respond_to(:cables) }
    it { is_expected.to respond_to(:add_cable) }
    it { is_expected.to respond_to(:remove_cable) }
    it { is_expected.to respond_to(:register_default) }
    it { is_expected.to respond_to(:available_addresses) }
  end

  describe "Server" do
    subject do
      Metasploit::Aggregator::Server.new
    end

    it { is_expected.to be_a Metasploit::Aggregator::Service }
    it { is_expected.to respond_to(:available?) }
    it { is_expected.to respond_to(:sessions) }
    it { is_expected.to respond_to(:obtain_session) }
    it { is_expected.to respond_to(:release_session) }
    it { is_expected.to respond_to(:cables) }
    it { is_expected.to respond_to(:add_cable) }
    it { is_expected.to respond_to(:remove_cable) }
    it { is_expected.to respond_to(:register_default) }
    it { is_expected.to respond_to(:available_addresses) }
  end

end
