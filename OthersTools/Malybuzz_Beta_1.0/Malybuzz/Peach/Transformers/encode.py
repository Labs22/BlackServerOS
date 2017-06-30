
'''
Encoding transforms (URL, Base64, etc).

@author: Michael Eddington
@version: $Id: encode.py 279 2007-05-08 01:21:58Z meddingt $
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

# $Id: encode.py 279 2007-05-08 01:21:58Z meddingt $


import urllib
import xml.sax.saxutils
import base64
from struct import *
from Peach.transformer import Transformer

class WideChar(Transformer):
	'''
	Make a normal string a wchar string
	'''
	def realTransform(self, data):
		ret = ""
		for c in str(data):
			ret +=  c + '\x00'
		
		return ret


class UrlEncode(Transformer):
	'''
	URL encode w/o pluses.
	'''
	
	def realTransform(self, data):
		return urllib.quote(data)
	
	def unittest():
		t = UrlEncode()
		print "UrlEncode: [" + t.realTransform("<script> alert('meow') </script>") + "]"
	unittest = staticmethod(unittest)



class NetBiosDecode(Transformer):
	'''
	NetBiosName Decode
	
	@author: Blake Frantz 
	'''
	
	def __init__(self, anotherTransformer = None):
		'''
		Create Transformer object.
		
		@type	anotherTransformer: Transformer
		@param	anotherTransformer: A transformer to run next

		'''
		self._anotherTransformer = anotherTransformer
	
	def realTransform(self, data):		
		
		if data % 2 != 0:
			raise("Invalid NetBiosEncoding -- length must be divisible by two")
		
		decoded = ""
		data = data.upper()
		
		# loop through each byte
		for cnt in range(0, len(data), 2):
		
			c1 = ord(data[cnt])
			c2 = ord(data[cnt+1])
			
			part1 = (c1 - 0x41) * 16
			part2 = (c2 - 0x41)
			
			decoded += chr(part1 + part2)
			
		return decoded
	
	def unittest():
		t = NetBiosDecode()
		print "NetBiosDecode: [" + t.realTransform("blewp") + "]"
		
	unittest = staticmethod(unittest)


class NetBiosEncode(Transformer):
	'''
	NetBiosName Encoded
	
	@author: Blake Frantz 
	'''
	
	def __init__(self, anotherTransformer = None, pad = True):
		'''
		Create Transformer object.
		
		@type	anotherTransformer: Transformer
		@param	anotherTransformer: A transformer to run next
		@type	pad: Boolean
		@param	pad: Will pad / trim encoded name to 32 bytes
		'''
		self._anotherTransformer = anotherTransformer
		self._pad = pad
	
	def realTransform(self, data):		
		
		encoded = ""
		
		data = data.upper()
		
		if self._pad:
			while len(data) < 16:
				data += " "

			data = data[:16]
		
		# loop through each byte
		for c in data:
			
			# grab the ascii value of it
			ascii = ord(c)
			
			encoded += chr((ascii / 16) + 0x41)
			encoded += chr((ascii - (ascii/16 * 16) + 0x41))
			
		# the 16th byte is the name scope id, set it to 'host'
		# and null term it
		if self._pad:
			encoded = encoded[:30]
			encoded += '\x41'
			encoded += '\x41'
		
		return encoded
	

	def unittest():
		t = NetBiosEncode()
		print "NetBiosEncode: [" + t.realTransform("blewp") + "]"
		
	unittest = staticmethod(unittest)


class UrlEncodePlus(Transformer):
	'''
	URL encode with spaces turned to pluses
	'''
	
	def realTransform(self, data):
		return urllib.quote_plus(data)
	
	def unittest():
		t = UrlEncodePlus()
		print "UrlEncodePlus: [" + t.realTransform("<script> alert('meow') </script>") + "]"
	unittest = staticmethod(unittest)
	

class BadUrlEncode(UrlEncode):
	'''
	TODO: IMPLEMENT ME!
	'''
	pass
	

class Base64Encode(Transformer):
	'''
	Base64 encode.
	'''
	def realTransform(self, data):
		return base64.encodestring(data).rstrip()
	
	def unittest():
		t = Base64Encode()
		print "Base64Encode: [" + t.realTransform("<script> alert('meow') </script>") + "]"
	unittest = staticmethod(unittest)
	
class Base64Decode(Transformer):
	'''
	Base64 decode.
	'''
	def realTransform(self, data):
		return base64.decodestring(data)
	
	def unittest():
		e = Base64Encode()
		t = Base64Decode()
		print "Base64Decode: [" + t.realTransform(e.realTransform("hellow world!")) + "]"
	unittest = staticmethod(unittest)
	

def _HtmlEncode(strInput, default=''):
	
	if strInput == None or len(strInput) == 0:
		strInput = default
		
		if strInput == None or len(strInput) == 0:
			return ''
	
	# Allow: a-z A-Z 0-9 SPACE , .
	# Allow (dec): 97-122 65-90 48-57 32 44 46
	
	out = ''
	for char in strInput:
		c = ord(char)
		if ((c >= 97 and c <= 122) or
			(c >= 65 and c <= 90 ) or
			(c >= 48 and c <= 57 ) or
			c == 32 or c == 44 or c == 46):
			out += char
		else:
			out += "&#%d;" % c
	
	return out

class HtmlEncodeAgressive(Transformer):
	'''
	Perform agressive HTML encoding.  Only alphanum's will not
	be encoded.
	'''
	
	def realTransform(self, data):
		return _HtmlEncode(data)
	
	def unittest():
		t = HtmlEncodeAgressive()
		print "HtmlEncode: [" + t.realTransform("<script> alert('meow') </script>") + "]"
	unittest = staticmethod(unittest)


class HtmlEncode(Transformer):
	'''
	Perform standard HTML encoding of < > & and "
	'''
	
	def realTransform(self, data):
		return xml.sax.saxutils.quoteattr(data).strip('"')
	
	def unittest():
		t = HtmlEncode()
		print "HtmlEncode: [" + t.realTransform("<script> alert('meow') </script>") + "]"
	unittest = staticmethod(unittest)
	
class HtmlDecode(Transformer):
	'''
	Decode HTML encoded string
	'''
	
	def realTransform(self, data):
		return xml.sax.saxutils.unescape(data)
	
	def unittest():
		e = HtmlEncode()
		t = HtmlDecode()
		print "HtmlDecode: [" + t.realTransform(e.realTransform("<script> alert('meow') </script>")) + "]"
	unittest = staticmethod(unittest)


class Utf16(Transformer):
	'''
	Encode string as UTF-16.  String is prefixed with BOM.
	Supports surrogate pair	encoding of values larger then 0xFFFF.
	'''
	
	def realTransform(self, data):
		if pack(">H", 0xFF00) == pack("=H", 0xFF00):
			return "%c%c%s" % (0xFE, 0xFF, Utf16Be().realTransform(data))
		
		return "%c%c%s" % (0xFF, 0xFE, Utf16Le().realTransform(data))
	
	def unittest():
		print "Utf16: NO UNIT TEST"
	unittest = staticmethod(unittest)


class Utf16Le(Transformer):
	'''
	Encode string as UTF-16LE.  Supports surrogate pair
	encoding of values larger then 0xFFFF.
	'''
	
	def realTransform(self, data):
		ret = ''
		
		for c in data:
			if ord(c) <= 0xFF:
				return pack("<BB", 0x00, ord(c))
			elif ord(c) <= 0xFFFF:
				return pack("<H", ord(c))
			elif ord(c) > 0xFFFF:
				# Perform surrogate pair encoding
				value = ord(c)		# value
				value -= 0x10000
				valueHigh = value & 0xFFF		# high bits
				valueLow = 	value &	0xFFF000	# low bits
				word1 = 0xD800
				word2 = 0xDC00
				word1 = word1 | valueHigh
				word2 = word2 | valueLow
				return pack("<HH", word1, word2)
		return ret
	
	def unittest():
		print "Utf8Le: NO UNIT TEST"
	unittest = staticmethod(unittest)
	
class Utf16Be(Transformer):
	'''
	Encode string as UTF-16BE.  Supports surrogate pair
	encoding of values larger then 0xFFFF.
	'''
	
	def realTransform(self, data):
		ret = ''
		
		for c in data:
			if ord(c) <= 0xFF:
				return pack(">BB", 0x00, ord(c))
			elif ord(c) <= 0xFFFF:
				return pack(">H", ord(c))
			elif ord(c) > 0xFFFF:
				# Perform surrogate pair encoding
				value = ord(c)		# value
				value -= 0x10000
				valueHigh = value & 0xFFF		# high bits
				valueLow = 	value &	0xFFF000	# low bits
				word1 = 0xD800
				word2 = 0xDC00
				word1 = word1 | valueHigh
				word2 = word2 | valueLow
				return pack("<HH", word1, word2)
		return ret
	
	def unittest():
		print "Utf8Be: NO UNIT TEST"
	unittest = staticmethod(unittest)


class Ipv4StringToOctet(Transformer):
	'''
	Convert a dot notiation ipv4 address into a 4 byte
	octect representation.
	'''
	
	def realTransform(self, data):
		return struct.pack('BBBB',data.split('.'))


class Ipv6StringToOctet(Transformer):
	'''
	Convert a collen notiation ipv6 address into a 4 byte
	octect representation.
	'''
	
	def realTransform(self, data):
		return struct.pack('BBBBBBBBBBBBBBBB',data.split(':'))


class HexString(Transformer):
	'''
	Transforms a string of bytes into the specified Hex format.
	
	Example:
	
	>>> gen = Static("AAAABBBB").setTransformer(HexString())
		>>> print gen.getValue()
	 41 41 41 41 42 42 42 42
	>>> gen = Static("AAAABBBB").setTransformer(HexString(None, 4, "0x"))
	>>> print gen.getValue()
	0x414141410x42424242
	>>> gen = Static("AAAABBBB").setTransformer(HexString(None, 1, " \\x"))
	>>> print gen.getValue()
	 \x41 \x41 \x41 \x41 \x42 \x42 \x42 \x42
	>>>
	'''
	
	def __init__(self, anotherTransformer = None, resolution = None, prefix = None):
		'''
		Create Transformer object.
		
		@type	anotherTransformer: Transformer
		@param	anotherTransformer: A transformer to run next
		@type	resolution: Int
		@param	resolution: Number of nibbles between separator (Must be a postive even integer) 
		@type	prefix: String 
		@param	prefix: A value to prepend each chunk with (defaults to ' ')
	
	'''
		self._anotherTransformer = anotherTransformer
		self._resolution = resolution
		self._prefix = prefix
	
	def realTransform(self, data):
		ret = ''
		
		if self._resolution == None:
			self._resolution = 1

		# try to detect if user passed in odd numbered value
		if self._resolution % 2 and self._resolution != 1:
			raise Exception("Resolution must be 1 or a multiple of two")
		
		if len(data) % self._resolution != 0:
			raise Exception("Data length must be divisible by resolution")
						
		if self._prefix == None:
			self._prefix = " "
		
		tmp = ''

		for c in data:
			h = hex(ord(c))[2:]
			
			if len(h) == 2:
				tmp += h 
			else:
				tmp += "0%s" % h
			
			if len(tmp) / 2 == self._resolution:
				ret += self._prefix + tmp
				tmp = ''
						
		ret = ret.strip()
			
		return ret



def unittest():
	UrlEncode.unittest()
	UrlEncodePlus.unittest()
	Base64Encode.unittest()
	Base64Decode.unittest()
	HtmlEncode.unittest()
	HtmlDecode.unittest()
	Utf16.unittest()
	Utf16Le.unittest()
	Utf16Be.unittest()

# end

