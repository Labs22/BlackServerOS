
'''
Base generator object implementations.

@author: Michael Eddington
@version: $Id: generator.py 284 2007-05-08 03:26:39Z meddingt $
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

# $Id: generator.py 284 2007-05-08 03:26:39Z meddingt $

import sys
import traceback

class Generator:
	'''
	Generators generate data.  Examples of generators could be a static
	string or integer, a string repeater, etc.  Generators can be "incremented"
	by calling C{next()} to produce the next varient of data.  Generators can
	be fairly complex, comainting sub-generators to build things like packets.
	
	Generators support the interator protocol and can be used as such.
	
	When building a Generator one should keep in mind that the value from a
	generator could be asked for more then once per "round".  Also it is
	recommended that you use the default C{getValue()} implementation and override
	the C{getRawValue()} method instead.
	
	@see: L{SimpleGenerator}
	'''
	
	_group = None
	_transformer = None
	_identity = None	# Stacktrace of were we came from
	_name = None
	
	def __init__(self):
		'''
		Base constructor, please call me!
		'''
		
		# For debugging.  This is slow (0.02 sec), sometimes this init
		# function can get called like 50K times during initialization
		# of a large fuzzing object tree!
		#self._identity = traceback.format_stack()
	
	def identity(self):
		'''
		Who are we and were do we come from?
		'''
		
		return self._identity
	
	def __iter__(self):
		'''
		Return iterator for Generator object.  This is always the
		Generator object itself.
		
		@rtype: Generator
		@return: Returns iterator, this is always self.
		'''
		return self
	
	def next(self):
		'''
		Next value. OVERRIDE
		
		From Python docs on next():
		
		I{The intention of the protocol is that once an iterator's next() method
		raises StopIteration, it will continue to do so on subsequent calls.
		Implementations that do not obey this property are deemed broken. (This
		constraint was added in Python 2.3; in Python 2.2, various iterators are
		broken according to this rule.)}
		
		For Generators, please use the GeneratorCompleted exception instead of
		StopIteration (its a subclass).
		'''
		#sys.stderr("Generator.next: Raising GeneratorCompleted\n")
		raise GeneratorCompleted("Peach.generator.Generator")
	
	def getValue(self):
		'''
		Return data, passed through a transformer if set.
		
		@rtype: string
		@return: Returns generated data
		'''
		
		if self._transformer != None:
			return self._transformer.transform(self.getRawValue())
		
		return self.getRawValue()
	
	def getRawValue(self):
		'''
		Return raw value w/o passing through transformer if set. OVERRIDE
		
		@rtype: string
		@return: Data before transformations
		'''
		
		return None
	
	def getGroup(self):
		'''
		Get group this Generator belongs to.  Groups are used to increment sets
		of Generators.
		
		@rtype: Group
		@return: Returns Group this generator belongs to
		'''
		return self._group
	
	def setGroup(self, group):
		'''
		Set group this Generator belongs to.  This function will automaticly add
		the Generator into the Group.
		
		Groups are used to increment sets
		of Generators.
		
		@type	group: Group
		@param	group: Group this generator belongs to
		'''
		self._group = group
		if self._group != None:
			self._group.addGenerator(self)
	
	def getTransformer(self):
		'''
		Get transformer (if set).  Transformers are used to transform data in
		some way (such as HTML encoding, etc).
		
		@rtype: Transformer
		@return: Current transformer or None
		'''
		return self._transformer
	
	def setTransformer(self, trans):
		'''
		Set trasnformer.  Transformers are used to transform data in some way
		(such as HTML encoding, etc).
		
		@type	trans: Transformer
		@param	trans: Transformer to run data through
		@rtype: Generator
		@return: self
		'''
		self._transformer = trans
		return self	
	
	def reset(self):
		'''
		Called to reset the generator to its initial state. OVERRIDE
		'''
		pass
	
	def getName(self):
		'''
		Get the name of this generator.  Usefull for debugging.
		'''
		return self._name
	
	def setName(self, name):
		'''
		Set the name of this generator.  Usefull for debugging complex
		data generators.  Stacktraces may end up in a generator creation
		statement giving limited feedback on which generator in an array
		might be causing the problem.
		
		@type	name: string
		@param	name: Name of generator
		'''
		self._name = name


class SimpleGenerator(Generator):
	'''
	A simple generator contains another, possibly complex generator statement.
	Usefull when breaking things apart for reuse.
	
	To use, simply create a class that contains a _generator:
	
		class MySimpleGenerator(SimpleGenerator):
			def __init__(self, group = None):
				SimpleGenerator.__init__(self, group)
				self._generator = GeneratorList(None, [
					Static('AAA'),
					Repeater(None, Static('A'), 1, 100)
					])
	
	NOTE: Do not set group on you generators unless they will not be incremented
	by self._generator.next().
	
	@see: L{Generator}
	'''
	
	_generator = None
	
	def __init__(self, group = None):
		'''
		@type	group: Group
		@param	group: Group to use
		'''
		
		Generator.__init__(self)
		self.setGroup(group)
	
	def next(self):
		self._generator.next()
	
	def getRawValue(self):
		return self._generator.getValue()
	
	def reset(self):
		self._generator.reset()


class GeneratorCompleted(StopIteration):
	'''
	Exception indicating that the generator has completed all
	permutations of its data
	'''
	pass


def unittest():
	pass


# end
