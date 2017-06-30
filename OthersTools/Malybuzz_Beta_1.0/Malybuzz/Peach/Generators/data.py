
'''
Common data generators.  Includes common bad strings, numbers, etc.

Modified by Jose Miguel Esparza to use in Malybuzz fuzzer

@author: Michael Eddington
@version: $Id: data.py 279 2007-05-08 01:21:58Z meddingt $
'''

#
# Copyright 2007 Jose Miguel Esparza
# Copyright (c) 2006-2007 Michael Eddington
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

# $Id: data.py 279 2007-05-08 01:21:58Z meddingt $

import static
import sys
import copy
import struct

from Peach.generator				import *
from Peach.Generators.dictionary	import *
from Peach.Generators.static		import *
from Peach.Generators.data			import *
from Peach.Generators.repeater		import *
from Peach.Generators.block			import *
from Peach.group					import *
import Peach.Transformers.type

#__all__ = ['BadString', 'BadDate', 'BadNumbers', 'BadUnsignedNumbers',
#		   'BadUnsignedNumbers16']

class BadString(SimpleGenerator):
	'''
	Generates variouse string tests.
	
	Examples of data generated:
	
		- Variations on format strings using '%n'
		- Long string
		- Empty string
		- Extended ASCII
		- Common bad ASCII (' " < >)
		- All numbers
		- All letters
		- All spaces
		- etc.
		
	'''
		
	_strings = [
		'Peach',
		'abcdefghijklmnopqrstuvwxyz',
		'ABCDEFGHIJKLMNOPQRSTUVWXYZ',
		'0123456789',
		' ',		'',
		'\n',		'\r',	'\r\n',
		'\t',		'10',
		'|',		'\\',
		'0.0',		'1.0',
		'0.1',		'1.1.1',
		'-2,147,483,648',
		'-2,147,483,649',
		'2,147,483,647',
		'2,147,483,649',
		'-2147483648',
		'-2147483649',
		'2147483647',
		'2147483649',
		'-129',
		'129',
		'255',
		'256',
		'-32769',
		'-32,769',
		'32767',
		'32769',
		'4,294,967,295',
		'4294967299',
		'-9,223,372,036,854,775,809',
		'-9223372036854775809',
		'9,223,372,036,854,775,809',
		'9223372036854775809',
		'18,446,744,073,709,551,615',
		'18,446,744,073,709,551,619',
		'18446744073709551619',
		'2.305843009213693952',
		'200000000000000000000.5',
		'200000000000000000000000000000000000000000000.5'
		'0xFF',
		'0xFFFF',
		'0xFFFFFF',
		'0xFFFFFFFFFFFFFFFFFFFF',
		'Yes',
		'No',
		'%n',
		'%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n%n',
		'%x',
		'%x%x%x%x%x%x%x%x',
		'%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x',
		'%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x',
		'%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x',
		"""<>"/\'""",
		"""~`!@#$%^&*()_+=-{}|\][:"';<>?/.,""",
		'\\"',
		"\\'",
		"%",
		"a%",
		"%a",
		"COM1",
		"COM2",
		"AUX",
		"COM1:",
		"COM2:",
		"AUX:",
		"\\\\peach\foo\foo.txt",
		"\\\\\\",
		"..\\..\\..\\..\\..\\..\\..\\..\\",
		"../../../../../",
		"../",
		"/../../../../../../"
		"/../../..",
		"\\..\\..\\..\\..\\..\\",
		chr(0),		chr(1),		chr(2),		chr(3),
		chr(4),		chr(5),		chr(6),		chr(7),
		chr(8),		chr(9),		chr(10),		chr(11),
		chr(12),		chr(13),		chr(14),		chr(15),
		chr(16),		chr(17),		chr(18),		chr(19),
		chr(20),		chr(21),		chr(22),		chr(23),
		chr(24),		chr(25),		chr(26),		chr(27),
		chr(28),		chr(29),		chr(30),		chr(31),
		chr(32),		chr(33),		chr(34),		chr(35),
		chr(36),		chr(37),		chr(38),		chr(39),
		chr(40),		chr(41),		chr(42),		chr(43),
		chr(44),		chr(45),		chr(46),		chr(47),
		chr(48),		chr(49),		chr(50),		chr(51),
		chr(52),		chr(53),		chr(54),		chr(55),
		chr(56),		chr(57),		chr(58),		chr(59),
		chr(60),		chr(61),		chr(62),		chr(63),
		chr(64),		chr(65),		chr(66),		chr(67),
		chr(68),		chr(69),		chr(70),		chr(71),
		chr(72),		chr(73),		chr(74),		chr(75),
		chr(76),		chr(77),		chr(78),		chr(79),
		chr(80),		chr(81),		chr(82),		chr(83),
		chr(84),		chr(85),		chr(86),		chr(87),
		chr(88),		chr(89),		chr(90),		chr(91),
		chr(92),		chr(93),		chr(94),		chr(95),
		chr(96),		chr(97),		chr(98),		chr(99),
		chr(100),		chr(101),		chr(102),		chr(103),
		chr(104),		chr(105),		chr(106),		chr(107),
		chr(108),		chr(109),		chr(110),		chr(111),
		chr(112),		chr(113),		chr(114),		chr(115),
		chr(116),		chr(117),		chr(118),		chr(119),
		chr(120),		chr(121),		chr(122),		chr(123),
		chr(124),		chr(125),		chr(126),		chr(127),
		chr(128).decode('latin1'), 		chr(129).decode('latin1'), 		chr(130).decode('latin1'), 		chr(131).decode('latin1'),
		chr(132).decode('latin1'),		chr(133).decode('latin1'),		chr(134).decode('latin1'),		chr(135).decode('latin1'),
		chr(136).decode('latin1'),		chr(137).decode('latin1'),		chr(138).decode('latin1'),		chr(139).decode('latin1'),
		chr(140).decode('latin1'),		chr(141).decode('latin1'),		chr(142).decode('latin1'),		chr(143).decode('latin1'),
		chr(144).decode('latin1'),		chr(145).decode('latin1'),		chr(146).decode('latin1'),		chr(147).decode('latin1'),
		chr(148).decode('latin1'),		chr(149).decode('latin1'),		chr(150).decode('latin1'),		chr(151).decode('latin1'),
		chr(152).decode('latin1'),		chr(153).decode('latin1'),		chr(154).decode('latin1'),		chr(155).decode('latin1'),
		chr(156).decode('latin1'),		chr(157).decode('latin1'),		chr(158).decode('latin1'),		chr(159).decode('latin1'),
		chr(160).decode('latin1'),		chr(161).decode('latin1'),		chr(162).decode('latin1'),		chr(163).decode('latin1'),
		chr(164).decode('latin1'),		chr(165).decode('latin1'),		chr(166).decode('latin1'),		chr(167).decode('latin1'),
		chr(168).decode('latin1'),		chr(169).decode('latin1'),		chr(170).decode('latin1'),		chr(171).decode('latin1'),
		chr(172).decode('latin1'),		chr(173).decode('latin1'),		chr(174).decode('latin1'),		chr(175).decode('latin1'),
		chr(176).decode('latin1'),		chr(177).decode('latin1'),		chr(178).decode('latin1'),		chr(179).decode('latin1'),
		chr(180).decode('latin1'),		chr(181).decode('latin1'),		chr(182).decode('latin1'),		chr(183).decode('latin1'),
		chr(184).decode('latin1'),		chr(185).decode('latin1'),		chr(186).decode('latin1'),		chr(187).decode('latin1'),
		chr(188).decode('latin1'),		chr(189).decode('latin1'),		chr(190).decode('latin1'),		chr(191).decode('latin1'),
		chr(192).decode('latin1'),		chr(193).decode('latin1'),		chr(194).decode('latin1'),		chr(195).decode('latin1'),
		chr(196).decode('latin1'),		chr(197).decode('latin1'),		chr(198).decode('latin1'),		chr(199).decode('latin1'),
		chr(200).decode('latin1'),		chr(201).decode('latin1'),		chr(202).decode('latin1'),		chr(203).decode('latin1'),
		chr(204).decode('latin1'),		chr(205).decode('latin1'),		chr(206).decode('latin1'),		chr(207).decode('latin1'),
		chr(208).decode('latin1'),		chr(209).decode('latin1'),		chr(210).decode('latin1'),		chr(211).decode('latin1'),
		chr(212).decode('latin1'),		chr(213).decode('latin1'),		chr(214).decode('latin1'),		chr(215).decode('latin1'),
		chr(216).decode('latin1'),		chr(217).decode('latin1'),		chr(218).decode('latin1'),		chr(219).decode('latin1'),
		chr(220).decode('latin1'),		chr(221).decode('latin1'),		chr(222).decode('latin1'),		chr(223).decode('latin1'),
		chr(224).decode('latin1'),		chr(225).decode('latin1'),		chr(226).decode('latin1'),		chr(227).decode('latin1'),
		chr(228).decode('latin1'),		chr(229).decode('latin1'),		chr(230).decode('latin1'),		chr(231).decode('latin1'),
		chr(232).decode('latin1'),		chr(233).decode('latin1'),		chr(234).decode('latin1'),		chr(235).decode('latin1'),
		chr(236).decode('latin1'),		chr(237).decode('latin1'),		chr(238).decode('latin1'),		chr(239).decode('latin1'),
		chr(240).decode('latin1'),		chr(241).decode('latin1'),		chr(242).decode('latin1'),		chr(243).decode('latin1'),
		chr(244).decode('latin1'),		chr(245).decode('latin1'),		chr(246).decode('latin1'),		chr(247).decode('latin1'),
		chr(248).decode('latin1'),		chr(249).decode('latin1'),		chr(250).decode('latin1'),		chr(251).decode('latin1'),
		chr(252).decode('latin1'),		chr(253).decode('latin1'),		chr(254).decode('latin1'),		chr(255).decode('latin1')
		]
	
	def __init__(self, group = None):
		SimpleGenerator.__init__(self, group)
		'''
		self._generator = GeneratorList(None, [
			List(None, self._strings),
			Repeater(None, Static("A"), 10, 200),
			Repeater(None, Static("A"), 127, 100),
			Repeater(None, Static("A"), 1024, 10),
			Repeater(None, Static("\x41\0"), 10, 200),
			Repeater(None, Static("\x41\0"), 127, 100),
			Repeater(None, Static("\x41\0"), 1024, 10),
			])
		'''
		self._generator = Statics(None, self._strings)
	
	def unittest():
		g = BadString(None)
		
		if g.getValue() != 'abcdefghijklmnopqrstuvwxyz':
			raise Exception("BadString unittest failed #1")
		g.next()
		
		if g.getValue() != 'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
			raise Exception("BadString unittest failed #2")
		
		print "BadString okay\n"
	unittest = staticmethod(unittest)


