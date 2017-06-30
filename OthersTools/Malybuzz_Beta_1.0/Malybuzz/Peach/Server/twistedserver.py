'''
Peach testing server.  This is an XML-RPC server that stored test run
information in a MySQL database using the schema found in 'server.sql'.
The client portion of the client server can be found in 'client.py' (go
figure).

The server portion requires the twisted python module to be installed.

@author: Michael Eddington
@version: $Id: twistedserver.py 279 2007-05-08 01:21:58Z meddingt $
'''

#
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

# $Id: twistedserver.py 279 2007-05-08 01:21:58Z meddingt $

import sys, string, time, os, base64, MySQLdb

from twisted.internet import reactor
from twisted.web import xmlrpc, server
from twisted.enterprise import util

class PeachServer(xmlrpc.XMLRPC):
	
	def xmlrpc_RunStarting(self, runId, client):
		'''
		Indicate start of test run.
		'''
		sys.stdout.write("+")
		cursor.execute("insert into testrun (runid, client, starttime) values (%s, %s, NOW())", 
			(runId, client))
		dbconn.commit()
		
		return 'OK'
	
	def xmlrpc_RunFinished(self, runId):
		'''
		Indicate test run has completed.
		'''
		sys.stdout.write("-")
		cursor.execute("update testrun set finished = 1, finishtime = NOW() where runid = %s", 
			(runId))
		dbconn.commit()
		
		return 'OK'
	
	def xmlrpc_StartTest(self, runId, testName, testNum, testData):
		'''
		Signal start of test.  All parameters are strings.
		'''
		
		sys.stdout.write(".")
		testData = base64.decodestring(testData)
		cursor.execute("insert into test (runid, testnum, testname, testdata, starttime) values (%s, %s, %s, %s, NOW())", 
			(runId, int(testNum), testName, testData))
		dbconn.commit()
		
		return 'OK'
	
	def xmlrpc_EndTest(self, runId, testName, testNum, result):
		'''
		Signal end of test.  All parameters are strings.
		'''
		
		sys.stdout.write("o")
		result = base64.decodestring(result)
		cursor.execute("update test set testresult = %s, finishtime = NOW() where runid = %s and testname = %s and testnum = %s", 
			(result, runId, testName, int(testNum)))
		
		return 'OK'


print ""
print "] Twisted Peach Test Server v0.3"
print "] Copyright (c) 2006-2007 Michael Eddington\n"

if len(sys.argv) < 5:
	print "Syntax:"
	print "  python twistedserver.py <PORT> <DB HOST> <DB NAME> <DB USER> <DB PASS>\n"
	sys.exit(0)

dbconn = MySQLdb.connect(host=sys.argv[2], db=sys.argv[3], user=sys.argv[4], passwd=sys.argv[5])
cursor = dbconn.cursor()

print "Starting on port %s\n" % sys.argv[1]

r = PeachServer()
reactor.listenTCP(string.atoi(sys.argv[1]), server.Site(r))
reactor.run()

# end
