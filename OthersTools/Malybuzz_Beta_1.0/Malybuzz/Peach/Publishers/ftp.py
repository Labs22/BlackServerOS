
'''
Default file publishers.

@author: Michael Eddington
@version: $Id: ftp.py 279 2007-05-08 01:21:58Z meddingt $
'''

#
# Copyright (c) 2005-2006 Michael Eddington
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

# Authors:
#   Michael Eddington (mike@phed.org)

# $Id: ftp.py 279 2007-05-08 01:21:58Z meddingt $


import socket
from Peach.publisher import Publisher

#__all__ = ['BasicFtp']

class BasicFtp(Publisher):
	'''
	Like tcp.Tcp except it will wait for a 220 line on start and
	attempt to send a QUIT commant on stop.
	'''
	
	_host = None
	_port = None
	_socket = None
	
	def __init__(self, host, port):
		'''
		@type	host: string
		@param	host: Remove host
		@type	port: number
		@param	port: Port number
		'''
		self._host = host
		self._port = port
	
	def start(self):
		if self._socket != None:
			# Close out old socket first
			self._socket.close()
		self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self._socket.connect((self._host,self._port))
		ret = self._socket.recv(4)
		if ret != '220 ':
			print "NO 220 FOUND [%s]" % ret
			raise publisher.PublisherStartError()
		else:
			print "FOUND 220! [%s]" % ret
	
	def stop(self):
		if self._socket != None:
			try:
				self._socket.sendall("\r\nQUIT\r\n")
			except:
				pass
			
			self._socket.close()
			self._socket = None
	
	def send(self, data):
		self._socket.sendall(data)
	
	def receive(self):
		#self._socket.setblocking(0)
		ret = self._socket.recv(10000)
		#self._socket.setblocking(1)
		return ret

def unittest():
	pass

# end

