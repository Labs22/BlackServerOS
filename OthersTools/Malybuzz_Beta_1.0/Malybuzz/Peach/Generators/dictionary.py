
'''
Contains generators that use a set of data (dictionaries) such as files, lists,
etc.

@author: Michael Eddington
@version: $Id: dictionary.py 279 2007-05-08 01:21:58Z meddingt $
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

# $Id: dictionary.py 279 2007-05-08 01:21:58Z meddingt $

import types, static, sys
from Peach import generator, group
from Peach.generator import *

#__all__ = ['Dictionary', 'GeneratorList', 'GeneratorList2', 'List', 'BinaryList']

class Dictionary(generator.Generator):
	'''
	Iterates through a collection of values stored in a file.
	Possible uses could be to brute force passwords or try a set of
	known bad values.
	'''
	
	_fileName = None
	_fd = None
	_currentValue = None
	
	def __init__(self, group, fileName):
		'''
		@type	group: Group
		@param	group: Group this Generator belongs to
		@type	fileName: string
		@param	fileName: Name of file use
		'''
		Generator.__init__(self)
		self._fileName = fileName
		self.setGroup(group)
	
	def getFilename(self):
		'''
		Get name of file.
		
		@rtype: string
		@return: name of file
		'''
		return self._fileName
	
	def setFilename(self, filename):
		'''
		Set filename.
		
		@type	filename: string
		@param	filename: Filename to use
		'''
		self._fileName = filename
	
	def next(self):
		if self._fd == None:
			self._fd = open(self._fileName, 'rb')
			if self._fd == None:
				raise Exception('Unable to open', self._fileName)
		
		oldValue = self._currentValue
		self._currentValue = self._fd.readline()
		if self._currentValue == None or len(self._currentValue) == 0:
			self._currentValue = oldValue
			raise generator.GeneratorCompleted("Dictionary completed for file [%s]" % self._fileName)
		
		self._currentValue = self._currentValue.rstrip("\r\n")
	
	def getRawValue(self):
		if self._fd == None:
			self._fd = open(self._fileName, 'rb')
			if self._fd == None:
				raise Exception('Unable to open', self._fileName)
		
		if self._currentValue == None:
			self._currentValue = self._fd.readline()
			self._currentValue = self._currentValue.rstrip("\r\n")
		
		return self._currentValue
	
	def reset(self):
		self._fd = None
		self._currentValue = None
	
	def unittest():
		g = group.Group()
		dict = Dictionary(g, 'samples/dict.txt')
		
		try:
			while g.next():
				print dict.getValue()
		except group.GroupCompleted:
			pass
		
		g.reset()
		
		try:
			while g.next():
				print dict.getValue()
		except group.GroupCompleted:
			pass
	unittest = staticmethod(unittest)


class List(generator.Generator):
	'''
	Iterates through a specified list of values.  When the end of the list is
	reached a generator.GeneratorCompleted exceoption is raised.  Last item
	will be returned until reset is called.
	
	Example:
	
		>>> list = List(None, ['1', '2', '3'])
		>>> list.getValue()
		1
		>>> list.next()
		>>> list.getValue()
		2
		>>> list.next()
		>>> list.getValue()
		3
		
	'''
	
	_list = None
	_curPos = 0
	
	def __init__(self, group, list = None):
		'''
		@type	group: Group
		@param	group: Group this Generator belongs to
		@type	list: list
		@param	list: List of values to iterate through
		'''
		Generator.__init__(self)
		self.setGroup(group)
		self._list = list
		self._curPos = 0
		
		if self._list == None:
			self._list = []
	
	def reset(self):
		self._curPos = 0
	
	def next(self):
		self._curPos += 1
		if self._curPos >= len(self._list):
			self._curPos -= 1
			raise generator.GeneratorCompleted("List")
	
	def getRawValue(self):
		return str(self._list[self._curPos])
	
	def getList(self):
		'''
		Get current list of values.
		
		@rtype: list
		@return: Current list of values
		'''
		self._list
	
	def setList(self, list):
		'''
		Set list of values.
		
		@type	list: list
		@param	list: List of values
		'''
		self._list = list
		
		if self._list == None:
			self._list = []
	
	def unittest():
		g = group.Group()
		list = List(g, ['A', 'B', 'C', 'D'])
		
		if list.getValue() != 'A':
			raise Exception("List unittest failed 1")
		g.next()
		if list.getValue() != 'B':
			raise Exception("List unittest failed 2")
		g.next()
		if list.getValue() != 'C':
			raise Exception("List unittest failed 3")
		g.next()
		if list.getValue() != 'D':
			raise Exception("List unittest failed 4")
		
		try:
			g.next()
			raise Exception("List unittest failed 5")
		except group.GroupCompleted:
			pass
		
		try:
			g.next()
			raise Exception("List unittest failed 5")
		except group.GroupCompleted:
			pass
		
		if list.getValue() != 'D':
			raise Exception("List unittest failed 6")
		
		list = List(g, [1, 2, 3, 4, 5])
		
		try:
			while g.next():
				print list.getValue()
		except group.GroupCompleted:
			pass
	unittest = staticmethod(unittest)


class BinaryList(List):
	'''
	Iterates through a specified list of binary values.  When the end
	of the list is reached a generator.GeneratorCompleted exceoption
	is raised.
	'''
	
	_packString = 'b'
	
	def __init__(self, group, list = None, packString = None):
		'''
		@type	group: Group
		@param	group: Group this Generator belongs to
		@type	list: list
		@param	list: List of values to iterate through
		@type	packString: string
		@param	packString: Defaults to 'b'
		'''
		Generator.__init__(self)
		List.__init__(self, group, list)
		self._packString = packString
	
	def getRawValue(self):
		out = self._list[self._curPos]
		if packString:
			return struct.pack(self._packString, out)
		
		return out
		
	def unittest():
		g = group.Group()
		list = BinaryList(g, [0, 1, 2, 3], '>B')
		
		try:
			while g.next():
				print list.getValue()
		except group.GroupCompleted:
			pass	
	unittest = staticmethod(unittest)


class GeneratorList(generator.Generator):
	'''
	Iterates through a specified list of generators.  When the end of the list is
	reached a generator.GeneratorCompleted exceoption is raised.
	
	NOTE: Generators are incremented by this object so DON'T SET A GROUP ON THEM!
	
	NOTE: We only increment to next generator in list when the GeneratorCompleted
	exception has been thrown from current generator.  This allows one todo kewl
	things like have 2 static generators, then a dictionary, then a repeater.
	
	Example:
	
		>>> gen = GeneratorList(None, [
		... 	Static('1'),
		... 	Static('2'),
		... 	Static('3')
		... 	])
		>>> print gen.getValue()
		1
		>>> gen.next()
		>>> print gen.getValue()
		2
		>>> gen.next()
		>>> print gen.getValue()
		3
		>>> try:
		... 	gen.next()	# Will raise GeneraterCompleted exception
		... except:
		... 	pass
		>>> print gen.getValue() # notice we get last value again.
		3
	
	Example:

		>>> gen = GeneratorList(None, [
		... 	Repeater(None, Static('Peach'), 1, 2),
		... 	Static('Hello World')
		... 	])
		>>> print gen.getValue()
		Peach
		>>> gen.next()
		>>> print gen.getValue()
		PeachPeach
		>>> gen.next()
		>>> print gen.getValue()
		Hello World
		>>> try:
		... 	gen.next()	# Will raise GeneraterCompleted exception
		... except:
		... 	pass
		>>> print gen.getValue() # notice we get last value again.
		Hello World
	
	Bad Example, group set on Generator in list:
	
		>>> group = Group()
		>>> gen = GeneratorList(group, [
		... 	Repeater(group, Static('Peach'), 1, 2),
		... 	Static('Hello World')
		... 	])
		>>> print gen.getValue()
		Peach
		>>> group.next()
		>>> print gen.getValue()
		Hello World
		>>> try:
		... 	gen.next()	# Will raise GeneraterCompleted exception
		... except:
		... 	pass
		>>> print gen.getValue() # notice we get last value again.
		Hello World
	
	'''
	
	_list = None
	_curPos = 0
	
	def __init__(self, group, list, name = None):
		'''
		@type	group: Group
		@param	group: Group this Generator belongs to
		@type	list: list
		@param	list: List of Generators to iterate through
		@type	name: string
		@type	name: Name of generator
		'''
		
		Generator.__init__(self)
		
		self.setName(name)
		
		if group != None:
			self.setGroup(group)
		
		self._list = list
		if self._list == None:
			self._list = []
	
	def next(self):
		try:
			self._list[self._curPos].next()
		except generator.GeneratorCompleted:
			#print "Peach.dictionary.GeneratorList.next(): caught GeneratorCompleted"
			self._curPos += 1
			
		if self._curPos >= len(self._list):
			self._curPos -= 1
			#print "Peach.dictionary.GeneratorList.next(): throwing complete exceptions"
			raise generator.GeneratorCompleted("Peach.dictionary.GeneratorList")
	
	def reset(self):
		self._curPos = 0
		
		for i in self._list:
			i.reset()
	
	def getRawValue(self):
		# Use .getValue to make sure we
		# pick up any transformers
		value = self._list[self._curPos].getValue()
		if value is None:
			print "Peach.dictionary.GeneratorList.getRawValue(): getValue() was None"
			print "Peach.dictionary.GeneratorList.getRawValue(): Name is %s" % self._list[self._curPos].getName()
			print "Peach.dictionary.GeneratorList.getRawValue(): Type is %s" % self._list[self._curPos]
		
		return value
	
	def getList(self):
		'''
		Get list of Generators.
		
		@rtype: list
		@return: list of Generators
		'''
		self._list
	
	def setList(self, list):
		'''
		Set list of Generators.
		
		@type	list: list
		@param	list: List of Generators
		'''
		self._list = list
		
		if self._list == None:
			self._list = []
			
	def unittest():
		g = group.Group()
		list = GeneratorList(g, [static.Static('A'), static.Static('B'), static.Static('C')])
		
		try:
			while g.next():
				print list.getValue()
		except group.GroupCompleted:
			pass
	unittest = staticmethod(unittest)


class GeneratorList2(GeneratorList):
	'''
	Iterates through a specified list of generators (different group control).
	When the end of the list is reached a generator.GeneratorCompleted exceoption
	is raised.
	
	This generator differs from GeneratorList by allowing one group to
	drive the rounds, but associating different sub groups to each generator.
	When the master group is incremented the group for the current generator is
	also incremented.  This allows more complex control of how generators
	create data.
	
	NOTE: We only increment to next generator in list when the GeneratorCompleted
	exception has been thrown from current generator.  This allows one todo kewl
	things like have 2 static generators, then a dictionary, then a repeater.
	
	Example:
	
		>>> groupA = Group()
		>>> groupBA = Group()
		>>> groupBB = Group()
		>>> groupB = GroupForeachDo(groupBA, groupBB)
		>>>
		>>> gen = GeneratorList2(None, [groupA,	groupB], [
		... 	Repeater(groupA, Static('A'), 1, 1, 3),
		... 	Block([
		... 		Statics(groupBA, [':', '\\', '/']),
		... 		Repeater(groupBB, Static('B'), 1, 1, 3)
		... 		])
		... 	])
		>>>
		>>> print gen.getValue()
		A
		>>> gen.next()
		>>> gen.getValue()
		AA
		>>> gen.next()
		>>> gen.getValue()
		AAA
		>>> gen.next()
		>>> gen.getValue()
		:B
		>>> gen.next()
		>>> gen.getValue()
		:BB
		>>> gen.next()
		>>> gen.getValue()
		:BBB
		>>> gen.next()
		>>> gen.getValue()
		\B
		>>> gen.next()
		>>> gen.getValue()
		\BB
		>>> gen.next()
		>>> gen.getValue()
		\BBB
		>>> gen.next()
		>>> gen.getValue()
		/B
		>>> gen.next()
		>>> gen.getValue()
		/BB
		>>> gen.next()
		>>> gen.getValue()
		/BBB		
	
	@see: L{GeneratorList}
	'''
	
	_groupList = None
	
	def __init__(self, group, groupList = None, list = None, name = None):
		'''
		@type	group: Group
		@param	group: Group this Generator belongs to
		@type	groupList: list
		@param	groupList: List of Groups to use on generators
		@type	list: list
		@param	list: List of Generators to iterate through
		@type	name: String
		@param	name: [optional] Name for this Generator.  Used for debugging.
		'''
		Generator.__init__(self)
		self.setGroup(group)
		self._list = list
		self._groupList = groupList
		self.setName(name)
		
		if self._list == None:
			self._list = []
		if self._groupList == None:
			self._groupList = []
	
	def next(self):
		try:
			self._groupList[self._curPos].next()
		except group.GroupCompleted:
			#print "GeneratorList2.next(): Next pos [%d]" % self._curPos
			self._curPos += 1
		
		if self._curPos >= len(self._list):
			self._curPos -= 1
			raise generator.GeneratorCompleted("Peach.dictionary.GeneratorList2")
	
	def setGroups(self, list):
		'''
		Set list of Groups.
		
		@type	list: list
		@param	list: List of Groups
		'''
		self._groupList = list
		
		if self._groupList == None:
			self._groupList = []
	
	def reset(self):
		self._curPos = 0
		
		for i in self._list:
			i.reset()
		
		for i in self._groupList:
			i.reset()
	
	def unittest():
		g = group.Group()
		list = GeneratorList2(g, [static.Static('A'), static.Static('B'), static.Static('C')])
		
		try:
			while g.next():
				print list.getValue()
		except group.GroupCompleted:
			pass
	unittest = staticmethod(unittest)


class GeneratorListGroupMaster(GeneratorList2):
	'''
	Provides a mechanism to create in effect a group of GeneratorList2's that
	will progress and increment together drivin by the master of the group.  This
	Generator is the Group Master generator and controls the slaves of the
	group.
	
	This generator comes in handy when you have two bits of data that are
	logically linked but in separate places.  An example would be a length of
	data being generated.  Both values are parameters and generated separaetly
	but a test calls for performing different length tests against different
	data being generated (zero length data and 100 bytes of data say) which
	would be a subset of the noramally generated data.
	
	Example:
	
		>>> groupNormalBlock = Group()
		>>> groupForeachBlock = Group()
		>>> groupDoLength = Group()
		>>> groupForeachBlockDoLength = GroupForeachDo(groupForeachBlock, groupDoLength)
		>>> 
		>>> genBlock = GeneratorListGroupMaster(None, [
		... 	groupNormalBlock,
		... 	groupForeachBlockDoLength
		... 	], [
		... 	
		... 	# Our normal tests
		... 	GeneratorList(groupNormalBlock, [
		... 		Static('A'),
		... 		Static('BB'),
		... 		]),
		... 	
		... 	# For each of these do all the length tests
		... 	GeneratorList(groupForeachBlock, [
		... 		Static(''),
		... 		Static('PEACH' * 10),
		... 		]),
		... 	])
		>>>
		>>> genLength = GeneratorListGroupSlave([
		...		None,
		... 	None,
		... 	], [
		... 	# generated value for the normal block tests
		... 	BlockSize(genBlock),
		...
		... 	# actual length tests
		... 	GeneratorList(groupDoLength, [
		... 		NumberVariance(None, BlockSize(genBlock), 20),
		... 		BadNumbers(),
		... 		])
		... 	], genBlock)
		>>>
		>>> print genBlock.getValue()
		A
		>>> print genLength.getValue()
		1
		>>> genBlock.next()
		>>> print genBlock.getValue()
		BB
		>>> print genLength.getValue()
		2
		>>> genBlock.next()
		>>> print genBlock.getValue()
		
		>>> print genLength.getValue()
		-20
		
	
	'''
	
	_slaves = []
	_completed = False
	
	def __init__(self, group, groupList, list, slaves = None, name = None):
		'''
		@type	group: Group
		@param	group: Group this Generator belongs to
		@type	groupList: list
		@param	groupList: List of Groups to use on generators
		@type	list: list
		@param	list: List of Generators to iterate through
		@type	name: String
		@param	name: [optional] Name for this Generator.  Used for debugging.
		'''
		GeneratorList2.__init__(self, group, groupList, list, name)
		
		if slaves != None:
			self._slaves = slaves
		else:
			self._slaves = []
	
	def next(self):
		
		if self._completed:
			raise generator.GeneratorCompleted("Peach.dictionary.GeneratorListGroupMaster")
		
		moveNext = True
		
		# next our current generator
		try:
			self._groupList[self._curPos].next()
			moveNext = False
		except group.GroupCompleted:
			pass
		
		# next the generator for each of our slaves
		for slave in self._slaves:
			try:
				slave.slaveNext()
				moveNext = False
			except group.GroupCompleted:
				pass
		
		if moveNext:
			print "GeneratorListGroupMaster.next(): Next pos [%d]" % self._curPos
		
			if (self._curPos+1) >= len(self._list):
				self._completed = True
				
				# Let the slaves know we are done
				for slave in self._slaves:
					slave.slaveCompleted()
				
				raise generator.GeneratorCompleted("Peach.dictionary.GeneratorListGroupMaster")
			
			# Move us and everyone else to next position
			self._curPos += 1
			for slave in self._slaves:
				slave.slaveNextPosition()
	
	def reset(self):
		self._completed = False
		self._curPos = 0
		
		for i in self._list:
			i.reset()
		
		for i in self._groupList:
			i.reset()
		
		for slave in self._slaves:
			slave.reset()

	def addSlave(self, slave):
		self._slaves.append(slave)


class GeneratorListGroupSlave(GeneratorList2):
	'''
	Provides a mechanism to create in effect a group of GeneratorList2's that
	will progress and increment together drivin by the master of the group.  This
	Generator is the slave of ghr group and is controlled by the master.  More
	then one slave can be part of the group.
	
	This generator comes in handy when you have two bits of data that are
	logically linked but in separate places.  An example would be a length of
	data being generated.  Both values are parameters and generated separaetly
	but a test calls for performing different length tests against different
	data being generated (zero length data and 100 bytes of data say) which
	would be a subset of the noramally generated data.
	
	Example:
	
		>>> groupNormalBlock = Group()
		>>> groupForeachBlock = Group()
		>>> groupDoLength = Group()
		>>> groupForeachBlockDoLength = GroupForeachDo(groupForeachBlock, groupDoLength)
		>>> 
		>>> genBlock = GeneratorListGroupMaster(None, [
		... 	groupNormalBlock,
		... 	groupForeachBlockDoLength
		... 	], [
		... 	
		... 	# Our normal tests
		... 	GeneratorList(groupNormalBlock, [
		... 		Static('A'),
		... 		Static('BB'),
		... 		]),
		... 	
		... 	# For each of these do all the length tests
		... 	GeneratorList(groupForeachBlock, [
		... 		Static(''),
		... 		Static('PEACH' * 10),
		... 		]),
		... 	])
		>>>
		>>> genLength = GeneratorListGroupSlave([
		...		None,
		... 	None,
		... 	], [
		... 	# generated value for the normal block tests
		... 	BlockSize(genBlock),
		...
		... 	# actual length tests
		... 	GeneratorList(groupDoLength, [
		... 		NumberVariance(None, BlockSize(genBlock), 20),
		... 		BadNumbers(),
		... 		])
		... 	], genBlock)
		>>>
		>>> print genBlock.getValue()
		A
		>>> print genLength.getValue()
		1
		>>> genBlock.next()
		>>> print genBlock.getValue()
		BB
		>>> print genLength.getValue()
		2
		>>> genBlock.next()
		>>> print genBlock.getValue()
		
		>>> print genLength.getValue()
		-20
		
	'''
	
	_master = None
	_completed = False
	
	def __init__(self, groupList = [], list = [], master = None, name = None):
		'''
		@type	group: Group
		@param	group: Group this Generator belongs to
		@type	groupList: list
		@param	groupList: List of Groups to use on generators
		@type	list: list
		@param	list: List of Generators to iterate through
		@type	master: GeneratorListGroupMaster
		@param	master: The master for this groupping.  Will register self with master
		@type	name: String
		@param	name: [optional] Name for this Generator.  Used for debugging.
		'''
		GeneratorList2.__init__(self, None, groupList, list, name)
		self._name = name
		
		if master != None:
			master.addSlave(self)
	
	def next(self):
		if self._completed:
			raise generator.GeneratorCompleted("Peach.dictionary.GeneratorListGroupSlave")
	
	def slaveNext(self):
		if self._groupList[self._curPos] != None:
			self._groupList[self._curPos].next()
		else:
			raise group.GroupCompleted("Peach.dictionary.GeneratorListGroupSlave")
	
	
	def slaveNextPosition(self):
		print self._name, "slaveNextPosition"
		self._curPos += 1
		
		if self._curPos >= len(self._list):
			print self._name
			raise Exception("%s Ran off end of generator array!!: %d of %d" % (self._name, self._curPos, len(self._list)))
	
	def slaveCompleted(self):
		self._completed = True
	
	def reset(self):
		self._completed = False
		self._curPos = 0
		
		for i in self._list:
			i.reset()
		
		for i in self._groupList:
			if i != None:
				i.reset()
	

def unittest():
	Dictionary.unittest()
	List.unittest()
	BinaryList.unittest()
	GeneratorList.unittest()

if __name__ == "__main__":
	unittest()

# end