class BadTime(SimpleGenerator):
	'''
	Test cases for HTTP-Date type
	'''
	def __init__(self, group = None):
		SimpleGenerator.__init__(self, group)
		
		groupSeq = [Group(), Group(), Group()]
		
		self._generator = GeneratorList2(None, [
			groupSeq[0],
			groupSeq[1],
			groupSeq[2],
			],[
			
			Block([
				GeneratorList(groupSeq[0], [
					Static('08'),
					BadString(),
					BadNumbers(),
					Static('08')
					]),
				Static(':01:01')
				]),
			
			Block([
				Static('08:'),
				GeneratorList(groupSeq[1], [
					Static('08'),
					BadString(),
					BadNumbers(),
					Static('08')
					]),
				Static(':01')
				]),
			
			Block([
				Static('08:01'),
				GeneratorList(groupSeq[2], [
					Static('08'),
					BadString(),
					BadNumbers(),
					Static('08')
					])
				])
			])


class BadDate(SimpleGenerator):
	'''
	[BETA] Generates alot of funky date's.  This Generator is still missing
	alot of test cases.
	
		- Invalid month, year, day
		- Mixed up stuff
		- Crazy odd date formats
	'''
	
	_strings = [
		'1/1/1',
		'0/0/0',
		'0-0-0',
		'00-00-00',
		'-1/-1/-1',
		'XX/XX/XX',
		'-1-1-1-1-1-1-1-1-1-1-1-',
		'Jun 39th 1999',
		'June -1th 1999',
		
		# ANSI Date formats
		'2000',
		'1997',
		'0000',
		'0001',
		'9999',
		
		'0000-00',
		'0000-01',
		'0000-99',
		'0000-13',
		'0001-00',
		'0001-01',
		'0001-99',
		'0001-13',
		'9999-00',
		'9999-01',
		'9999-99',
		'9999-13',
		
		'0000-00-00',
		'0000-01-00',
		'0000-99-00',
		'0000-13-00',
		'0001-00-00',
		'0001-01-00',
		'0001-99-00',
		'0001-13-00',
		'9999-00-00',
		'9999-01-00',
		'9999-99-00',
		'9999-13-00',
		'0000-00-01',
		'0000-01-01',
		'0000-99-01',
		'0000-13-01',
		'0001-00-01',
		'0001-01-01',
		'0001-99-01',
		'0001-13-01',
		'9999-00-01',
		'9999-01-01',
		'9999-99-01',
		'9999-13-01',
		'0000-00-99',
		'0000-01-99',
		'0000-99-99',
		'0000-13-99',
		'0001-00-99',
		'0001-01-99',
		'0001-99-99',
		'0001-13-99',
		'9999-00-99',
		'9999-01-99',
		'9999-99-99',
		'9999-13-99',
		]
	
	def __init__(self, group = None):
		SimpleGenerator.__init__(self, group)
		self._generator = List(None, self._strings)
	
	def unittest():
		g = BadDate(None)
		
		if g.getValue() != '1/1/1':
			raise Exception("BadDate unittest failed #1")
		g.next()
		
		if g.getValue() != '0/0/0':
			raise Exception("BadDate unittest failed #2")
		
		print "BadDate okay\n"
	unittest = staticmethod(unittest)


