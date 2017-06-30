
'''
Currently contains W3C XML test suite

@author: Michael Eddington
@version: $Id: xmlstuff.py 279 2007-05-08 01:21:58Z meddingt $
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

# $Id: xmlstuff.py 279 2007-05-08 01:21:58Z meddingt $

import re, struct
from Peach.generator import Generator
from Peach.Generators.dictionary  import *
from Peach.Generators.static  import *

#__all__ = ['XmlParserTests', 'XmlParserTestsInvalid', 'XmlParserTestsValid',
#		   'XmlParserTestsNotWellFormed', 'XmlParserTestsError',
#		   'XmlCreateNodes', 'XmlCreateElements']

class XmlCreateElements(Generator):
	'''
	This generator create XML elements N deep
	'''
	
	_startingDepth = 1
	_increment = 1
	_nodePrefix = Static('PeachFuzz')
	_nodePostfix = None
	_elementAttributs = None
	_currentDepth = 1
	_maxDepth = 1000
	
	def __init__(self, group, startingDepth = None, increment = None,
				 maxDepth = None, nodePrefix = None, nodePostfix = None,
				 elementAttributes = None):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	startingDepth: integer
		@param	startingDepth: How many deep to start at, default 1
		@type	increment: integer
		@param	increment: Incrementor, default 1
		@type	maxDepth: integer
		@param	maxDepth: Max depth, default 1000
		@type	nodePrefix: Generator
		@param	nodePrefix: Node prefix, default is Static('PeachFuzz')
		@type	nodePostfix: Generator
		@param	nodePostfix: Node postfix, default is None
		@type	elementAttributes: Generator
		@param	elementAttributes: Element attributes, default is None
		'''
		self.setGroup(group)
		if startingDepth != None:
			self._startingDepth = startingDepth
		if increment != None:
			self._increment = increment
		if nodePrefix != None:
			self._nodePrefix = nodePrefix
		if nodePostfix != None:
			self._nodePostfix = nodePostfix
		if elementAttributes != None:
			self._elementAttributes = elementAttributes
		if maxDepth != None:
			self._maxDepth = maxDepth
	
	def next(self):
		self._currentDepth += self._increment
		if self._currentDepth > self._maxDepth:
			raise generator.GeneratorCompleted("XmlCreateNodes")
	
	def getRawValue(self):
		ret = ''
		
		postFixs = []
		for i in range(self._currentDepth):
			if self._nodePostfix != None:
				postFixs[i] = self._nodePostfix.getValue()
				if self._elementAttributes != None:
					ret += "<%s%s %s>" % (self._nodePrefix.getValue(), postFixs[i],
										  self._elementAttributes.getValue())
				else:
					ret += "<%s%s>" % (self._nodePrefix.getValue(), postFixs[i])
			else:
				if self._elementAttributes != None:
					ret += "<%s %s>" % (self._nodePrefix.getValue(),
										  self._elementAttributes.getValue())
				else:
					ret += "<%s>" % self._nodePrefix.getValue()
		
		for j in range(self._currentDepth):
			if self._nodePostfix != None:
				ret += "</%s%s>" % (self._nodePrefix.getValue(), postFixs[i-j])
			else:
				ret += "</%s>" % self._nodePrefix.getValue()
		
		return ret
	
	def reset(self):
		self._currentDepth = 1
	
	def unittest():
		expected = '<PeachFuzz1><PeachFuzz2><PeachFuzz3></PeachFuzz3></PeachFuzz2></PeachFuzz1>'
		g = XmlCreateNodes(1, 1)
		g.next()
		g.next()
		g.next()
		if g.getRawValue() != expected:
			print "FAILURE!!! XmlCreateNodes"
		
	unittest = staticmethod(unittest)

