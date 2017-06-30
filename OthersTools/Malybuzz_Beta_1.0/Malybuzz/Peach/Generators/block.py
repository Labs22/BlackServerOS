
'''
Contains implementation of a Block generator and BlockSize generator.

Modified by Jose Miguel Esparza to use in Malybuzz fuzzer

@author: Michael Eddington
@version: $Id: block.py 73 2006-12-11 22:54:54Z meddingt $
'''

#
# Copyright 2007 Jose Miguel Esparza
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
#   Michael Eddington (meddington@gmail.com)

# $Id: block.py 73 2006-12-11 22:54:54Z meddingt $

import sys
from Peach import generator, group
import static

#__all__ = ["Block", "BlockSize", "Block2"]

class Block(generator.Generator):
	'''
	Block is a set of Generators in a specific order that have
	a total size.  The BlockSize generator can be used to output
	the block size someplace.
	
	NOTE: Do not use with DictionaryList generator.  Will cause
	non-intended results :)
	'''
	
	inGetValue = 0
	inRawValue = 1
	_generators = None
	_genCompleted = []
	
	def __init__(self, generators = None):
		'''
		@type	generators: List
		@param	generators: List of generators
		'''
		
		generator.Generator.__init__(self)
		
		if generators == None:
			self._generators = []
		else:
			self._generators = generators
		self._genCompleted = []
	

	def next(self):
		'''
		Method added to provide the next on the sub generators.It finish when all the
		generators have reached the final
		'''
		
		for i in range(len(self._generators)):
			try:
				self._generators[i].next()
			except generator.GeneratorCompleted,e:
				if self._genCompleted.count(i)==0:
					self._genCompleted.append(i)
				if len(self._genCompleted)==len(self._generators):
					raise
				else:
					continue

	
	def getValue(self):
		self.inGetValue = 1
		ret = ''
		
		for i in range(len(self._generators)):
			try:
				ret += self._generators[i].getValue()
			except TypeError, e:
				print "Peach.block.Block: Caught type error, here is identification information"
				print "self._generators[i].getName(): %s" % self._generators[i].getName()
				if self._generators[i].identity() != None:
					print "self._generators[i].identity: (%s) =====" % self._generators[i]
					for p in self._generators[i].identity():
						print p
					print "==============="
				else:
					print "self._generators[i].identity: None!"
				
				raise e
		
		if self._transformer != None:
			return self._transformer.transform(ret)
		
		self.inGetValue = 0
		return ret
	
	def getRawValue(self):
		self.inRawValue = 1
		ret = ''
		
		for i in range(len(self._generators)):
			ret += self._generators[i].getRawValue()
		
		self.inRawValue = 0
		return ret
	
	def getSize(self):
		'''
		Size of generator after all transformations
		
		@rtype: number
		@return: size of data generated
		'''
		return len(self.getValue())
	
	def append(self, generator):
		'''
		Append a generator to end of list.
		
		@type	generator: Generator
		@param	generator: Generator to append
		'''
		self._generators.append(generator)
	def insert(self, pos, generator):
		'''
		Insert generator into list
		
		@type	generator: Generator
		@param	generator: Generator to insert
		'''
		self._generators.insert(pos, generator)
	def remove(self, generator):
		'''
		Remove a generator from list
		
		@type	generator: Generator
		@param	generator: Generator to remove
		'''
		self._generators.remove(generator)
	def clear(self):
		'''
		Clear list of generators.
		'''
		self._generators = []
	def setGenerators(self, generators):
		'''
		Set array of generators.
		
		@type	generators: list
		@param	generators: list of Generator objects
		'''
		self._generators = generators
	
	def unittest():
		s = static.Static('hello world')
		block = Block([s])
		if block.getRawValue() != 'hello world':
			raise Exception('Block::unittest: Failed getRawValue')
		block.remove(s)
		block.insert(0,s)
		block.clear()
		block.append(s)
		block.getSize()
		block.getValue()
	unittest = staticmethod(unittest)

class Block2(Block):
	'''
	A Block that will call next() on each generator.  The BlockSize generator
	can be used to output the block size someplace.
	
	This is a specialized version of Block.  This version will
	call next() on each generator when our next() is called.  This
	was added to make complex sub-blocks that are part of
	GeneratorList's work properly.
	'''
	
	def next(self):
		exit = 0
		for i in self._generators:
			try:
				i.next()
				exit = 0
			except:
				exit = 1
		if exit:
			raise generator.GeneratorCompleted("Block2")

class BlockSize(generator.Generator):
	'''
	Will generate size of Block.
	'''
	
	_inGetRawValue = 0
	_block = None
	_defaultSize = None
	
	def __init__(self, block, defaultSize = 1):
		'''
		@type	block: Block
		@param	block: Block to get size of
		@type	defaultSize: number
		@param	defaultSize: To avoid recursion this is how big we are
		(optional)
		'''
		self.setBlock(block)
		self._defaultSize = defaultSize
	
	def getValue(self):
		'''
		Return data, passed through a transformer if set.
		'''
		out = self.getRawValue()
		#print "block.BlockSize::getValue(): out = %s" % out
		if self._transformer != None and self._inGetRawValue == 0:
			out = self._transformer.transform(out)
		
		#print "block.BlockSize::getValue(): out = %s" % out
		return out
	
	def getRawValue(self):
		'''
		Returns size of block as string.
		
		@rtype: string
		@return: size of specified Block
		'''
		if self._inGetRawValue == 1:
			# Avoid recursion and return a string
			# that is defaultSize in length
			out = ''
			for cnt in range(self._defaultSize):
				out += ' '
			return out
		
		self._inGetRawValue = 1
		out = "%d" % self._block.getSize()
		self._inGetRawValue = 0
		return out
	
	def getBlock(self):
		'''
		Get block object we act on.
		
		@rtype: Block
		@return: current Block
		'''
		return self._block
	def setBlock(self, block):
		'''
		Set block we act on.
		
		@type	block: Block
		@param	block: Block to set.
		'''
		self._block = block
		return self
	
	def unittest():
		block = Block([static.Static('hello world')])
		blockSize = BlockSize(block)
		
		if blockSize.getBlock() != block:
			raise Exception("BlockSize::unittest(): Failed getBlock")
		
		blockSize.setBlock(None)
		if blockSize.getBlock() != None:
			raise Exception("BlockSize::unittest(): Failed setBlock")
		blockSize.setBlock(block)
		
		size = blockSize.getRawValue()
		if int(size) != len('hello world'):
			raise Exception("BlockSize::unittest(): Failed getRawValue")
	unittest = staticmethod(unittest)

def unittest():
	Block.unittest()
	BlockSize.unittest()

# end