class NumberLimiter(Generator):
	'''
	Wraps another generator that produces numbers and limits the produced
	number to a range.  If the number produced is outside of this range
	we will skip it.
	'''
	
	def __init__(self, group, generator, min, max):
		'''
		Min and max can be used to limit the produced numbers.
		
		@type	group: Group
		@param	group: Group to use
		@type	generator: Generator
		@param	generator: Generatored number to limit
		@param	min: Number
		@type	min: Minimum allowed number
		@param	max: Number
		@type	max: Maximum allowed number
		'''
		
		Generator.__init__(self)
		self.setGroup(group)
		
		self._generator = generator
		self._min = min
		self._max = max
		self._lastGood = str(self._min)
	
	def _checkAndSkip(self):
		val = int(self._generator.getValue())
		
		while val < self._min or val > self._max:
			#print "_checkAndSkip: Skipping: ", val
			self._generator.next()
			val = int(self._generator.getValue())
	
	def next(self):
		self._generator.next()
		self._checkAndSkip()
	
	def reset(self):
		self._generator.reset()
		self._checkAndSkip()
	
	def getRawValue(self):
		val = int(self._generator.getValue())
		
		if val < self._min or val > self._max:
			return self._lastGood
		
		self._lastGood = str(val)
		return self._lastGood


class StringVariance(Generator):
	'''
	Generate a range of string sizes from len(str) - variance to len(str) + variance.
	'''
	
	_number = None
	_variance = None
	_min = None
	_max = None
	_current = None
	
	def __init__(self, group, string, variance, min = None, max = None):
		'''
		Min and max can be used to limit the produced numbers.
		
		@type	group: Group
		@param	group: Group to use
		@type	string: String or Generator
		@param	string: String to vary length of
		@type	variance: + and - change to give length range
		@param	min: Number
		@type	min: Minimum allowed length
		@param	max: Number
		@type	max: Maximum allowed length
		'''
		
		Generator.__init__(self)
		self.setGroup(group)
		
		# Can't create negative strings :)
		if variance > len(string) and min == None:
			min = 0
		
		if min != None and min < 0:
			raise Exception("Negative string min length???")
		
		self._string = string
		self._stringLength = len(string)
		self._numberVariance = NumberVariance(None, len(string), variance, min, max)
		self._minAllowed = min
		self._maxAllowed = max
		self._current = string[:self._numberVariance.getValue()]
	
	def getRawValue(self):
		return self._current
	
	def next(self):
		self._numberVariance.next()
		
		# make current value
		length = self._numberVariance.getValue()
		if length < self._stringLength:
			self._current = self._string[:length]
		else:
			multiplier = (length/self._stringLength) + 1
			#print "Multiplier: ",multiplier
			#print "Target length: ",length
			val = self._string * multiplier
			self._current = val[:length]
	
	def reset(self):
		self._numberVariance.reset()


class NumberVariance(Generator):
	'''
	Generate a range of numbers from (number - variance) to (number + variance).
	
	Example:
	
		>>> gen = NumberVariance(None, 10, 5)
		>>> print gen.getValue()
		5
		>>> gen.next()
		>>> gen.getValue()
		6
		>>> gen.next()
		>>> gen.getValue()
		7
		>>> gen.next()
		>>> gen.getValue()
		8
		>>> gen.next()
		>>> gen.getValue()
		9
		>>> gen.next()
		>>> gen.getValue()
		10
		>>> gen.next()
		>>> gen.getValue()
		11
		>>> gen.next()
		>>> gen.getValue()
		12
		>>> gen.next()
		>>> gen.getValue()
		13
		>>> gen.next()
		>>> gen.getValue()
		14
		>>> gen.next()
		>>> gen.getValue()
		15
	'''
	
	_number = None
	_variance = None
	_min = None
	_max = None
	_current = None
	
	def __init__(self, group, number, variance, min = None, max = None):
		'''
		Min and max can be used to limit the produced numbers.
		
		@type	group: Group
		@param	group: Group to use
		@type	number: Number or Generator
		@param	number: Number to change
		@type	variance: + and - change to give range
		@param	min: Number
		@type	min: Minimum allowed number
		@param	max: Number
		@type	max: Maximum allowed number
		'''
		
		Generator.__init__(self)
		self.setGroup(group)
		
		self._number = number
		self._variance = variance
		self._min = None
		self._max = None
		self._current = None
		self._minAllowed = min
		self._maxAllowed = max
	
	def next(self):
		if self._current == None:
			self.getRawValue()
		
		self._current += 1
		if self._current > self._max:
			raise GeneratorCompleted("NumberVariance")
	
	def getRawValue(self):
		if self._current == None:
			# firt time here figure out stuff
			if str(type(self._number)) != "<type 'int'>" and str(type(self._number)) != "<type 'long'>":
				self._number = long(self._number.getValue())
			
			if (self._number - self._variance) < (self._number + self._variance):
				self._min = self._number - self._variance
				self._max = self._number + self._variance
			else:
				self._max = self._number - self._variance
				self._min = self._number + self._variance
			
			# Verify min and max
			if self._minAllowed != None and self._min < self._minAllowed:
				self._min = self._minAllowed
			
			if self._maxAllowed != None and self._max > self._maxAllowed:
				self._max = self._maxAllowed
			
			self._current = self._min
		
		return self._current
	
	def reset(self):
		self._min = None
		self._max = None
		self._current = None
	
	def unittest():
		gen = NumberVariance(None, 10, 5)
		for cnt in range(5, 15):
			if cnt != gen.getValue():
				raise "NumberVariance broken %d != %d" % (cnt, gen.getValue())
		
		print "NumberVariance OK!"
		
	unittest = staticmethod(unittest)