class XmlCreateNodes(Generator):
	'''
	This generator create XML nodes N deep
	'''
	
	_startingDepth = 1
	_increment = 1
	_nodePrefix = Static('PeachFuzz')
	_currentDepth = 1
	_maxDepth = 1000
	
	def __init__(self, group, startingDepth, increment, maxDepth, nodePrefix):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	startingDepth: integer
		@param	startingDepth: How many deep to start at, default 1
		@type	increment: integer
		@param	increment: Incrementor, default 1
		@type	maxDepth: integer
		@param	maxDepth: Max depth, default 1000
		@type	nodePrefix: Generator
		@param	nodePrefix: Node prefix, default is Static('PeachFuzz')
		'''
		self.setGroup(group)
		if startingDepth != None:
			self._startingDepth = startingDepth
		if increment != None:
			self._increment = increment
		if nodePrefix != None:
			self._nodePrefix = nodePrefix
		if maxDepth != None:
			self._maxDepth = maxDepth
	
	def next(self):
		self._currentDepth += self._increment
		if self._currentDepth > self._maxDepth:
			raise generator.GeneratorCompleted("XmlCreateNodes")
	
	def getRawValue(self):
		ret = ''
		
		for i in range(self._currentDepth):
			ret += "<%s%d>" % (self._nodePrefix.getValue(), i)
		
		for j in range(self._currentDepth):
			ret += "</%s%d>" % (self._nodePrefix.getValue(), i-j)
		
		return ret
	
	def reset(self):
		self._currentDepth = 1
	
	def unittest():
		expected = '<PeachFuzz1><PeachFuzz2><PeachFuzz3></PeachFuzz3></PeachFuzz2></PeachFuzz1>'
		g = XmlCreateNodes(1, 1)
		g.next()
		g.next()
		g.next()
		if g.getRawValue() != expected:
			print "FAILURE!!! XmlCreateNodes"
		
	unittest = staticmethod(unittest)

class XmlParserTests(Generator):
	'''
	W3C XML Validation Tests.  This includes
	all sets of tests, invalid, non-well formed, valid and error.
	
	NOTE: Test files are in samples/xmltests.zip these are the
	latest test cases from W3C as of 02/23/06 for XML.
	'''
	
	_generatorList = None
	
	def __init__(self, group, testFiles):
		'''
		@type	group: Group
		@param	group: Group this Generator belongs to
		@type	testFiles: string
		@param	testFiles: Location of test files
		'''
		
		self._generatorList = GeneratorList(group,
									[XmlParserTestsInvalid(None, testFiles),
									XmlParserTestsNotWellFormed(None, testFiles),
									XmlParserTestsValid(None, testFiles)])
	
	def getRawValue(self):
		return self._generatorList.getRawValue()
	
	def next(self):
		self._generatorList.next()
	
	def unittest():
		pass
	unittest = staticmethod(unittest)
	
class XmlParserTestsGeneric(Generator):
	'''
	Base class
	'''
	
	_testsFolder = 'xmltests'
	_testsFile = 'invalid.txt'
	_currentValue = None
	_currentTestNum = 1
	_currentFilename = None
	_fdTests = None
	_fd = None
	
	def __init__(self, group, testsFolder, testsFile):
		'''
		@type	group: Group
		@param	group: Group this Generator belongs to
		@type	testsFolder: string
		@param	testsFolder: Location of test files
		@type	testsFile: string
		@param	testsFile: File with listing of test files
		'''
		
		self.setGroup(group)
		if testsFile != None:
			self._testsFile = testsFile
		if testsFolder != None:
			self._testsFolder = testsFolder
		
	
	def next(self):
		if self._fdTests == None:
			self._fdTests = open("%s/%s" % (self._testsFolder, self._testsFile),
								 'rb')
			
		self._currentFilename = "%s/%s" % (self._testsFolder,
										   self._fdTests.readline())
		self._currentFilename = self._currentFilename.strip("\r\n")
		if len(self._currentFilename) <= len(self._testsFolder)+2:
			raise generator.GeneratorCompleted(
				"Peach.Generators.xml.XmlParserTestsInvalid")
		
		if self._fd == None:
			self._fd = open(self._currentFilename, 'rb')
			if self._fd == None:
				raise Exception('Unable to open', self._currentFilename)
		
		self._currentValue = self._fd.read()
		self._fd = None
	
	def getRawValue(self):
		if self._currentValue == None:
			self.next()
		
		return self._currentValue
	
	def reset(self):
		self._fd = None
		self._fdTests = None
		self._currentValue = None
	
	def unittest():
		pass
	unittest = staticmethod(unittest)

class XmlParserTestsInvalid(XmlParserTestsGeneric):
	'''
	W3C XML Validation Tests, invalid set only.
	
	NOTE: Test files are in samples/xmltests.zip these are the
	latest test cases from W3C as of 02/23/06 for XML.
	'''
	
	def __init__(self, group, testsFolder):
		'''
		@type	group: Group
		@param	group: Group this Generator belongs to
		@type	testsFolder: string
		@param	testsFolder: Location of test files
		'''
		self.setGroup(group)
		self._testsFile = 'valid.txt'
		if testsFolder != None:
			self._testsFolder = testsFolder

class XmlParserTestsValid(XmlParserTestsGeneric):
	'''
	W3C XML Validation Tests, valid set only.
	
	NOTE: Test files are in samples/xmltests.zip these are the
	latest test cases from W3C as of 02/23/06 for XML.
	'''
	
	def __init__(self, group, testsFolder):
		'''
		@type	group: Group
		@param	group: Group this Generator belongs to
		@type	testsFolder: string
		@param	testsFolder: Location of test files
		'''
		self.setGroup(group)
		self._testsFile = 'valid.txt'
		if testsFolder != None:
			self._testsFolder = testsFolder

class XmlParserTestsError(XmlParserTestsGeneric):
	'''
	W3C XML Validation Tests, error set only.
	
	NOTE: Test files are in samples/xmltests.zip these are the
	latest test cases from W3C as of 02/23/06 for XML.
	'''
	
	def __init__(self, group, testsFolder):
		'''
		@type	group: Group
		@param	group: Group this Generator belongs to
		@type	testsFolder: string
		@param	testsFolder: Location of test files
		'''
		self.setGroup(group)
		self._testsFile = 'error.txt'
		if testsFolder != None:
			self._testsFolder = testsFolder

class XmlParserTestsNotWellFormed(XmlParserTestsGeneric):
	'''
	W3C XML Validation Tests, Invalid set only.
	
	NOTE: Test files are in samples/xmltests.zip these are the
	latest test cases from W3C as of 02/23/06 for XML.
	'''
	
	def __init__(self, group, testsFolder):
		'''
		@type	group: Group
		@param	group: Group this Generator belongs to
		@type	testsFolder: string
		@param	testsFolder: Location of test files
		'''
		self.setGroup(group)
		self._testsFile = 'nonwf.txt'
		if testsFolder != None:
			self._testsFolder = testsFolder

def unittest():
	XmlParserTestsInvalid.unittest()
	XmlParserTestsValid.unittest()
	XmlParserTestsNotWellFormed.unittest()
	XmlParserTestsError.unittest()

# end
