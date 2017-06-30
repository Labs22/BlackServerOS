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
	sip.py
	
	Specific module with the functions necessaries in the SIP protocol fuzzing
'''

import sys,logging,md5,time

from xml.dom import minidom
from Utils.misc import error


def answerChallenge(command = "", response = "", logMode = 0, verboseMode = 0, endLine = '\n', xmlFiles = []):
  '''
  This function answers to the challenge sent by the other side to authenticate the sip session.
  It's only a simple beta implementation in the case that qop="auth" and algorithm="md5".
	
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
	
	@return: command with response to challenge
	@rtype: string
  '''
  
  if logMode:
    logFile = logging.getLogger("SIP.answerChallenge")
  xmlCommandsFile = minidom.parse(xmlFiles[0])
  commandLines = command.split(endLine)
  responseLines = response.split(endLine)
  qop = None
  algorithm = None
  realm = None
  nonce = None
  method = None
  uri = None
  username = None
  password = None
  
  for i in responseLines:
    # If we need to authenticate...
    if i.find("WWW-Authenticate") != -1:
      # Getting the necessary data in the response (get more elements to have a good implementation)
      qop = getParameterValue(i, "qop")
      algorithm = getParameterValue(i, "algorithm")
      #Only implementing this case
      if (qop == None or qop == "auth" or qop.find("auth,") != -1) and (algorithm == None or algorithm == "md5"):
        # Getting all parameters necessaries to generate the response
        realm = getParameterValue(i, "realm")
        nonce = getParameterValue(i, "nonce")
        if realm == None:
          error(logMode, logFile, "Authentication Error: \"realm\" not present in response.")
        if nonce == None:
          error(logMode, logFile, " Authentication Error: \"nonce\" not present in response.")
        
        requestLine = commandLines[0].split()
        if len(requestLine) > 1:
          method = requestLine[0]
          uri = requestLine[1]
        else:
          error(logMode, logFile, "Authentication Error: Malformed Request-line.")
        for j in range(len(commandLines)):
          if commandLines[j].find("Authorization") != -1:
            username = getParameterValue(commandLines[j], "username")
            if username == None:
            	error(logMode, logFile, "Authentication Error: \"username\" empty in Authorization line.", i)
            break
        else:
        	error(logMode, logFile, "Authentication Error: Username must be present in Authorization line.",j)
        
        password = xmlCommandsFile.getElementsByTagName('password')
        if password != []:
          password = password[0].lastChild.nodeValue
        else:
          error(logMode, logFile, "Authentication Error: \"password\" must be present in Authorization line.")
        # Generating challenge response
        a1 = username+":"+realm+":"+password
        a2 = method+":"+uri
        hashA1 = md5.new(a1).hexdigest()
        hashA2 = md5.new(a2).hexdigest()
        challengeResponse = md5.new(hashA1+":"+nonce+":"+hashA2).hexdigest()
        # Generating Authorization line
        authLine = 'Authorization: Digest username="'+username+'",realm="'+realm
        authLine += '",nonce="'+nonce+'",response="'+challengeResponse+'"'
        # Generating command
        command = ""
        commandLines[j] = authLine
        for j in commandLines:
          command += j+"\r\n"
        command += "\r\n"
        return command
      if logMode:
        logFile.warning("Authentication method or algorithm not implemented yet")
      sys.exit("Authentication method or algorithm not implemented yet")
      

def getParameterValue(response, parameter):
  '''
  Function to obtain the value of a given parameter from a line response 
	
	@arg response: response sent by the remote host
	@type response: string

	@arg parameter: parameter name
	@type parameter: string
	
	@return: parameter value
	@rtype: string
  '''
  
  parameter = response.split(parameter+'="')
  if len(parameter) > 1:
    parameter = parameter[1].split('"')[0]
    return parameter
  else:
    return None
  
  
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
	
	time.sleep(5)