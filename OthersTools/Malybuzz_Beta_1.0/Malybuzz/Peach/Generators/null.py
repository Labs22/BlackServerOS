
'''
These Generators evaluate to empty strings but are usefull for displaying status
messages and other random stuff.

@author: Michael Eddington
@version: $Id: null.py 279 2007-05-08 01:21:58Z meddingt $
'''

#
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

# Authors:
#   Michael Eddington (mike@phed.org)

# $Id: null.py 279 2007-05-08 01:21:58Z meddingt $


from Peach import generator
import sys

#__all__ = ["PrintStdout", "PrintStderr"]

class PrintStdout(generator.Generator):
	'''
	Logical value of empty string, but will display a value to stdout
	when called.  Usefull for displaying status messages.
	'''
	
	_msg = None
	_generator = None
	
	def __init__(self, msg, generator = None):
		'''
		@type	msg: string
		@param	msg: Value to output
		@type	generator: Generator
		@param	generator: Generator to wrap
		'''
		self._msg = msg
		self._generator = generator
	
	def getRawValue(self):
		print self._msg
		
		if self._generator:
			return self._generator.getRawValue()
		
		return ''
	
	def next(self):
		if self._generator:
			self._generator.next()
		else:
			raise generator.GeneratorCompleted("PrintStdout")


class PrintStderr(generator.Generator):
	'''
	Logical value of empty string, but will display a value to stderr
	when called.  Usefull for displaying status messages.
	'''
	
	_msg = None
	_generator = None
	
	def __init__(self, msg, generator = None):
		'''
		@type	msg: string
		@param	msg: Value to output
		@type	generator: Generator
		@param	generator: Generator to wrap
		'''
		self._msg = msg
		self._generator = generator
	
	def getRawValue(self):
		sys.stderr.write(self._msg + '\n')
		
		if self._generator:
			return self._generator.getRawValue()
		
		return ''
	
	def next(self):
		if self._generator:
			self._generator.next()
		else:
			raise generator.GeneratorCompleted("PrintStderr")
	

# end