class NumbersVariance(SimpleGenerator):
	'''
	Performs a L{NumberVariance} on a list of numbers.  This is a specialized
	version of L{NumberVariance} that takes an array of numbers to perform a
	variance on instead of just a single number.
	
	Example:
	
		>>> gen = NumbersVariance(None, [1,10], 1)
		>>> gen.getValue()
		0
		>>> gen.next()
		>>> gen.getValue()
		1
		>>> gen.next()
		>>> gen.getValue()
		2
		>>> gen.next()
		>>> gen.getValue()
		9
		>>> gen.next()
		>>> gen.getValue()
		10
		>>> gen.next()
		>>> gen.getValue()
		11
		
	
	@see: L{NumberVariance}
	
	'''

	def __init__(self, group, numbers, variance):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	numbers: Array of numbers
		@param	numbers: Numbers to change
		@type	variance: + and - change to give range
		'''
		
		Generator.__init__(self)
		self.setGroup(group)
		
		gens = []
		
		for n in numbers:
			gens.append(NumberVariance(None, n, variance))
		
		self._generator = GeneratorList(group, gens)
		
	def unittest():
		raise "NumbersVariance needs a unittest"		
	unittest = staticmethod(unittest)



class BadNumbersAsString(SimpleGenerator):
	'''
	[DEPRICATED] Use L{BadNumbers} instead.
	
	@see: Use L{BadNumbers} instead.
	@depricated
	@undocumented
	'''
	
	_ints = [
		'0',
		'1',
		'1.18446744073709551615',
		'1.340282366920938463463374607431768211457',
		'-1',
		'-127',
		'-128',
		'-128.681',
		'-129',
		'127',
		'128',
		'128.681',
		'129',
		'-255',
		'-256',
		'-257',
		'255',
		'256',
		'257',
		'-32767',
		'-32768',
		'-32769',
		'32767',
		'32768',
		'32769',
		'-65535',
		'-65536',
		'-65537',
		'65535',
		'65536',
		'65537',
		'-4294967295',
		'-4294967296',
		'-4294967297',
		'4294967295',
		'4294967296',
		'4294967297',
		'-2147483647',
		'-2147483648',
		'-2147483649',
		'2147483647',
		'2147483648',
		'2147483649',
		'-9223372036854775807',
		'-9223372036854775808',
		'-9223372036854775809',
		'9223372036854775807',
		'9223372036854775808',
		'9223372036854775809',
		'-18446744073709551615',
		'-18446744073709551616',
		'-18446744073709551617',
		'18446744073709551615',
		'18446744073709551616',
		'18446744073709551617',
		'-340282366920938463463374607431768211455',
		'-340282366920938463463374607431768211456',
		'-340282366920938463463374607431768211457',
		'340282366920938463463374607431768211455',
		'340282366920938463463374607431768211456',
		'340282366920938463463374607431768211457',
		'0x100'
		'0x1000'
		'0x3fffffff'
		'0x7ffffffe'
		'0x7fffffff'
		'0x80000000'
		'0xfffffffe'
		'0xffffffff'
		'0x10000'
		'0x100000'	
		]

	def __init__(self, group = None):
		SimpleGenerator.__init__(self, group)
		self._generator = Statics(None, self._ints)

	
#	def __init__(self, group = None):
#		SimpleGenerator.__init__(self, group)
#		self._generator = WithDefault(None, 10, NumbersVariance(None, self._ints, 50))
	
	def getRawValue(self):
		try:
			val = self._generator.getValue()
		
		except OverflowError:
			# Wow, that sucks!
			print "BadNumbersAsString(): OverflowError spot 1!"
			return str(0)
		
		return str(val)


class BadNumbers16(SimpleGenerator):
	'''
	Generate numbers that may trigger integer overflows for
	both signed and unsigned numbers.  Under the hood this generator
	performs a L{NumbersVariance} on the boundry numbers for:
	
		- int8 (0, -128, 127)
		- unsigned int8 (255)
		- int16 (-32768, 32767)
		- unsigned int16 (65535)
	
	@see: L{BadNumbers}, L{NumbersVariance}, L{BadUnsignedNumbers}, L{BadPositiveNumbers}
	'''
	
	_ints = [
		0,
		-128,	# signed 8
		127,
		255,	# unsigned 8
		-32768,	# signed 16
		32767,
		65535	# unsigned 16
		]
	
	def __init__(self, group = None):
		SimpleGenerator.__init__(self, group)
		self._generator = WithDefault(None, 10, NumbersVariance(None, self._ints, 50))
	
	def getRawValue(self):
		try:
			val = self._generator.getValue()
		
		except OverflowError:
			# Wow, that sucks!
			print "BadNumbersAsString16(): OverflowError spot 1!"
			return str(0)
		
		return str(val)


class BadNumbers(BadNumbersAsString):
	'''
	Generate numbers that may trigger integer overflows for
	both signed and unsigned numbers.  Under the hood this generator
	performs a L{NumbersVariance} on the boundry numbers for:
	
		- int8 (0, -128, 127)
		- unsigned int8 (255)
		- int16 (-32768, 32767)
		- unsigned int16 (65535)
		- int32 (-2147483648, 2147483647)
		- unsigned int32 (4294967295)
		- int64 (-9223372036854775808, 9223372036854775807)
		- unsigned int64 (18446744073709551615)
	
	@see: L{BadNumbers16}, L{NumbersVariance}, L{BadUnsignedNumbers}, L{BadPositiveNumbers}
	'''
	pass


class BadPositiveNumbers(SimpleGenerator):
	'''
	Generate positive numbers that may trigger integer overflows for
	both signed and unsigned numbers.  Under the hood this generator
	performs a L{NumbersVariance} on the boundry numbers for:
	
		- int8 (0, 127)
		- unsigned int8 (255)
		- int16 (32767)
		- unsigned int16 (65535)
		- int32 (2147483647)
		- unsigned int32 (4294967295)
		- int64 (9223372036854775807)
		- unsigned int64 (18446744073709551615)
	
	@see: L{BadNumbers16}, L{NumbersVariance}, L{BadUnsignedNumbers}, L{BadPositiveNumbers}
	'''
	
	_ints = [
		50,						# Don't want any negative numbers
		127,
		255,					# unsigned 8
		32767,
		65535,					# unsigned 16
		2147483647,
		4294967295,				# unisnged 32
		9223372036854775807,
		18446744073709551615,	# unsigned 64
		]
	
	def __init__(self, group = None):
		SimpleGenerator.__init__(self, group)
		self._generator = WithDefault(None, 10, NumbersVariance(None, self._ints, 50))
	
	def getRawValue(self):
		try:
			val = self._generator.getValue()
			
			if val < 0:
				val = 0
		
		except OverflowError:
			# Wow, that sucks!
			print "BadPositiveNumbers(): OverflowError spot 1!"
			return str(0)
		
		return str(val)


class BadUnsignedNumbers(SimpleGenerator):
	'''
	Generate numbers that may trigger integer overflows for
	both signed and unsigned numbers.  Under the hood this generator
	performs a L{NumbersVariance} on the boundry numbers for:
	
		- unsigned int8  (0, 255)
		- unsigned int16 (65535)
		- unsigned int32 (4294967295)
		- unsigned int64 (18446744073709551615)
	
	@see: L{BadNumbers16}, L{NumbersVariance}, L{BadUnsignedNumbers}, L{BadPositiveNumbers}
	'''
	
	_ints = [
		50,
		255,
		65535,
		4294967295,
		18446744073709551615,
		]
	
	def __init__(self, group = None):
		SimpleGenerator.__init__(self, group)
		self._generator = WithDefault(None, 10, NumbersVariance(None, self._ints, 50))
	
	def getRawValue(self):
		try:
			val = self._generator.getValue()
			
			if val < 0:
				val = 0
		
		except OverflowError:
			# Wow, that sucks!
			print "BadUnsignedNumbers(): OverflowError spot 1!"
			return str(0)
		
		return str(val)
	

class BadUnsignedNumbers16(SimpleGenerator):
	'''
	Generate numbers that may trigger integer overflows for
	both signed and unsigned numbers.  Under the hood this generator
	performs a L{NumbersVariance} on the boundry numbers for:
	
		- unsigned int8  (0, 255)
		- unsigned int16 (65535)
	
	@see: L{BadNumbers16}, L{NumbersVariance}, L{BadUnsignedNumbers}, L{BadPositiveNumbers}
	'''
	
	_ints = [
		50,
		255,
		65535,
		]
	
	def __init__(self, group = None):
		SimpleGenerator.__init__(self, group)
		self._generator = WithDefault(None, 10, NumbersVariance(None, self._ints, 50))
	
	def getRawValue(self):
		try:
			val = self._generator.getValue()
			
			if val < 0:
				val = 0
		
		except OverflowError:
			# Wow, that sucks!
			print "BadUnsignedNumbers(): OverflowError spot 1!"
			return str(0)
		
		return str(val)


class Wrap(SimpleGenerator):
	'''
	Wraps another generator.  This is usefull when you
	want to re-use a generator in a GeneratorList with
	different transformers to change the permutations.
	
	Note: Wrap is implemented using a deep copy of the
	generator object passed to it.
	
	Example:
	
	  gen = Static('123456')
	  
	  allThings = GeneratorList([
		gen,
		Wrap(gen).setTransformer(HtmlEncode()),
		Wrap(gen).setTransformer(UrlEncode())
		])
	
	'''
	
	def __init__(self, generator):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	generator: Generator
		@param	group: Generator to wrap
		'''
		
		Generator.__init__(self)
		self._generator = copy.deepcopy(generator)


