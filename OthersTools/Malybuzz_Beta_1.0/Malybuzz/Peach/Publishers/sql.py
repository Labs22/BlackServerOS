
'''
SQL publisher objects.  Includes a default ODBC publisher.

@author: Michael Eddington
@version: $Id: sql.py 279 2007-05-08 01:21:58Z meddingt $
'''

#
# Copyright (c) 2005 Michael Eddington
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

# $Id: sql.py 279 2007-05-08 01:21:58Z meddingt $

from types import *

try:
	import dbi, odbc
except:
	pass

from Peach.publisher import Publisher

#__all__ = ["Odbc"]

class Odbc(Publisher):
	'''
	Publisher for ODBC connections.  Generated data sent as a SQL query via
	execute.  Calling receave will return a string of all row data concatenated
	together with \t as field separator.
	
	Currently this Publisher makes use of the odbc package which is some what
	broken in that you must create an actual ODBC DSN via the ODBC manager.  
	Check out mxODBC which is not open source for another alterative.
	
	Note:  Each call to start/stop will create and close the SQL connection and
	cursor.
	'''
	
	_dsn = None
	_sql = None
	_cursor = None
	_sql = None
	
	def __init__(self, dsn):
		'''
		@type	dsn: string
		@param	dsn: DSN must be in format of "dsn/user/password" where DSN is a DSN name.
		'''
		self._dsn = dsn
	
	def getDsn(self):
		'''
		Get current DSN.
		
		@rtype: string
		@return: current DSN
		'''
		return self._dsn
	def setDsn(self, dsn):
		'''
		Set new DSN.
		
		@type	dsn: string
		@param	dsn: New DSN
		'''
		self._dsn = dsn
	
	def start(self):
		'''
		Create connection to server.
		'''
		self._sql = odbc.odbc(self._dsn)
	
	def stop(self):
		'''
		Close any open cursors, and close connection to server.
		'''
		self._cursor.close()
		self._cursor = None
		self._sql.close()
		self._sql = None
	
	def send(self, data):
		'''
		Create cursor and execute data.
		'''
		self._cursor = self._sql.cursor()
		self._cursor.execute(data)
	
	def receive(self):
		'''
		Returns a single row of data with \t as column separator.
		'''
		row = self._cursor.fetchone()
		ret = ''
		for i in range(len(row)):
			retType = type(row[i])
			if retType is IntType:
				ret += "\t%d" % row[i]
			elif retType is FloatType:
				ret += "\t%f" % row[i]
			elif retType is LongType:
				ret += "\t%d" % row[i]
			elif retType is StringType:
				ret += "\t%s" % row[i]
		
		return ret

def unittest():
	pass

# end

