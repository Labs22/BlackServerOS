
'''
Some default script classes.  These classes contain common code to perform
protocol step and group increment.

@author: Michael Eddington
@version: $Id: script.py 279 2007-05-08 01:21:58Z meddingt $
'''

#
# Copyright (c) 2005-2007 Michael Eddington
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

# $Id: script.py 279 2007-05-08 01:21:58Z meddingt $

import time,sys,os,exceptions
import traceback

from Peach	import *
from Peach	import group, protocol, publisher, generator
from Peach.Server.client import PeachClient

class Script:
	'''
	This is a basic script implementation that takes in a protocol
	and a single group with optional wait time between each try.
	'''
	
	_protocol = None
	_group = None
	_waitTime = None
	_counter = None
	
	def __init__(self, protocol, group, waitTime = 0):
		'''
		All params should be odviouse, waitTime is time to pause between
		each try.
		
		@type	protocol: Protocol
		@param	protocol: Protocol to run
		@type	group: Group
		@param	group: Group to iterate
		@type	waitTime: number
		@param	waitTime: Time to wait between iterations/steps.  Can be frac.
		'''
		
		self._protocol = protocol
		self._waitTime = waitTime
		self._group = group
	
	def go(self):
		'''
		Run stuff!
		'''
		
		try:
			self._counter = 1
			while 1 == 1:
				time.sleep(self._waitTime)
				self._protocol.step()
				self._group.next()
				self._counter += 1
		
		except generator.GeneratorCompleted, e:
			print "Script: Odd, caught GeneratorCompleted exception [%s]" % e
			traceback.print_tb(sys.exc_info()[2])
		
		except AttributeError, e:
			print "Script: Odd, caught AttributeError exception [%s]" % e
			traceback.print_tb(sys.exc_info()[2])
		
		except group.GroupCompleted:
			print "Script: Finished -- May indicate that nothing was found!"
		
		except:
			print "Script: Caught unknown error [%s] using data block [%s] on step [%d]" % (
				sys.exc_info()[0], self._protocol.getGenerator().getValue(), self._counter)
			traceback.print_tb(sys.exc_info()[2])


class ScriptCmd:
	'''
	This is a basic script implementation that takes in a protocol
	and a single group with optional wait time between each try.
	'''
	
	_command = None
	_parameters = None
	_group = None
	_waitTime = None
	_counter = None
	_testStart = 0
	_clientArgs = None
	_client = None
	
	def __init__(self, group, command, parameters, waitTime = 0, client = None):
		'''
		All params should be odviouse, waitTime is time to pause between
		each try.
		
		@type	group: Group
		@param	group: Group to iterate
		@type	command: string
		@param	command: Executable to run
		@type	parameters: Array of Generators
		@param	parameters: Array of Generators to use as parameters
		@type	waitTime: number
		@param	waitTime: Time to wait between iterations/steps.  Can be frac.
		@type	client: Array
		@param	client: PeachClient if one is wanted, [ url, client name, testName ]
		'''
		
		self._command = command
		self._parameters = parameters
		self._waitTime = waitTime
		self._group = group
		if client != None:
			self._clientArgs = client
			self._client = PeachClient(client[0], client[1])
	
	def go(self):
		'''
		Run stuff!
		'''
		
		self.handleCommandline()
		self._waitTime = 0
		self._counter = 1
		params = None
		
		if self._client:
			self._client.RunStarting()
		
		try:
			while 1 == 1:
				time.sleep(self._waitTime)
				
				params = []
				for g in self._parameters:
					params.append(g.getValue())
				
				if self._testStart >= 0 and self._counter >= self._testStart:
					try:
						if self._client != None:
							self._client.StartTest(self._clientArgs[2], self._counter, str(params))
						
						print "Test %d" % self._counter
						
						ret = os.spawnv(os.P_WAIT, 
							self._command, params )
						
						if self._client != None:
							self._client.EndTest(self._clientArgs[2], self._counter, "")
						
					except exceptions.TypeError:
						pass
					except exceptions.NameError:
						pass
					
					#if ret != 0:
					#	raise StopIteration("Looks like a fault to me!")
				
				self._group.next()
				self._counter += 1

		except generator.GeneratorCompleted, e:
			print "Script: Odd, caught GeneratorCompleted exception [%s]" % e
			traceback.print_tb(sys.exc_info()[2])
		
		except group.GroupCompleted:
			if self._testStart == -1:
				print "\nScript: Counted %d tests" % self._counter
				
			else:
				print "\nScript: Finished -- May indicate that nothing was found!"
				print "Script: %d tests performed" % self._counter
		
		except AttributeError, e:
			print "Script: Odd, caught AttributeError exception [%s] test [%d]" % (e, self._counter)
			traceback.print_tb(sys.exc_info()[2])
		
		except:
			print "Script: Caught unknown error [%s] using data block [%s] on test [%d]" % (
				sys.exc_info()[0], 
				params,
				self._counter)
			traceback.print_tb(sys.exc_info()[2])
		
		if self._client:
			self._client.RunFinished()
	
	def handleCommandline(self):
		if len(sys.argv) > 1:
			if sys.argv[1] == "count":
				print "Script: Counting test cases...\n"
				self._testStart = -1
			else:
				self._testStart = int(sys.argv[1])
				print "Script: Skipping to test case [%d]\n" % self._testStart
	
	
def unittest():
	pass

# end