class BadIpAddress(SimpleGenerator):
	'''
	[BETA] Generate some bad ip addresses.  Needs work
	should also implement one for ipv6.
	'''
	
	def __init__(self, group = None):
		SimpleGenerator.__init__(self, group)
		self._groupA = Group()
		self._groupB = Group()
		self._generator = GeneratorList(None, [
			Static('10.10.10.10'),
			
			GeneratorList2(None, [
				self._groupA,
				self._groupB
				], [
				GeneratorList(self._groupA, [
					List(None, [
						'0', '0.', '1.', '1.1', '1.1.1', '1.1.1.1.',
						'.1', '.1.1.1', '.1.1.1.1',
						'1.1.1.1\0AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA',
						'0.0.0.0',
						'127.0.0.1',
						'255.255.255',
						'0.0.0.0',
						'256.256.256',
						'-1.-1.-1.-1',
						'FF.FF.FF',
						'\0.\0.\0.\0',
						'\01.\01.\01.\01',
						'\00.\00.\00.\00',
						'1\0.1\0.1\0.1\0',
						'1\0.\01\0.\01\0.\01\0',
						'0\0.\00\0.\00\0.\00\0',
						'999.999.999'
						]),
					
					Block2([
						BadNumbersAsString(),
						Static('.'),
						BadNumbersAsString(),
						Static('.'),
						BadNumbersAsString(),
						Static('.'),
						BadNumbersAsString()
						])
					]),
				
				Block([
					Repeater(self._groupB, Static('120.'), 1, 20),
					Static('1')
					]),
				], 'BadIpAddress-Sub'),
			
				Static('10.10.10.10')
			], 'BadIpAddress')


class TopLevelDomains(SimpleGenerator):
	'''
	Top-level domain names in upper case.  List current
	as of 12/06/2006.  Includes country's.
	'''
	
	def __init__(self, group = None):
		SimpleGenerator.__init__(self, group)
		self._generator = List(None, [
			'com',
			'AC',	'AD',	'AE',	'AERO',	'AF',
			'AG',	'AI',	'AL',	'AM',	'AN',
			'AO',	'AQ',	'AR',	'ARPA',	'AS',
			'AT',	'AU',	'AW',	'AX',	'AZ',
			'BA',	'BB',	'BD',	'BE',	'BF',
			'BG',	'BH',	'BI',	'BIZ',	'BJ',
			'BM',	'BN',	'BO',	'BR',	'BS',
			'BT',	'BV',	'BW',	'BY',	'BZ',
			'CA',	'CAT',	'CC',	'CD',	'CF',
			'CG',	'CH',	'CI',	'CK',	'CL',
			'CM',	'CN',	'CO',	'COM',	'COOP',
			'CR',	'CU',	'CV',	'CX',	'CY',
			'CZ',	'DE',	'DJ',	'DK',	'DM',
			'DO',	'DZ',	'EC',	'EDU',	'EE',
			'EG',	'ER',	'ES',	'ET',	'EU',
			'FI',	'FJ',	'FK',	'FM',	'FO',
			'FR',	'GA',	'GB',	'GD',	'GE',
			'GF',	'GG',	'GH',	'GI',	'GL',
			'GM',	'GN',	'GOV',	'GP',	'GQ',
			'GR',	'GS',	'GT',	'GU',	'GW',
			'GY',	'HK',	'HM',	'HN',	'HR',
			'HT',	'HU',	'ID',	'IE',	'IL',
			'IM',	'IN',	'INFO',	'INT',	'IO',
			'IQ',	'IR',	'IS',	'IT',	'JE',
			'JM',	'JO',	'JOBS',	'JP',	'KE',
			'KG',	'KH',	'KI',	'KM',	'KN',
			'KR',	'KW',	'KY',	'KZ',	'LA',
			'LB',	'LC',	'LI',	'LK',	'LR',
			'LS',	'LT',	'LU',	'LV',	'LY',
			'MA',	'MC',	'MD',	'MG',	'MH',
			'MIL',	'MK',	'ML',	'MM',	'MN',
			'MO',	'MOBI',	'MP',	'MQ',	'MR',
			'MS',	'MT',	'MU',	'MUSEUM',	'MV',
			'MW',	'MX',	'MY',	'MZ',	'NA',
			'NAME',	'NC',	'NE',	'NET',	'NF',
			'NG',	'NI',	'NL',	'NO',	'NP',
			'NR',	'NU',	'NZ',	'OM',	'ORG',
			'PA',	'PE',	'PF',	'PG',	'PH',
			'PK',	'PL',	'PM',	'PN',	'PR',
			'PRO',	'PS',	'PT',	'PW',	'PY',
			'QA',	'RE',	'RO',	'RU',	'RW',
			'SA',	'SB',	'SC',	'SD',	'SE',
			'SG',	'SH',	'SI',	'SJ',	'SK',
			'SL',	'SM',	'SN',	'SO',	'SR',
			'ST',	'SU',	'SV',	'SY',	'SZ',
			'TC',	'TD',	'TF',	'TG',	'TH',
			'TJ',	'TK',	'TL',	'TM',	'TN',
			'TO',	'TP',	'TR',	'TRAVEL',	'TT',
			'TV',	'TW',	'TZ',	'UA',	'UG',
			'UK',	'UM',	'US',	'UY',	'UZ',
			'VA',	'VC',	'VE',	'VG',	'VI',
			'VN',	'VU',	'WF',	'WS',	'YE',
			'YT',	'YU',	'ZA',	'ZM',	'ZW',
			'com'
			])


