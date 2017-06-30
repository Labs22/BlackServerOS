
'''
Incrementing generators (numerical, etc)

@author: Michael Eddington
@version: $Id: incrementor.py 279 2007-05-08 01:21:58Z meddingt $
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

# $Id: incrementor.py 279 2007-05-08 01:21:58Z meddingt $

import struct
from types import *
from Peach import generator, group
from Peach.generator import *

#__all__ = ['Incrementor', 'PerCallIncrementor', 'PerRoundIncrementor']

class Incrementor(generator.Generator):
	'''
	Increment a value by another value each round.  For example,
	one could set 1 as an initial value with an incrementor of 1.
	'''
	
	_roundCount = 0
	_value = None
	_incrementor = None
	_currentValue = None
	_formatString = None
	_maxValue = None
	_maxIterations = None
	_packString = None
	
	def __init__(self, group = None, value = 1, incrementor = 1, formatString = None,
				 maxValue = None, maxIterations = None, packString = None):
		'''
		@type	group: Group
		@param	group: Group this generator works with
		@type	value: number
		@param	value: Number to increment
		@type	incrementor: number
		@param	incrementor: Increment amount (can be negative), default is 1
		@type	formatString: string
		@param	formatString: Format string for value (optional)
		@type	maxValue: number
		@param	maxValue: Maximum value (optional, default None)
		@type	maxIterations: number
		@param	maxIterations: Maximum number of times to increment
				value (optional, default None)
		@type	packString: string
		@param	packString: Pack format string.  Note that use of this
				option will override formatString. (optional, default None)
		'''
		Generator.__init__(self)
		self._value = value
		self._incrementor = incrementor
		self._formatString = formatString
		self._maxValue = maxValue
		self._maxIterations = maxIterations
		self._packString = packString
		self.setGroup(group)
	
	def next(self):
		self._roundCount += 1
		
		if self._currentValue == None:
			self._currentValue = self._value
		else:
			self._currentValue += self._incrementor
		
		if self._maxValue:
			if self._currentValue > self._maxValue:
				raise generator.GeneratorCompleted('Generators.Incrementor: maxValue')
		if self._maxIterations:
			if self._roundCount > self._maxIterations:
				raise generator.GeneratorCompleted('Generators.Incrementor: maxIterations')
		
	def reset(self):
		self._roundCount = 0
		self._currentValue = None
	
	def getRawValue(self):
		if self._currentValue == None:
			self._currentValue = self._value
		
		ret = None
		
		if self._packString != None:
			ret = struct.pack(self._packString, self._currentValue)
		else:
			if self._formatString == None:
				retType = type(self._currentValue)
				if retType is IntType:
					ret = "%d" % self._currentValue
				elif retType is FloatType:
					ret = "%f" % self._currentValue
				elif retType is LongType:
					ret = "%d" % self._currentValue
			else:
				ret = self._formatString % self._currentValue
		
		return ret
	
	def setValue(self, value):
		'''
		Set value to increment.
		
		@type	value: number
		@param	value: Number to increment
		'''
		self._value = value
		
	def unittest():
		g = group.GroupFixed(5)
		inc = Incrementor(g, 1, 1)
		
		try:
			while g.next():
				print inc.getValue()
		except group.GroupCompleted:
			pass
		
		g = group.GroupFixed(5)
		inc = Incrementor(g, 1, 10, "<<%d>>")
		
		try:
			while g.next():
				print inc.getValue()
		except group.GroupCompleted:
			pass
		
		g = group.GroupFixed(5)
		inc = Incrementor(g, 1, 0.212673, "[[%0.2f]]")
		
		try:
			while g.next():
				print inc.getValue()
		except group.GroupCompleted:
			pass
	unittest = staticmethod(unittest)


class PerCallIncrementor(generator.Generator):
	'''
	Each call to getValue will increment.  Usefull to make a string
	unique accross fuzz.
	'''
	
	_incrementor = None
	
	def __init__(self, group = None, value = 1, incrementor = 1, formatString = None):
		'''
		@type	group: Group
		@param	group: Group this generator works with
		@type	value: number
		@param	value: Number to increment
		@type	incrementor: number
		@param	incrementor: Amount to increment
		@type	formatString: string
		@param	formatString: Format string for value (optional)
		'''
		self._incrementor = Incrementor(group, value, incrementor, formatString)
	
	def next(self):
		raise generator.GeneratorCompleted('PerCallIncrementor')
	
	def reset(self):
		self._incrementor.reset()
	
	def getRawValue(self):
		ret = self._incrementor.getRawValue()
		self._incrementor.next()
		return ret
	
	def setValue(self, value):
		'''
		Set value to increment.
		
		@type	value: number
		@param	value: Number to increment
		'''
		self._incrementor.setValue(value)
	
	def unittest():
		g = group.GroupFixed(5)
		inc = PerCallIncrementor(g, 1, 0.212673, "[[%0.2f]]")
		
		try:
			while g.next():
				print inc.getValue()
				print inc.getValue()
				print inc.getValue()
		except group.GroupCompleted:
			pass
	unittest = staticmethod(unittest)

class PerRoundIncrementor(generator.Generator):
	'''
	Each round we increment.  Has it's uses :)
	'''
	
	_incrementor = None
	
	def __init__(self, value = 1, incrementor = 1, formatString = None):
		'''
		@type	value: number
		@param	value: Number to increment
		@type	incrementor: number
		@param	incrementor: Amount to increment
		@type	formatString: string
		@param	formatString: Format string for value (optional)
		'''
		
		self._incrementor = Incrementor(None, value, incrementor, formatString)
	
	def next(self):
		self._incrementor.next()
		raise generator.GeneratorCompleted('PerCallIncrementor')
	
	def reset(self):
		self._incrementor.reset()
	
	def getRawValue(self):
		return self._incrementor.getRawValue()
	
	def setValue(self, value):
		'''
		Set value to increment.
		
		@type	value: number
		@param	value: Number to increment
		'''
		self._incrementor.setValue(value)
	
	def unittest():
		g = group.GroupFixed(5)
		inc = PerCallIncrementor(g, 1, 0.212673, "[[%0.2f]]")
		
		try:
			while g.next():
				print inc.getValue()
				print inc.getValue()
				print inc.getValue()
		except group.GroupCompleted:
			pass
	unittest = staticmethod(unittest)


def unittest():
	Incrementor.unittest()
	PerCallIncrementor.unittest()
	

# ############################################################################

import inspect, pyclbr

def RunUnit(obj, clsName):
	print "Unittests for: %s..." % clsName,
	cnt = 0
	try:
		while True:
			s = obj.getValue()
			obj.next()
			cnt+=1
			
	except generator.GeneratorCompleted:
		print "%d tests found." % cnt

if __name__ == "__main__":
	print "\n -- Running A Quick Unittest for %s --\n" % __file__
	mod = inspect.getmodulename(__file__)
	for clsName in pyclbr.readmodule(mod):
		cls = globals()[clsName]
		if str(cls).find('__main__') > -1 and hasattr(cls, 'next') and hasattr(cls, 'getValue'):
			try:
				RunUnit(cls(), clsName)
			except TypeError:
				pass

# end
