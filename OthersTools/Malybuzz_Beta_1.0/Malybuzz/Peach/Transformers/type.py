
'''
Type transforms (atoi, itoa, etc).
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

# $Id: type.py 279 2007-05-08 01:21:58Z meddingt $

import struct, types, sys
from types import *
from Peach.transformer import Transformer

#__all__ = ['Pack', 'NumberToString', 'StringToInt', 'StringToFloat',
#		   'IntToHex']

class Pack(Transformer):
	'''Simple pack transform.  Only a single piece of data can be used.
	Usefull to generate binary data from a generator.
	
	Format   	C Type   	Python   	Notes 
	x 	pad byte 	no value 	 
	c 	char 	string of length 1 	 
	b 	signed char 	integer 	 
	B 	unsigned char 	integer 	 
	h 	short 	integer 	 
	H 	unsigned short 	integer 	 
	i 	int 	integer 	 
	I 	unsigned int 	long 	 
	l 	long 	integer 	 
	L 	unsigned long 	long 	 
	q 	long long 	long 	(1)
	Q 	unsigned long long 	long 	(1)
	f 	float 	float 	 
	d 	double 	float 	 
	s 	char[] 	string 	 
	p 	char[] 	string 	 
	P 	void * 	integer 	 
	'''
	
	_packFormat = None
	
	def __init__(self, packFormat):
		'''Create a Pack trasnformer.  packFormat is a standard pack
		format string.  Format string should only contain a single
		data place holder.'''
		self._packFormat = packFormat
	
	def realTransform(self, data):
		'''Run pack on data'''
		
		return struct.pack(self._packFormat, data)
	
	def unittest():
		e = Pack('B')
		print "Pack 1: " + e.realTransform(64)
		e = Pack('!B')
		print "Pack 2: " + e.realTransform(64)
	unittest = staticmethod(unittest)


class NumberToString(Transformer):
	'''Transforms any type of number (int, long, float) to string.'''
	
	_formatString = None
	
	def __init__(self, formatString = None):
		'''Create NumberToString Instance.  formatString is a standard 
		Python string formater (optional).'''
		self._formatString = formatString
	
	def realTransform(self, data):
		'''Convert number to string.  If no formatString was specified
		in class contructor data type is dynamicly determined and converted
		using a default formatString of "%d", "%f", or "%d" for Int, Float,
		and Long respectively.'''
		
		if self._formatString == None:
			retType = type(data)
			if retType is IntType:
				return "%d" % data
			elif retType is FloatType:
				return "%f" % data
			elif retType is LongType:
				return "%d" % data
			else:
				return data
		
		return self._formatString % data
	
	def unittest():
		e = NumberToString()
		print "NumberToString 1: " + e.realTransform(1)
		print "NumberToString 2: " + e.realTransform(1.23343)
		e = NumberToString('%1.2f')
		print "NumberToString 3: " + e.realTransform(1)
		print "NumberToString 4: " + e.realTransform(1.23343)
	unittest = staticmethod(unittest)


class StringToInt(Transformer):
	'''Transform a string into an integer (atoi).'''
	
	def realTransform(self, data):
		return int(data)
	
	def unittest():
		e = StringToInt()
		print "StringToInt : %d" % e.realTransform("1")
	unittest = staticmethod(unittest)


class StringToFloat(Transformer):
	'''Transform a string into an float (atof).'''
	
	def realTransform(self, data):
		return float(data)
	
	def unittest():
		e = StringToFloat()
		print "StringToFloat: %f" % e.realTransform("1.12345")
	unittest = staticmethod(unittest)


class IntToHex(Transformer):
	'''Transform an integer into hex.'''
	
	_withPrefix = 0
	
	def __init__(self, withPrefix = 0):
		'''Create IntToHex object.  withPrefix flag indicates if
		0x prefix should be tagged onto string.  Default is no.'''
		self._withPrefix = withPrefix
	
	def realTransform(self, data):
		
		if type(data) == StringType:
			data = int(data)
		
		ret = hex(data)
		if self._withPrefix == 0:
			return ret[2:]
		
		return ret
	
	def unittest():
		e = IntToHex()
		print "IntToHex: " + e.realTransform(65)
	unittest = staticmethod(unittest)

class _AsNumber(Transformer):
	'''Transform an number to a specific size 'n stuff'''
	
	def __init__(self, isSigned = 1, isLittleEndian = 1):
		'''
		@type	isSigned: number
		@param	isSigned: 1 for signed, 0 for unsigned
		@type	isLittleEndian: number
		@param	isLittleEndian: 1 for signed, 0 for unsigned
		'''
		
		self._isSigned = isSigned
		self._isLittleEndian = isLittleEndian
	
	def _unfuglyNumber(self, data):
		'''
		Will attempt to figure out if the incoming data
		is a byte stream that must be converted to get our
		number we will then cast.  Due to StaticBinary issues.
		
		Chears to Blake for pointing this out.
		'''
		
		try:
			# First check to see if we need todo this
			if int(data) == int(str(data)):
				return int(data)
		except:
			pass
		
		# Now lets unfugly the thing
		hexString = ""
		for c in data:
			h = hex(ord(c))[2:]
			if len(h) < 2:
				h = "0" + h
			
			hexString += h
		
		return int(hexString, 16)
	
	def realTransform(self, data):
		
		data = self._unfuglyNumber(data)
		
		packStr = ''
		
		if self._isLittleEndian == 1:
			packStr = '<'
		else:
			packStr = '>'
		
		if self._isSigned == 1:
			packStr += self._packFormat.lower()
		else:
			packStr += self._packFormat.upper()
		
		#print type(data)
		#try:
		#	print data._identity
		#except:
		#	pass
		try:
			return struct.pack(packStr, int(data))
		except:
			return 0


class AsInt8(_AsNumber):
	'''Transform an number to an INT8 or UINT8
	'''
	_packFormat = 'b'
	
class AsInt16(_AsNumber):
	'''Transform an number to an INT16 or UINT16
	'''
	_packFormat = 'h'

class AsInt24(Transformer):
	'''Transform an number to a UINT24 (don't ask)
	'''
	
	def __init__(self, isSigned = 1, isLittleEndian = 1):
		'''
		@type	isSigned: number
		@param	isSigned: 1 for signed, 0 for unsigned (we ignore this)
		@type	isLittleEndian: number
		@param	isLittleEndian: 1 for signed, 0 for unsigned
		'''
		
		self._isLittleEndian = isLittleEndian
	
	def realTransform(self, data):
		
		if type(data) == 'str':
			data = int(data)
		
		packStr = ''
		
		if self._isLittleEndian == 1:
			packStr = '<'
		else:
			packStr = '>'
		
		packStr += 'L'
		
		ret = struct.pack(packStr, data)
		
		if self._isLittleEndian == 1:
			return data[:3]
		else:
			return data[1:]


class AsInt32(_AsNumber):
	'''Transform an number to an INT32 or UINT32
	'''
	_packFormat = 'l'

class AsInt64(_AsNumber):
	'''Transform an number to an INT64 or UINT64
	'''
	_packFormat = 'q'



def unittest():
	Pack.unittest()
	NumberToString.unittest()
	StringToInt.unittest()
	StringToFloat.unittest()
	IntToHex.unittest()

# end
