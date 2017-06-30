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
	ftp.py
	
	Specific module with the functions necessaries in the FTP protocol fuzzing
'''

import socket,sys,logging,time,os

from string import atoi
from Utils.misc import checkIpOctet

# Global socket object
sck = None
 
 
def port(command = "", response = "", logMode = 0, verboseMode = 0, endLine = '\n', xmlFiles = []):
	'''
	This function opens a port in the host in order to interchange data with the ftp server
	
	@arg command: command sent to the remote host
	@type command: string
	
	@arg response: response sent by the remote host
	@type response: string
	
	@arg logMode: if a logging is needed
	@type logMode: integer
	
	@arg verboseMode: if the verbose mode is activated
	@type verboseMode: integer
	
	@arg endLine: characters of end of line
	@type endLine: string
	
	@arg xmlFiles: names of configuration files
	@type xmlFiles: list
	'''
	
	global sck
	dataList = []
	buffer = ""
	if logMode:
		logFile = logging.getLogger("FTP.port")
	
	# Argument needed by the method: 192,168,0,45,123,34
	host_port = command[5:].split(endLine)[0]
	list = host_port.split(",")
	# Checking if each part of the argument is ok
	if checkIpOctet(list[0]) and checkIpOctet(list[1]) and checkIpOctet(list[2]) and checkIpOctet(list[3]):
		if checkIpOctet(list[4]) and checkIpOctet(list[5]):
			# Host address
			host = list[0]+"."+list[1]+"."+list[2]+"."+list[3]
			# Host port 
			port = atoi(list[4])*256+atoi(list[5])
			try:
				if sck != None:
					sck.close()
				sck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
				sck.bind((host,port))
				# Listening for connections
				sck.listen(5)
				sck.settimeout(5)
				
			except socket.error,e:
				if logMode:
					logFile.error(str(e))
				sck.close()
				
			except:
				raise


def receive(command = "", response = "", logMode = 0, verboseMode = 0, endLine = '\n', xmlFiles = []):
	'''
	With that function we can receive data from the ftp server in the port specified before
	
	@arg command: command sent to the remote host
	@type command: string
	
	@arg response: response sent by the remote host
	@type response: string
	
	@arg logMode: if a logging is needed
	@type logMode: integer
	
	@arg verboseMode: if the verbose mode is activated
	@type verboseMode: integer
	
	@arg endLine: characters of end of line
	@type endLine: string
	
	@arg xmlFiles: names of configuration files
	@type xmlFiles: list
	'''
	
	global sck
	sckData = None
	if logMode:
		logFile = logging.getLogger("FTP.receive")
	try:
		if sck != None:
			# Waiting for connections
			(sckData, address) = sck.accept()
			if logMode:
				logFile.debug("Receiving data...")
			sckData.settimeout(5)
			data = sckData.recv(1024)
			if data != "":
				if verboseMode:
					print "------"
					print "|DATA|"
					print "------"
					print data
				if logMode:
					logFile.info("\n------\n|DATA|\n------\n"+data)
					
	except socket.error,e:
		if logMode:
			logFile.error(str(e))
		if sckData:
			sckData.close()
		if sck:
			sck.close()
			
	except:
		raise
			
			
def wait(command = "", response = "", logMode = 0, verboseMode = 0, endLine = '\n', xmlFiles = []):
	'''
	Method to force the application to wait some time before continuing
	
	@arg command: command sent to the remote host
	@type command: string
	
	@arg response: response sent by the remote host
	@type response: string
	
	@arg logMode: if a logging is needed
	@type logMode: integer
	
	@arg verboseMode: if the verbose mode is activated
	@type verboseMode: integer
	
	@arg endLine: characters of end of line
	@type endLine: string
	
	@arg xmlFiles: names of configuration files
	@type xmlFiles: list
	'''	
	
	time.sleep(2)
	
