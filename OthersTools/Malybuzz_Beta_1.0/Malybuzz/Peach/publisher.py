
'''
Base Publisher object implementation.

@author: Michael Eddington
@version: $Id: publisher.py 279 2007-05-08 01:21:58Z meddingt $
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

# $Id: publisher.py 279 2007-05-08 01:21:58Z meddingt $

#__all__ = ["Publisher", "PublisherStartError", "PublisherStopError"]

class Publisher:
	'''
	The Publisher object(s) implement a way to send and/or receave
	data by some means.  This could be a TCP connection, a filehandle, or
	SQL, etc.
	
	I would like to have a good generic publisher interface such that one
	could swap out publishers to target different things.  Currently this
	interface needs more work.
	'''
	
	def start(self):
		'''
		Change state such that send/receave will work.  For Tcp this
		could be connecting to a remote host
		'''
		pass
	def stop(self):
		'''
		Change state such that send/receave will not work.  For Tcp this
		could be closing a connection, for a file it might be closing the
		file handle.
		'''
		pass
	def send(self, data):
		'''
		Publish some data
		
		@type	data: string
		@param	data: Data to publish
		'''
		pass
	def receive(self):
		'''
		Receive some data.
		
		@rtype: string
		@return: data received
		'''
		pass
		
class PublisherStartError(Exception):
	'''
	Exception thrown if error occurs during start call
	'''
	pass

class PublisherStopError(Exception):
	'''
	Exception thrown if error occurs during stop call
	'''
	pass


def unittest():
	pass

# end
