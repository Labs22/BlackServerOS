
'''
Unicode generators

@author: Michael Eddington
@version: $Id: unicode.py 279 2007-05-08 01:21:58Z meddingt $
'''

#
# Copyright (c) 2006 Michael Eddington
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

# $Id: unicode.py 279 2007-05-08 01:21:58Z meddingt $

import re, struct
from Peach import generator
from Peach.Generators.dictionary  import *
from Peach.Generators.static  import *

#__all__ = ['OverLongUtf8']

class GoodUnicode(generator.SimpleGenerator):
	_generator = Static('PLACE HOLDER')

	def __init__(self, group, generator):
		'''
		@type	group: Group
		@param	group: Group to use
		'''
		
		Generator.__init__(self)
		self.setGroup(group)

class BadUnicode(generator.SimpleGenerator):
	_generator = Static('PLACE HOLDER')

	def __init__(self, group, generator):
		'''
		@type	group: Group
		@param	group: Group to use
		'''
		
		Generator.__init__(self)
		self.setGroup(group)

class OverLongUtf8(generator.Generator):
	'''
	This generator creates overlong UTF-8 encodings.  First output
	is correct notation, then on each generation we perform a longer
	encoding of each character until we can do no more.
	
	NOTE: Only supports ascii chars under 127 right now :/
	'''
	
	_data = None
	_size = 1
	_maxSize = 6
	_emptyByte = 0x80
	_start2 = 0xC0
	_start3 = 0xE0
	_start4 = 0xF0
	_start5 = 0xF8
	_start6 = 0xFC
	_firstMask = 0xC0
	_lastMask = 0x80
	
	def __init__(self, group, data):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	data: Generator
		@param	data: Data to perform UTF-8 encoding on
		'''
		self.setGroup(group)
		self._data = data
	
	def next(self):
		self._size += 1
		if self._size > self._maxSize:
			raise generator.GeneratorCompleted("OverLongUtf8")
	
	def getRawValue(self):
		data = self._data.getValue()
		ret = ''
		
		if self._size == 1:
			return data
			
		elif self._size == 2:
			for c in data:
				ret += "%c%c" % (self._start2, self._lastMask | ord(c))
			
		elif self._size == 3:
			for c in data:
				ret += "%c%c%c" % (self._start3, self._emptyByte,
								   self._lastMask | ord(c))
			
		elif self._size == 4:
			for c in data:
				ret += "%c%c%c%c" % (self._start4, self._emptyByte,
									 self._emptyByte, self._lastMask | ord(c))
			
		elif self._size == 5:
			for c in data:
				ret += "%c%c%c%c%c" % (self._start5, self._emptyByte,
									   self._emptyByte, self._emptyByte,
									   self._lastMask | ord(c))
			
		elif self._size == 6:
			for c in data:
				ret += "%c%c%c%c%c%c" % (self._start6, self._emptyByte,
										 self._emptyByte, self._emptyByte,
										 self._emptyByte, self._lastMask | ord(c))
		
		return ret
	
	def reset(self):
		self._size = 1
	
	def unittest():
		expected1 = "%c" % 			(0x0A)
		expected2 = "%c%c" % 		(0xC0, 0x8A)
		expected3 = "%c%c%c" % 		(0xE0, 0x80, 0x8A)
		expected4 = "%c%c%c%c" % 	(0xF0, 0x80, 0x80, 0x8A)
		expected5 = "%c%c%c%c%c" % 	(0xF8, 0x80, 0x80, 0x80, 0x8A)
		expected6 = "%c%c%c%c%c%c" %(0xFC, 0x80, 0x80, 0x80, 0x80, 0x8A)
		
		g = OverLongUtf8(None, Static("%c" % 0x0A))
		
		if g.getRawValue() != expected1:
			print "OverLongUtf8 unittest failure 1"
		g.next()
		if g.getRawValue() != expected2:
			print "OverLongUtf8 unittest failure 2"
		g.next()
		if g.getRawValue() != expected3:
			print "OverLongUtf8 unittest failure 3"
		g.next()
		if g.getRawValue() != expected4:
			print "OverLongUtf8 unittest failure 4"
		g.next();
		if g.getRawValue() != expected5:
			print "OverLongUtf8 unittest failure 5"
		g.next()
		if g.getRawValue() != expected6:
			print "OverLongUtf8 unittest failure 6"
		print "Done with OverLongUtf8 unittests"
	unittest = staticmethod(unittest)


def unittest():
	OverLongUtf8.unittest()

# end

