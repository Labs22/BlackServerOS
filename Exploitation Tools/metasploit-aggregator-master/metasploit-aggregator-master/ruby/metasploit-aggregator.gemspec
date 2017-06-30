# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'metasploit/aggregator/version'

Gem::Specification.new do |spec|
  spec.name          = "metasploit-aggregator"
  spec.version       = Metasploit::Aggregator::VERSION
  spec.authors       = ['Metasploit Hackers']
  spec.email         = ['metasploit-hackers@lists.sourceforge.net']
  spec.summary       = "metasploit-aggregator"
  spec.description   = "metasploit-aggregator"
  spec.homepage      = 'https://www.msf.com'
  spec.license       = 'BSD-3-Clause'

  spec.files         = `git ls-files -z`.split("\x0").reject do |f|
    f.match(%r{^(test|spec|features)/})
  end
  spec.bindir        = "bin"
  spec.executables << 'metasploit-aggregator'
  spec.require_paths = ["lib"]
  spec.platform      = Gem::Platform::RUBY

  spec.required_ruby_version = '>= 2.2.0'

  spec.add_development_dependency "bundler", "~> 1.13"
  spec.add_development_dependency "pry"
  spec.add_development_dependency "rake", "~> 10.0"
  spec.add_development_dependency "rspec", "~> 3.0"
  spec.add_development_dependency "grpc-tools"
  spec.add_runtime_dependency 'grpc'
  spec.add_runtime_dependency 'rex-arch'
end