class BadHostname(BadString):
	'''
	[BETA] Crazy hostnames.
	'''
	
	def __init__(self, group = None):
		SimpleGenerator.__init__(self, group)
		self._generator = GeneratorList(None, [
#			Static('localhost'),
#			BadString(),
#			Repeater(None, Static('A'), 1, 1000),
#			Repeater(None, Static('A'), 100, 100),
			Repeater(None, Static('A.'), 5, 100),
			Repeater(None, Static('.'), 1, 10),
			Repeater(None, Static('.'), 20, 20),
			Block2([
				Repeater(None, Static('A'), 5, 20),
				Static('.'),
				Repeater(None, Static('A'), 5, 20),
				Static('.'),
				Repeater(None, Static('A'), 5, 20)
				]),
			Block2([
				Static('AAAA.'),
				TopLevelDomains()
				]),
			
			Static('localhost')
			])


class BadPath(BadString):
	'''
	[BETA] Path generation fun!
	'''
	
	def __init__(self, group = None):
		SimpleGenerator.__init__(self, group)
		self._generator = GeneratorList(None, [
			Static('A'),
			Repeater(None, Static('.'), 1, 1000),
			Repeater(None, Static('\\'), 1, 1000),
			Repeater(None, Static('/'), 1, 1000),
			Repeater(None, Static(':'), 1, 1000),
			Repeater(None, Static('../'), 1, 1000),
			Repeater(None, Static('..\\'), 1, 1000),
			Repeater(None, Static('*\\'), 10, 100),
			Repeater(None, Static('*/'), 10, 100),
			Repeater(None, Static('//\\'), 10, 100),
			Repeater(None, Static('//..\\..'), 10, 100),
			Repeater(None, Static('aaa//'), 10, 100),
			Repeater(None, Static('aaa\\'), 10, 100),
			Block2([
				BadString(),
				Static(':\\')
				]),
			Block2([
				BadString(),
				Static(':/')
				]),
			Block2([
				Static('\\\\'),
				BadString(),
				]),
			Block2([
				Static('./'),
				BadString()
				]),
			Block2([
				Static('/'),
				BadString(),
				Static('/')
				]),
			Static('A')
			])


class BadFilename(BadString):
	'''
	Lots of bad filenames.
	'''
	
	def __init__(self, group = None):
		SimpleGenerator.__init__(self, group)
		self._generator = GeneratorList(None, [
			Static('Peach.txt'),
			BadString(),
			Block2([
				BadString(),
				Static('.'),
				BadString()
				]),
			Block2([
				Static("."),
				BadString()
				]),
			Block2([
				BadString(),
				Static('.')
				]),
			Repeater(None, Static('.'), 1, 1000),
			Repeater(None, Static("a.a"), 1, 1000),
			Block2([
				Static("A."),
				Repeater(None, Static('A'), 1, 1000)
				]),
			Block2([
				Repeater(None, Static('A'), 1, 1000),
				Static('.A')
				]),
			Block2([
				Static('AAAA'),
				Repeater(None, Static('.doc'), 1, 1000)
				]),
			Block2([
				Repeater(None, Static('A'), 10, 100),
				Static('.'),
				Repeater(None, Static('A'), 10, 100)
				]),
				Static('Peach.txt'),
			])

class AsInt4x4(SimpleGenerator):
	'''
	Specify the high and low parts of an Int8
	'''
	
	def __init__(self, group, high, low, isLittleEndian = 1):
		'''
		@type	group: Group
		@param	group: [optional] Group for this generator
		@type	high: Generator
		@param	high: High portion of octet
		@type	low: Generator
		@param	low: Low portion of octet
		@type	isLittleEndian: number
		@param	isLittleEndian: 1 for little, 0 for big
		'''
		Generator.__init__(self)
		self.setGroup(group)
		self._high = high
		self._low = low
		self._isLittleEndian = isLittleEndian
	
	def next(self):
		completed = 0
		
		try:
			self._high.next()
		except GeneratorCompleted:
			completed = 1
		
		try:
			self._low.next()
		except GeneratorCompleted:
			if completed == 1:
				raise GeneratorCompleted("AsInt4x4 Completed")
	
	def getRawValue(self):
		
		high = int(self._high.getValue())
		low = int(self._low.getValue())
		
		ret = (high << 4) + low
		
		if self._isLittleEndian == 1:
			return struct.pack("<B", ret)
		else:
			return struct.pack(">B", ret)
	
	def reset(self):
		self._high.reset()
		self._low.reset()
		

class _AsInt(SimpleGenerator):
	'''
	Base class for AsIntXX functions that implements logic
	to skip values that are the same.
	'''
	
	_last = None
	_inMe = 0
	
	def _getValue(self):
		return self._transformer.transform(self._generator.getValue())
	
	def next(self):
		'''
		Our implementation of next will return skip
		values that are the same as the last value generated.
		
		This is done because packing larger numbers down can
		result in the same value lots of times.
		'''
		
		self._generator.next()
		cur = self._getValue()
		
		while cur == self._last:
			# Skip till we have something different
			try:
				self._generator.next()
			except GeneratorCompleted:
				break
			
			cur = self._getValue()
		
		self._last = cur
		
	def reset(self):
		SimpleGenerator.reset(self)
		cur = None
	
	def getRawValue(self):
		return self._generator.getValue()


