'''
Peach testing client.  This is an XML-RPC client that communicates with
the server portion (twistedserver.py) to store test runs in a MySQL database.

@author: Michael Eddington
@version: $Id: client.py 279 2007-05-08 01:21:58Z meddingt $
'''

#
# Copyright (c) 2006-2007 Michael Eddington
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

# $Id: client.py 279 2007-05-08 01:21:58Z meddingt $

import sys, string, time, os, base64, uuid, socket
from xmlrpclib import *

class PeachClient:
	
	_server = None
	_run = None
	
	def __init__(self, url, client = None):
		'''
		A Peach reporting server client.  Communicates to server
		portion (twistedserver.py) via XML-RPC.  Many clients can send
		results to a single server for collection.
		
		NOTE: DNS lookup is *slow*, use and IP address even for localhost!
		
		@type	url: string
		@param	url: URL of Peach server ("http://127.0.0.1:8000")
		@type	client: string
		@param	client: Name of client (defaults to hostname)
		'''
		self._server = ServerProxy(url)
		
		if client == None:
			self._client = socket.gethostname()
		else:
			self._client = client
		
		self._run = None
	
	def RunStarting(self):
		'''
		Note start of test run.  A test run is a collection of tests that
		are executed as a group.  Typically each time you run a fuzzer that
		would be a test run.
		'''
		self._run = str(uuid.uuid4())
		self._server.RunStarting(self._run, self._client)
	
	def StartTest(self, testName, testNum, testData = None):
		'''
		Indicate start of a test.
		
		@type 	testName: string
		@param	testName: Name of test, must match EndTest call
		@type	testNum: int
		@param	testNum: Test #, must match EndTest call
		@type	testData: string
		@param	testData: [Optional] Test data generated
		'''
		
		if self._run == None:
			raise PeachClientException("Error: Call RunStarting first!")
		
		if testData == None:
			testData = ""
		
		self._server.StartTest(self._run, testName,
			testNum, base64.encodestring(testData).rstrip())
	
	def EndTest(self, testName, testNum, result = None):
		'''
		Indicate end of a test
		
		@type 	testName: string
		@param	testName: Name of test, must match StartTest call
		@type	testNum: int
		@param	testNum: Test #, must match StartTest call
		@type	testData: string
		@param	testData: [Optional] Test data generated
		'''
		if self._run == None:
			raise PeachClientException("Error: Call RunStarting first!")
		
		if result == None:
			result = ""
		
		self._server.EndTest(self._run, testName,
			testNum, base64.encodestring(result).rstrip())
	
	def RunFinished(self):
		'''
		Note end of test run.
		'''
		
		if self._run == None:
			raise PeachClientException("Error: Call RunStarting first!")
		
		self._server.RunFinished(self._run)
		self._run = None


class PeachClientException(Exception):
	'''
	Yikes!  Something bad happened... Or maybe it was your fault ;)
	'''
	pass

# end
