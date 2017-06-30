
'''
Crypto transforms (encrypting, hashing, etc), and misc auth crap.

@author: Michael Eddington
@version: $Id: crypto.py 287 2007-05-08 03:29:26Z meddingt $
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

# $Id: crypto.py 287 2007-05-08 03:29:26Z meddingt $


import md5, sha, fcrypt, md5crypt, hmac

#__all__ = ['Crypt', 'UnixMd5Crypt', 'ApacheMd5Crypt', 'CvsScramble',
#		   'Md5', 'Sha1', 'Hmac']

from Peach.transformer import Transformer
from Peach.Generators import static

class Crypt(Transformer):
	'''
	UNIX style crypt.  If no salt is specified will use first
	two chars of data, ala pwd style.
	
	This transform uses a pure Python implementation of the crypt
	function which had been ported from an old C version.  See
	fcrypt.py for licensing differences.
	
	From underlying docs:
	
	I{Generate an encrypted hash from the passed password.  If the password
	is longer than eight characters, only the first eight will be used.}
	
	I{The first two characters of the salt are used to modify the encryption
	algorithm used to generate in the hash in one of 4096 different ways.
	The characters for the salt should be upper- and lower-case letters A
	to Z, digits 0 to 9, '.' and '/'.}
	
	I{The returned hash begins with the two characters of the salt, and
	should be passed as the salt to verify the password.}
	'''
	
	_salt = None
	
	def __init__(self, salt = None):
		'''
		@type	salt: string
		@param	salt: Salt for crypt (optional)
		'''
		self._salt = salt
	
	def realTransform(self, data):
		if self._salt == None:
			return fcrypt.crypt(data, data[:2])
		return fcrypt.crypt(data, self._salt)
	
	def unittest():
		crypted = "heD1umJOQHx9A" # 'hello world' salt 'he'
		t = Crypt()
		print "Crypt 1: " + t.realTransform("hello world")
		if crypted != t.realTransform('hello world'):
			raise Exception("Crypt::unittest(): Failed to match 'hello world' with 'he' salt.")
		print "Crypt: " + t.realTransform("<script> alert('meow') </script>")
	unittest = staticmethod(unittest)


class UnixMd5Crypt(Transformer):
	'''
	UNIX style MD5 crypt.  If no salt is specified will use first
	two chars of data, ala pwd style.
	
	From underlying docs:
	
	I{unix_md5_crypt() provides a crypt()-compatible interface to the
	rather new MD5-based crypt() function found in modern operating systems.
	It's based on the implementation found on FreeBSD 2.2.[56]-RELEASE and
	contains the following license in it:}
	
	I{"THE BEER-WARE LICENSE" (Revision 42):
	<phk@login.dknet.dk> wrote this file.  As long as you retain this notice you
	can do whatever you want with this stuff. If we meet some day, and you think
	this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp}
	'''
	
	_salt = None
	_magic = None
	
	def __init__(self, salt = None, magic = None):
		'''
		@type	salt: string
		@param	salt: Salt for crypt (optional)
		@type	magic: string
		@param	magic: Magic, usually $1$ on unix (optional)
		'''
		self._salt = salt
		self._magic = magic
	
	def realTransform(self, data):
		if self._salt == None:
			return md5crypt.unix_md5_crypt(data, data[:2], self._magic)
		return md5crypt.unix_md5_crypt(data, self._salt, self._magic)
	
	def unittest():
		#crypted = "heD1umJOQHx9A" # 'hello world' salt 'he'
		t = UnixMd5Crypt()
		print "UnixMd5Crypt 1: " + t.realTransform("hello world")
		t = UnixMd5Crypt('ME')
		print "UnixMd5Crypt 2: " + t.realTransform("hello world")
		#if crypted != t.realTransform('hello world'):
		#	raise Exception("Crypt::unittest(): Failed to match 'hello world' with 'he' salt.")
		#print "Crypt: " + t.realTransform("<script> alert('meow') </script>")
	unittest = staticmethod(unittest)


class ApacheMd5Crypt(Transformer):
	'''
	Apache style MD5 crypt.  If no salt is specified will use first	two chars of
	data, ala pwd style.
	
	Uses '$apr1$' as magic.
	
	From underlying docs:
	
	I{apache_md5_crypt() provides a function compatible with Apache's
	.htpasswd files. This was contributed by Bryan Hart <bryan@eai.com>.}
	
	I{"THE BEER-WARE LICENSE" (Revision 42):
	<phk@login.dknet.dk> wrote this file.  As long as you retain this notice you
	can do whatever you want with this stuff. If we meet some day, and you think
	this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp}
	'''
	
	_salt = None
	
	def __init__(self, salt = None):
		'''
		@type	salt: string
		@param	salt: Salt for crypt (optional)
		'''
		self._salt = salt
	
	def realTransform(self, data):
		if self._salt == None:
			return md5crypt.apache_md5_crypt(data, data[:2])
		return md5crypt.apache_md5_crypt(data, self._salt)
	
	def unittest():
		#crypted = "heD1umJOQHx9A" # 'hello world' salt 'he'
		t = ApacheMd5Crypt()
		print "ApacheMd5Crypt 1: " + t.realTransform("hello world")
		t = ApacheMd5Crypt('ME')
		print "ApacheMd5Crypt 2: " + t.realTransform("hello world")
		#if crypted != t.realTransform('hello world'):
		#	raise Exception("Crypt::unittest(): Failed to match 'hello world' with 'he' salt.")
		#print "Crypt: " + t.realTransform("<script> alert('meow') </script>")
	unittest = staticmethod(unittest)


class CvsScramble(Transformer):
	'''
	CVS pserver password scramble
	'''
	
	_shifts = [
		0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
		16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
		114,120, 53, 79, 96,109, 72,108, 70, 64, 76, 67,116, 74, 68, 87,
		111, 52, 75,119, 49, 34, 82, 81, 95, 65,112, 86,118,110,122,105,
		41, 57, 83, 43, 46,102, 40, 89, 38,103, 45, 50, 42,123, 91, 35,
		125, 55, 54, 66,124,126, 59, 47, 92, 71,115, 78, 88,107,106, 56,
		36,121,117,104,101,100, 69, 73, 99, 63, 94, 93, 39, 37, 61, 48,
		58,113, 32, 90, 44, 98, 60, 51, 33, 97, 62, 77, 84, 80, 85,223,
		225,216,187,166,229,189,222,188,141,249,148,200,184,136,248,190,
		199,170,181,204,138,232,218,183,255,234,220,247,213,203,226,193,
		174,172,228,252,217,201,131,230,197,211,145,238,161,179,160,212,
		207,221,254,173,202,146,224,151,140,196,205,130,135,133,143,246,
		192,159,244,239,185,168,215,144,139,165,180,157,147,186,214,176,
		227,231,219,169,175,156,206,198,129,164,150,210,154,177,134,127,
		182,128,158,208,162,132,167,209,149,241,153,251,237,236,171,195,
		243,233,253,240,194,250,191,155,142,137,245,235,163,242,178,152
		]
	
	def realTransform(self, data):
		s = []
		for i in range(len(data)):
			s.append(data[i])
		s.append(None)
		for i in range(len(data)):
			s[i+1] = "%c" % self._shifts[ord(data[i])]
		
		out = ''
		for i in range(len(s)):
			out += s[i]
		
		return out
	
	def unittest():
		t = CvsScramble()
		print "CvsScramble 1: " + t.realTransform("hello world")
	unittest = staticmethod(unittest)


class Md5(Transformer):
	'''
	MD5 transform (hex and binary)
	'''
	
	_asHex = 0
	
	def __init__(self, asHex = 0):
		'''
		@type	asHex: number
		@param	asHex: 1 is hex, 0 is binary
		'''
		self._asHex = asHex
	
	def realTransform(self, data):
		if self._asHex == 0:
			return md5.new(data).digest()
		return md5.new(data).hexdigest()
	
	def unittest():
		t = Md5(1)
		print "Md5 1: " + t.realTransform("hello world")
	unittest = staticmethod(unittest)


class Sha1(Transformer):
	'''
	SHA-1 transform (hex and binary)
	'''
	
	_asHex = 0
	
	def __init__(self, asHex = 0):
		'''
		@type	asHex: number
		@param	asHex: 1 is hex, 0 is binary
		'''
		self._asHex = asHex
	
	def realTransform(self, data):
		if self._asHex == 0:
			return sha.new(data).digest()
		return sha.new(data).hexdigest()
	
	def unittest():
		t = Sha1(1)
		print "Sha1 1: " + t.realTransform("hello world")
	unittest = staticmethod(unittest)


class Hmac(Transformer):
	'''
	HMAC as described in RFC 2104.  Key is a generator.
	'''
	
	_key = None
	_digestmod = None
	_asHex = None
	
	def __init__(self, key, digestmod = md5, asHex = 0):
		'''
		Key is a generator for HMAC key, digestmod is hash to use (md5 or sha)
		
		@type	key: Generator
		@param	key: HMAC key
		@type	digestmod: md5 or sha
		@param	digestmod: Which digest to use
		@type	asHex: number
		@param	asHex: 1 is hex, 0 is binary
		'''
		self._key = key
		self._digestmod = digestmod
		self._asHex = asHex
	
	def realTransform(self, data):
		if self._asHex == 0:
			return hmac.new(self._key.getValue(), data, self._digestmod).digest()
		return hmac.new(self._key.getValue(), data, self._digestmod).hexdigest()
	
	def unittest():
		t = Hmac(static.Static('hello world'), md5, 1)
		print "Hmac (md5) 1: " + t.realTransform("hello world")
		t = Hmac(static.Static('hello world'), sha, 1)
		print "Hmac (sha1) 1: " + t.realTransform("hello world")
	unittest = staticmethod(unittest)


def unittest():
	Crypt.unittest()
	UnixMd5Crypt.unittest()
	ApacheMd5Crypt.unittest()
	CvsScramble.unittest()
	Md5.unittest()
	Sha1.unittest()
	Hmac.unittest()


# end
