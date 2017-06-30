
'''
Default UDP publishers.

Modified by Jose Miguel Esparza to use in Malybuzz fuzzer
'''

#
# Copyright 2007 Jose Miguel Esparza
# Copyright (c) 2005 Michael Eddington
# Copyright (c) 2004-2005 IOActive Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy 
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights 
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
# copies of the Software, and to permit persons to whom the Software is 
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in	
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#


import socket, sys, logging
from Peach.publisher import Publisher
from Utils.misc import error

#__Udp__ = ['Udp']

class Udp(Publisher):
	'''
	A simple UDP publisher.
	'''
	
	_host = None
	_port = None
	_socket = None
	_sourcePort = None
	_timeout = None
	_verbose = None
	_logFile = None
	_logMode = None
	
	def __init__(self, host, port, parametersList):
		'''
		@type	host: string
		@param	host: Remote host
		@type	port: number
		@param	port: Remote port
		@type	parametersList: list
		@param	parametersList: configuration parameters of the connection
		'''
		
		self._host = host
		self._port = port
		self._protocol = parametersList[1]
		self._sourcePort = parametersList[2]
		if parametersList[3] == None:
			# Default value
			self._timeout = 100000
		else:
			self._timeout = parametersList[3]
		self._verbose = parametersList[4]
		self._logMode = parametersList[5]
		if self._logMode:
			self._logFile = logging.getLogger("Udp")
		
	def getSocket(self):
		'''
		This is a cheet for lack of a really good publisher interface.
		'''
		return self._socket

	def getHost(self):
		return self._host

	def getPort(self):
		return self._port
	
	def getProtocol(self):
		return self._protocol

	def getTimeout(self):
		return self._timeout
	
	def start(self):
		'''Create connection'''
		
		connectionData = ""
		try:
			if self._socket != None:
				# Close out old socket first
				self._socket.close()
						# Connection information to print later
			connectionData += "Creating socket to connect with "+self._host+" in udp port "+str(self._port)+"..."+"\n"
			if self._logMode:
				self._logFile.info("Creating socket to connect with "+self._host+" in udp port "+str(self._port)+"...\n")
			self._socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
			if self._sourcePort != 0:
				# Setting the source port if specified
				self._socket.bind(("",self._sourcePort))
			connectionData += "Socket created successfully"
			if self._logMode:
				self._logFile.info("Socket created successfully")
			if self._timeout > 0:
				# Setting the timeout for connections if specified
				self._socket.settimeout(self._timeout/1000)
			return connectionData
		
		except socket.error,e:
			if self._socket != None:
				self._socket.close()
			error(self._logMode, self._logFile, "Socket Error: \""+str(e[1])+"\".")
			
		except: 
			self._socket.close()
			error(self._logMode, self._logFile, "Communication Error: Problems while starting the connection.","\""+str(sys.exc_info()[0])+str(sys.exc_info()[1])+"\"\n"+str(sys.exc_info()[2]))
	
	def stop(self):
		'''Close connection if open'''
		if self._socket != None:
			self._socket.close()
			self._socket = None
	
	def send(self, data):
		'''
		Send data via sendall.
		
		@type	data: string
		@param	data: Data to send
		'''
		try:
			if self._logMode:
				self._logFile.debug("Before sendto...")
			self._socket.sendto(data,(self._host,self._port))
			if self._logMode:
				self._logFile.debug("After sendto...")
				
		except socket.error, e:
			if str(e)=="timed out":
				print "\nSending Timeout ("+str(self._timeout)+"ms): possible positive fuzzing while sending command"
				if self._logMode:
					self._logFile.warning("\nSending Timeout ("+str(self._timeout)+"ms): possible positive fuzzing while sending command")
			raise
		
		except:
			raise
	
	def receive(self):
		'''
		Receive upto 10000 bytes of data.
		
		@rtype: string
		@return: received data
		'''
		try:
			if self._logMode:
				self._logFile.debug("Before recv...")
			ret = self._socket.recv(10000)
			if self._logMode:
				self._logFile.debug("After recv...")
		except socket.error, e:
			if str(e)=="timed out":
				print "\nReceiving Timeout ("+str(self._timeout)+"ms): possible positive fuzzing while waiting for response"
				if self._logMode:
					self._logFile.warning("\nReceiving Timeout ("+str(self._timeout)+"ms): possible positive fuzzing while waiting for response")
			raise
		except:
			raise
		return ret


# end