class AsInt8(_AsInt):
	'''
	Cause generated value to be an 8 bit number.
	'''
	def __init__(self, group, generator, isSigned = 1, isLittleEndian = 1):
		'''
		@type	isSigned: number
		@param	isSigned: 1 for signed, 0 for unsigned
		@type	isLittleEndian: number
		@param	isLittleEndian: 1 for little, 0 for big
		'''
		SimpleGenerator.__init__(self, group)
		self._generator = generator
		self.setTransformer(Peach.Transformers.type.AsInt8(isSigned, isLittleEndian))
		

class AsInt16(_AsInt):
	'''
	Cause generated value to be a 16 bit number
	'''
	def __init__(self, group, generator, isSigned = 1, isLittleEndian = 1):
		'''
		@type	isSigned: number
		@param	isSigned: 1 for signed, 0 for unsigned
		@type	isLittleEndian: number
		@param	isLittleEndian: 1 for little, 0 for big
		'''
		SimpleGenerator.__init__(self, group)
		self._generator = generator
		self.setTransformer(Peach.Transformers.type.AsInt16(isSigned, isLittleEndian))


class AsInt24(_AsInt):
	'''
	Cause generated value to be a 24 bit number (don't ask)
	'''
	def __init__(self, group, generator, isSigned = 1, isLittleEndian = 1):
		'''
		@type	isSigned: number
		@param	isSigned: 1 for signed, 0 for unsigned (we ignore this)
		@type	isLittleEndian: number
		@param	isLittleEndian: 1 for little, 0 for big
		'''
		SimpleGenerator.__init__(self, group)
		self._generator = generator
		self.setTransformer(Peach.Transformers.type.AsInt24(isSigned, isLittleEndian))


class AsInt32(_AsInt):
	'''
	Cause generated value to be a 32 bit number
	'''
	def __init__(self, group, generator, isSigned = 1, isLittleEndian = 1):
		'''
		@type	isSigned: number
		@param	isSigned: 1 for signed, 0 for unsigned
		@type	isLittleEndian: number
		@param	isLittleEndian: 1 for little, 0 for big
		'''
		SimpleGenerator.__init__(self, group)
		self._generator = generator
		self.setTransformer(Peach.Transformers.type.AsInt32(isSigned, isLittleEndian))


class AsInt64(_AsInt):
	'''
	Cause generated value to be a 64 bit number
	'''
	def __init__(self, group, generator, isSigned = 1, isLittleEndian = 1):
		'''
		@type	isSigned: number
		@param	isSigned: 1 for signed, 0 for unsigned
		@type	isLittleEndian: number
		@param	isLittleEndian: 1 for little, 0 for big
		'''
		SimpleGenerator.__init__(self, group)
		self._generator = generator
		self.setTransformer(Peach.Transformers.type.AsInt64(isSigned, isLittleEndian))


class WithDefault(SimpleGenerator):
	'''
	Wrapps a Generator and makes the first and last value be a default value.
	'''
	def __init__(self, group, default, generator):
		'''
		@type	default: Python primitive or Generator
		@param	default: Default value
		@type	generator: Generator
		@param	generator: Generator to wrap
		'''
		SimpleGenerator.__init__(self, group)
		# If the user passed us a generator as our default value, get the
		# first value out of it and use it.
		if (str(type(default)) == "<type 'instance'>" and hasattr(default, "getValue")):
			self._default = default
		else:
			self._default = Static(default)
		
		self._generator = GeneratorList(None, [
			self._default,
			generator,
			self._default,
			])
	
	def setDefaultValue(self, data):
		'''
		Set the default value, assumes we have a static or
		some other generator that exposes a "setValue()" method
		'''
		self._default.setValue(data)


class FixedLengthString(SimpleGenerator):
	'''
	Generates a fixed length string.  If the generated string
	that this wrapps is to long it will be truncated.  If to short
	it will be padded.
	
	@author Collin Greene
	'''
	def __init__(self, group, gen, length, padChar, charSize=1):
		'''
		@type	group: Group
		@param	group: Group
		@type	generator: Generator
		@param	generator: Generator that generates strings
		@type	length: int
		@param	length: Length of string
		@type	padChar: string
		@param	padChar: Character to pad string with
		@type	charSize: int
		@param	charSize: Character size, defaults to 1.  For WCHARs use 2.
		'''
		SimpleGenerator.__init__(self, group)
		self._generator = gen
		self._maxLength = length
		# at minimum charsize must be 1
		self._charSize = max(charSize, 1)
		self._padChar = padChar
		if len(padChar) != charSize:
			raise Exception("FixedLengthString(): padChar is not the same length as charSize.")

	def getRawValue(self):
		val = self._generator.getValue()
		
		if val is not None and len(val) % self._charSize != 0:
			raise Exception("FixedLengthString(): length of value is not a multiple of charSize.")
		
		# If the value is too long, return a subset of it.
		if val is not None and len(val) > self._maxLength * self._charSize:
			return val[:self._maxLength * self._charSize]
		
		# If the value is too short, pad it.
		if val is not None and len(val) < self._maxLength * self._charSize:
		   padding = ((self._maxLength * self._charSize) - len(val)) / self._charSize
		   return val + (self._padChar * padding)
		
		return val


class FlagPermutations(SimpleGenerator):
	'''
	[BETA] Generates all possible permutations of bit flag combinations.  Flags
	are ored with each other.  Currently we only do a max of 2 orings
	
	TODO: Make it generate all permutations.
	
	@author Collin Greene
	'''
	def __init__(self, group, flags):
		SimpleGenerator.__init__(self, group)
		self.__flags = flags
		self._y = 0
		self._x = 0


	def next(self):
		self._y += 1
		if self._y >= len(self.__flags):
			self._y = 0
			self._x += 1
			
			if self._x >= len(self.__flags):
				self._x -= 1
				raise GeneratorCompleted("Bah WE ARE DOONE!!!!")
				
	def reset(self):
		self._x = 0
		self._y = 0

	def getRawValue(self):
		return self.__flags[self._x] | self.__flags[self._y]


