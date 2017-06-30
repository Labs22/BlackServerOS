
'''
Peach is a fuzzing framework written in Python.

Please see the included README file and the samples folder for more
information.

@author: Michael Eddington
@version: $Id: __init__.py 286 2007-05-08 03:26:57Z meddingt $
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

# $Id: __init__.py 286 2007-05-08 03:26:57Z meddingt $

import generator
import group
import protocol
import publisher
import transformer
import script
import Generators
import Protocols
import Publishers
import Transformers

__all__ = ["generator", "group", "protocol", "publisher", "transformer", "script",
		   "Generators", "Protocols", "Publishers", "Transformers"]

def unittest():
	generator.unittest()
	group.unittest()
	protocol.unittest()
	publisher.unittest()
	script.unittest()
	transformer.unittest()
	
	Generators.unittest()
	Protocols.unittest()
	Publishers.unittest()
	Transformers.unittest()


if __name__ == "__main__":
	unittest()

# end
