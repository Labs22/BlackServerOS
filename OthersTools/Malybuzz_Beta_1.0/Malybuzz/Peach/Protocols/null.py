
'''
Null protocols are protocols that don't implement any state management.  Usefull
for simple fuzzers and testing.

@author: Michael Eddington
@version: $Id: null.py 279 2007-05-08 01:21:58Z meddingt $
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

# $Id: null.py 279 2007-05-08 01:21:58Z meddingt $

import sys
from Peach.protocol import Protocol

#__all__ = ['Null', 'NullStdout']

class Null(Protocol):
	'''
	A protocol with no state, just a single Generator.  Single step.
	'''
	
	_generator = None
	_verbose = 0
	
	def __init__(self, publisher = None, generator = None, verbose = 0):
		'''
		@type	publisher: Publisher
		@param	publisher: Publisher to use
		@type	generator: Generator
		@param	generator: Generator to use
		@type	verbose: number
		@param	verbose: Should protocol be verbose
		'''
		self.setGenerator(generator)
		self.setPublisher(publisher)
		self._verbose = verbose
	
	def getGenerator(self):
		'''
		Get current Generator.
		
		@rtype: Generator
		@return: current Generator
		'''
		return self._generator
	def setGenerator(self, generator):
		'''
		Set new Generator.
		
		@type	generator: Generator
		@param	generator: Generator to use
		'''
		self._generator = generator
	
	def step(self):
		data = self.getGenerator().getValue()
		
		if self._verbose == 1:
			print "Null::step(): Trying with [%s]" % data
		
		self.getPublisher().start()
		self.getPublisher().send(  self.getGenerator().getValue() )
		self.getPublisher().stop()

class NullStdout(Protocol):
	'''
	A protocol with no state, just a single Generator.  Single step.
	Will echo any receeaved data to stdout.
	'''
	
	_generator = None
	_verbose = 0
	
	def __init__(self, publisher = None, generator = None, verbose = 0):
		'''
		@type	publisher: Publisher
		@param	publisher: Publisher to use
		@type	generator: Generator
		@param	generator: Generator to use
		@type	verbose: number
		@param	verbose: Should protocol be verbose
		'''
		self.setGenerator(generator)
		self.setPublisher(publisher)
		self._verbose = verbose
	
	def getGenerator(self):
		'''
		Get current Generator.
		
		@rtype: Generator
		@return: current Generator
		'''
		return self._generator
	def setGenerator(self, generator):
		'''
		Set new Generator.
		
		@type	generator: Generator
		@param	generator: Generator to use
		'''
		self._generator = generator
	
	def step(self):
		data = self.getGenerator().getValue()
		
		if self._verbose == 1:
			print "NullStdout::step(): Trying with [%s]" % data
		
		self.getPublisher().start()
		self.getPublisher().send( data )
		
		try:
			print self.getPublisher().receive()
		except AttributeError, e:
			print "NullStdout::step(): Caught error on receave, ignoring [%s]" % e
		except:
			print "NullStdout::step(): Caught error on receave, ignoring [%s]" % sys.exc_info()[0]
		
		self.getPublisher().stop()

def unittest():
	pass

# end