import random
class PseudoRandomNumber(Generator):
	'''
	Use the pseudo random number generator to return a set of random numbers.
	Each run of numbers will be the same if the seed is the same.  The default
	seed used is 6587243.
	
	NOTE: If numbersOfNumbers is not specified an infinit number of numbers is
	generated.
	'''
	
	_seed = 6587243234234
	
	def __init__(self, group, numberOfNumbers = None, seed = None, min = None, max = None):
		'''
		@type	group: Group
		@param	group: Group
		@type	numberOfNumbers: Number
		@param	numberOfNumbers: [optional] Number of random numbers to generate
		@type	seed: Number
		@param	seed: [optional] Seed to pseudo random number generator
		@type	min: Number
		@param	min: [optional] Minimum number to return
		@type	max: Number
		@param	max: [optional] Maximum number to return
		'''
		
		Generator.__init__(self)
		self.setGroup(group)
		
		if seed != None:
			self._seed = seed
		
		self._min = min
		self._max = max
		self._numberOfNumbers = numberOfNumbers
		self._count = 0
		self._random = random.Random()
		
		if self._min == None and self._max != None:
			raise Exception("Cannot set only one of min or max")
		if self._max == None and self._min != None:
			raise Exception("Cannot set only one of min or max")
		
		self._random.seed(self._seed)
		self._getRandomNumber()
	
	def _getRandomNumber(self):
		if self._min == None:
			#self._current = self._random.randint(0, 18446744073709551615)
			self._current = self._random.random()
		else:
			self._current = self._random.randint(self._min, self._max)
	
	def next(self):
		self._count += 1
		if self._numberOfNumbers != None and self._count >= self._numberOfNumbers:
			raise GeneratorCompleted("PseudoRandomNumber is done")
		self._getRandomNumber()
	
	def reset(self):
		self._random.seed(self._seed)
		self._getRandomNumber()
		self._count = 0
	
	def getRawValue(self):
		return self._current


class BadDerEncodedOctetString(Generator):
	'''
	Performs DER encoding of an octect string with incorrect lengths.
	'''
	
	def __init__(self, group, generator):
		'''
		@type	group: Group
		@param	group: Group
		@type	generator: Generator
		@param	generator: Generator that produces strings that will have bad der encodings done on 'em
		'''
		
		Generator.__init__(self)
		self.setGroup(group)
		self._generator = generator
		self._setupNextValue()
	
	def _setupNextValue(self):
		self._string = self._generator.getValue()
		self._variance = GeneratorList(None, [
			Static(len(self._string)),
			NumberVariance(None, len(self._string), 20, 0),
			Static(len(self._string)),
			])
	
	def next(self):
		try:
			self._variance.next()
		except GeneratorCompleted:
			self._generator.next()
			self._setupNextValue()
	
	def reset(self):
		self._variance.reset()
	
	def getRawValue(self):
		val = '\x04'
		length = self._variance.getValue()
		
		if length < 255:
			val += struct.pack("B", length)
		
		elif length < 65535:
			cal += '\x82'
			val += struct.pack("H", length)
		
		elif length < 4294967295:
			cal += '\x83'
			val += struct.pack("I", length)
		
		elif length < 18446744073709551615:
			cal += '\x84'
			val += struct.pack("L", length)
		
		else:
			raise Exception("Length way to big for us %d" % length)
		
		return val + self._string


class BadBerEncodedOctetString(BadDerEncodedOctetString):
	'''
	Performs BER encoding of an octect string with incorrect lengths.
	'''
	pass


class BadFormatString(BadString):
	'''
	Generates chains that can show a format string vulnerability
	'''

	_strings = ["%","a%","%a",'%n',
		'%x',	
		'%p',
		'%d',
		'%s',
		'%s%p%x%d',
		'%.1023d',
		'%.1024d',
		'%.1025d',
		'%.2047d',
		'%.2048d',
		'%.2049d',
		'%.4096d',
		'%.8200d',
		'%99999999999s',
		'%99999999999d',
		'%99999999999x',
		'%99999999999n',
		'%08x',
		'%%20d',
		'%%20n',
		'%%20x',
		'%%20s',				
    '%#0123456x%08x%x%s%p%d%n%o%u%c%h%l%q%j%z%Z%t%i%e%g%f%a%C%S%08x%%'		 
	] 
	
class SQLInjection(BadString):
	'''
	Generates chains that can show a sql injection vulnerability
	'''
	
	_strings = [
		"'||(elt(-3+5,bin(15),ord(10),hex(char(45))))",
		"||6",
		"'||'6",
		"(||6)",
		"' OR 1=1-- ",
		"OR 1=1",
		"' OR '1'='1",
		"; OR '1'='1'",
		"%22+or+isnull%281%2F0%29+%2F*",
		"%27+OR+%277659%27%3D%277659",
		"%22+or+isnull%281%2F0%29+%2F*",
		"%27+--+",
		"' or 1=1--",
		"\" or 1=1--",
		"' or 1=1 /*",
		"or 1=1--",
		"' or 'a'='a",
		"\" or \"a\"=\"a",
		"') or ('a'='a",
		"Admin' OR '",
		"'%20SELECT%20*%20FROM%20INFORMATION_SCHEMA.TABLES--",
		") UNION SELECT%20*%20FROM%20INFORMATION_SCHEMA.TABLES;",
		"' having 1=1--",
		"' having 1=1--",
		"' group by userid having 1=1--",
		"' SELECT name FROM syscolumns WHERE id = (SELECT id FROM sysobjects WHERE name = tablename')--",
		"' or 1 in (select @@version)--",
		"' union all select @@version--",
		"' OR 'unusual' = 'unusual'",
		"' OR 'something' = 'some'+'thing'",
		"' OR 'text' = N'text'",
		"' OR 'something' like 'some%'",
		"' OR 2 > 1",
		"' OR 'text' > 't'",
		"' OR 'whatever' in ('whatever')",
		"' OR 2 BETWEEN 1 and 3",
		"' or username like char(37);",
		"' union select * from users where login = char(114,111,111,116);",
		"' union select", 
		"Password:*/=1--",
		"UNI/**/ON SEL/**/ECT",
		"'; EXECUTE IMMEDIATE 'SEL' || 'ECT US' || 'ER'",
		"'; EXEC ('SEL' + 'ECT US' + 'ER')",
		"'/**/OR/**/1/**/=/**/1",
		"' or 1/*",
		"+or+isnull%281%2F0%29+%2F*",
		"%27+OR+%277659%27%3D%277659",
		"%22+or+isnull%281%2F0%29+%2F*",
		"%27+--+&password=",
		"'; begin declare @var varchar(8000) set @var=':' select @var=@var+'+login+'/'+password+' ' from users where login >", 
		" @var select @var as var into temp end --",
		"' and 1 in (select var from temp)--",
		"' union select 1,load_file('/etc/passwd'),1,1,1;",
		"1;(load_file(char(47,101,116,99,47,112,97,115,115,119,100))),1,1,1;",
		"' and 1=( if((load_file(char(110,46,101,120,116))<>char(39,39)),1,0));",
	]
	
class HexData(BadString):
	'''
	Generates chains in an hexadecimal way
	'''
	_count = 0
	
	def getRawValue(self):
		val = self._generator.getValue()
		out = ""
		for i in range(len(val)):
			if self._count == 0:
				# Data like '0x45'
				out += hex(ord(val[i]))
			else:
				# Data like '\x45'
				out += '\\x'+hex(ord(val[i]))[2:]
		if val == self._strings[len(self._strings)-1] and self._count == 0:
			self._count = 1
			self._generator.reset()
		return out


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
			
	except GeneratorCompleted:
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
