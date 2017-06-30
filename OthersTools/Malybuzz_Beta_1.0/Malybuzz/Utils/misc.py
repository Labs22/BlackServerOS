#
# Malybuzz
# By Jose Miguel Esparza <josemiguel.esparza@gmail.com>
#											   <jesparza@s21sec.com>
# Copyright 2007 Jose Miguel Esparza
#
# This file is part of Malybuzz.
#
#    Malybuzz is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 3 of the License, or
#    (at your option) any later version.
#
#    Malybuzz is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

'''
	misc.y
	
	Module with miscellaneous implementations
'''

import sys,socket,traceback

from datetime import datetime
from string import atoi

def timeTest(host, port, transport):
	'''
	Method to check the response time of the target host
	
	@arg host: host to check
	@type: string
	
	@arg port: port where will be sent the packets
	@type: integer
	
	@arg transport: connection transport protocol
	@type: string
	
	@return: the time or -1 if error (no socket error)
	@rtype: integer
	'''
	
	t = 0
	
	try:
		for i in range(0,5):
			if transport == 'TCP':
				sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			elif transport == 'UDP':
				sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
			t1 = datetime.now().microsecond
			sock.connect((host,port))
			t2 = datetime.now().microsecond
			t = t + (t2 - t1)
			sock.close()
		return (t/5/1000.0)
	
	except socket.error,e:
		if sock != None:
			sock.close()
		sys.exit("Socket Error: \""+str(e[1])+"\" while time testing the remote host.")
			
	except: 
		return -1
	
	
def error(logMode, logFile, errorMessage, debugMessage = None):
	'''
		Method to manage the errors
		
		@arg logMode: parameter to say if we're in log mode or not
		@type logMode: integer
		
		@arg logFile: the object which manage the logging session
		@type: Logger
		
		@arg errorMessage: the error message
		@type: string
		
		@arg debugMessage: the debugging information of the error
		@type: string
	'''
	
	if logMode:
		if logFile != None:
			logFile.error(errorMessage)
			if debugMessage != None:
				logFile.debug(debugMessage)
	sys.exit(errorMessage)
	
	
def tabs(element, maxLen):
	'''
		Method to give the number of tabs necessary to print in the screen correctly
		
		@arg element: element to align 
		@type element: string
		
		@arg maxLen: limit to add tabs
		@type maxLen: integer
		
		@return: tabs necessaries to align the element correctly
		@rtype: string
	'''
	
	tabSpace = 8
	numTabs = 0
	maxTabs = maxLen/tabSpace
	
	if len(element) > maxLen:
		numTabs = 1
	else:
		numTabs = ((maxLen - len(element))/tabSpace)+1
	if numTabs > maxTabs:
		numTabs = maxTabs
	return numTabs*'\t'


def endFormat(end):
	'''
		Converts the letter way of end of line into control character form
		
		@arg end: end of line in letter form
		@type end: string
		
		@return: end of line in with control characters
		@rtype: string
	'''
	
	if end == 'CRLF':
		return "\r\n"
	elif end == 'CR':
		return "\r"
	elif end == 'LF':
		return "\n"
	
	
def inputConverter(input, endLine):
	'''
		Converts the space and end of line characters of input in a special string form
		
		@arg input: input data
		@type input: string
		
		@arg endLine: characters of end of line
		@type endLine: string
		
		@return: converted data
		@rtype: string
	'''
	
	inputConverted = input.replace(' ','##space##')
	inputConverted = inputConverted.replace(endLine,'##intro##')
	return inputConverted


def outputConverter(output, endLine):
	'''
		Converts the space and end of line characters from special strings to normal form
		
		@arg output: input data
		@type output: string
		
		@arg endLine: characters of end of line
		@type endLine: string
		
		@return: converted data
		@rtype: string
	'''
	
	outputConverted = output.replace('##space##',' ')
	outputConverted = outputConverted.replace('##intro##',endLine)
	return outputConverted


def checkIpOctet(octet):
	'''
		Checks if the value of an octet is correct
		
		@arg octet: octet string
		@type octet: string
		
		@return: if its correct or not
		@rtype: boolean
	'''
	
	if octet.isdigit():
		if int(octet) > 0 and int(octet) < 256:
			return True
	return False


###########################################################
#		XML Functions	Variables																			#
###########################################################

def size(arguments):
	'''
		Method to obtain the length of an xml element
		
		@arg arguments: a list with a unique value indicating the element
		@type arguments: list
		
		@return: the length of the element
		@rtype: string
	'''
	string = arguments[0]
	return str(len(string))
