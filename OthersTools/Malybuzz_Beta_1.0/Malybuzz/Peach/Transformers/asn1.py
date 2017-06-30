
'''
ASN.1 transformers.  These transformers perform correct ASN.1 encoding.  The
data Generators module contains a couple of additional ASN.1 classes
that perform incorrect encodings.

@author: Michael Eddington
@version: $Id$
'''

#
# Copyright (c) 2007 Michael Eddington
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

# $Id$

try:
	from struct import *
	from Peach.transformer import Transformer
	from pyasn1.type import univ
	from pyasn1.codec import der, ber
	
	class DerEncodeOctetString(Transformer):
		'''
		DER encode a string ASN.1 style
		'''
		
		def realTransform(self, data):
			return der.encoder.encode(univ.OctetString(data))
	
	
	class DerEncodeInteger(Transformer):
		'''
		DER encode an integer ASN.1 style
		'''
		
		def realTransform(self, data):
			return der.encoder.encode(univ.Integer(int(data)))
	
	
	class BerEncodeOctetString(Transformer):
		'''
		BER encode a string ASN.1 style
		'''
		
		def realTransform(self, data):
			return ber.encoder.encode(univ.OctetString(data))
	
	
	class BerEncodeInteger(Transformer):
		'''
		BER encode an integer ASN.1 style
		'''
		
		def realTransform(self, data):
			return ber.encoder.encode(univ.Integer(int(data)))
except:
	pass


# end
