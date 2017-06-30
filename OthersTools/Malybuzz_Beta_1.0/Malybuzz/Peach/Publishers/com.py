
'''
Windows COM/DCOM/COM+ publishers.

@author: Michael Eddington
@version: $Id: com.py 279 2007-05-08 01:21:58Z meddingt $
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

# $Id: com.py 279 2007-05-08 01:21:58Z meddingt $

try:
	import time, sys, pywintypes, signal, os
	import win32com.client
	import win32com.client.gencache
except:
	pass
from Peach.publisher import Publisher

#__all__ = ['Com']

class Com(Publisher):
	"""
	Very simple Com publisher that allows for a single method call.  The
	method call is fed in as a string which is evaled.  This allows for
	calling any method with any number of parameters.
	
	The simple usage would be to call with a methodFormat that has only a
	single parameter such as StringTest('''%s''').  A more complex call
	could use multiple data items such as:
	
	StringTest('''%s''', '''%s''', '''%s''')
	
	The data passed into send() should be a tuple in this case.
	
	"""
	_clsid = None
	_methodFormat = None
	_lastReturn = None
	_object = None
	
	def __init__(self, clsid, methodFormat):
		"""
		Create Com Object. clsid = '{...}', methodFormat is method name and variable placement
		for example: "StringTest('''%s''')"
		
		@type	clsid: string
		@param	clsid: CLSID of COM object in {...} format
		@type	methodFormat: string
		@param	methodFormat: Method call with data placement
		"""
		
		self._clsid = clsid
		self._methodFormat = "self._object." + methodFormat
	
	def start(self):
		try:
			self._object = None
			self._object = win32com.client.DispatchEx(self._clsid)
			
		except pywintypes.com_error, e:
			print "Caught pywintypes.com_error creating ActiveX control [%s]" % e
			raise
			
		except:
			print "Caught unkown exception creating ActiveX control [%s]" % sys.exc_info()[0]
			raise
	
	def stop(self):
		self._object = None
	
	def send(self, data):
		'''
		Call method on COM object.
		
		@type	data: string or tuple
		@param	data: Data to use, can be string or list of strings
		'''
		
		self._lastReturn = None
		
		try:
			self._lastReturn = eval(self._methodFormat % data)
			
		except pywintypes.com_error, e:
			print "Caught pywintypes.com_error on call [%s]" % e
			raise
		
		except NameError, e:
			print "Caught NameError on call [%s]" % e
			raise
		
		except:
			print "Com::send(): Caught unknown exception"
			raise
		
	def receive(self):
		return self._lastReturn

def unittest():
	pass
	
# end

