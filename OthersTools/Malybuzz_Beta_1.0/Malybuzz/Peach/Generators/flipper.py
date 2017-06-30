
'''
Default flippers.  Flippers are used to flip bits in a data.  This
is used for "random" or "bute force" fuzzing.  Usefull on codecs and
fully unknown protocols.  Runtime is long for flippers.

Flippers can be "stacked" so to speak to make for interesting random
flipping.  To stack, just use one Flipper as the data generator for
another flipper.

@author: Michael Eddington
@version: $Id: flipper.py 279 2007-05-08 01:21:58Z meddingt $
'''

#
# Copyright (c) 2005-2006 Michael Eddington
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

# $Id: flipper.py 279 2007-05-08 01:21:58Z meddingt $


import re, struct
from Peach import generator

#__all__ = ["SequentialFlipper"]

class SequentialFlipper(generator.Generator):
	'''
	Sequencially flipps bits in a data blob.  This is for
	"random" fuzzing.  Usefull brute forcing, codecs, etc.
	
	TODO: Make take a generator in, make sure we work
	with sequences of data from the generator (e.g. flip
	set one, then set 2, etc).
	'''
	
	_data = None
	_position = 0
	
	def __init__(self, data):
		'''
		@type	data: string
		@param	data: Binary static blob to flip
		'''
		self._data = data;
		raise Exception, "Not fully implemented"
	
	def next():
		_position += 1
	
	def reset():
		_position = 0
	
	def getRawValue(self):
		data = self._data;
	
	def unittest():
		None
	unittest = staticmethod(unittest)


def unittest():
	SequentialFlipper.unittest()

# end
