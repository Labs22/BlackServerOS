class Logger
  def self.log(message)
    $stderr.puts Time.now.to_s + ' ' + message
  end
end