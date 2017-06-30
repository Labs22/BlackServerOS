
'''
Some default compression transforms (gzip, compress, etc).

@author: Michael Eddington
@version: $Id: compress.py 279 2007-05-08 01:21:58Z meddingt $
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

# $Id: compress.py 279 2007-05-08 01:21:58Z meddingt $


import zlib, bz2
from Peach.transformer import Transformer

#__all__ = ['GzipCompress', 'GzipDecompress', 'Bz2Compress', 'Bz2Decompress']

class GzipCompress(Transformer):
	'''
	Gzip compression transform.  Also allows for compression level 
	selection (default is 6).
	'''
	
	_level = None
	
	def __init__(self, level = 6):
		'''
		@type	level: number
		@param	level: level is an integer from 1 to 9 controlling the level
		of compression; 1 is fastest and produces the least compression, 9
		is slowest and produces the most. The default value is 6.
		'''
		self._level = level
	
	def realTransform(self, data):
		return zlib.compress(data, self._level)
	
	def unittest():
		e = GzipDecompress()
		t = GzipCompress()
		print "GzipCompress 1: " + t.realTransform("Hello World!")
		print "GzipCompress 2: " + e.realTransform(t.realTransform("Hello World!"))
	unittest = staticmethod(unittest)


class GzipDecompress(Transformer):
	'''
	Gzip decompression transform.
	'''
	
	_wbits = None
	
	def __init__(self, wbits = 15):
		'''
		@type	wbits: number
		@param	wbits: The absolute value of wbits is the base two logarithm
		of the size of the history buffer (the ``window size'') used when
		compressing data. Its absolute value should be between 8 and 15 for
		the most recent versions of the zlib library, larger values resulting
		in better compression at the expense of greater memory usage. The
		default value is 15. When wbits is negative, the standard gzip
		header is suppressed; this is an undocumented feature of the zlib
		library, used for compatibility with unzip's compression file format.
		'''
		self._wbits = wbits
	
	def realTransform(self, data):
		return zlib.decompress(data, self._wbits)
	
	def unittest():
		e = GzipDecompress()
		t = GzipCompress()
		print "GzipDecompress: " + e.realTransform(t.realTransform("Hello World!"))
	unittest = staticmethod(unittest)


class Bz2Compress(Transformer):
	'''
	bzip2 compression transform.  Also allows for compression level 
	selection (default is 9).
	'''
	
	_level = None
	
	def __init__(self, level = 9):
		'''
		@type	level: number
		@param	level: The compresslevel parameter, if given, must be a number
		between 1 and 9; the default is 9.
		'''
		self._level = level
	
	def realTransform(self, data):
		return bz2.compress(data, self._level)
	
	def unittest():
		e = Bz2Decompress()
		t = Bz2Compress()
		print "Bz2Compress 1: " + t.realTransform("Hello World!")
		print "Bz2Compress 2: " + e.realTransform(t.realTransform("Hello World!"))
	unittest = staticmethod(unittest)


class Bz2Decompress(Transformer):
	'''
	bzip2 decompression transform.
	'''
	
	def realTransform(self, data):
		return bz2.decompress(data)
	
	def unittest():
		e = Bz2Decompress()
		t = Bz2Compress()
		print "Bz2Decompress: " + e.realTransform(t.realTransform("Hello World!"))
	unittest = staticmethod(unittest)


def unittest():
	GzipCompress.unittest()
	GzipDecompress.unittest()
	Bz2Compress.unittest()
	Bz2Decompress.unittest()


# end
