
'''
Base protocol object implementation.

@author: Michael Eddington
@version: $Id: protocol.py 279 2007-05-08 01:21:58Z meddingt $
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

# $Id: protocol.py 279 2007-05-08 01:21:58Z meddingt $

#__all__ = ["Protocol"]

class Protocol:
	'''
	A Protocol ties Generators to Publishers with the ability to
	manage state transitions.  For example, one might write a generator to 
	produce a TCP packet and a Protocol object to manage state between
	TCP packets (sequence numbers, etc).
	'''
	
	_publisher = None
	
	def getPublisher(self):
		'''
		Get current Publisher object.
		
		@rtype: Publisher
		@return: current Publisher object
		'''
		return self._publisher
	def setPublisher(self, publisher):
		'''
		Set Publisher object.
		
		@type	publisher: Publisher
		@param	publisher: Publisher to set
		'''
		self._publisher = publisher
	
	def step(self):
		'''
		Called to run a single 'round'.  Does not include incrementing
		Generators.
		'''
		pass

def unittest():
	pass

# end
