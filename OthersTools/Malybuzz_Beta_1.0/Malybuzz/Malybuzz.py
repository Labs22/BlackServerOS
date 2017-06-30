#! /usr/bin/env python

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
	Malybuzz.py
	
	Main script file to begin the fuzzing actions: commands generation, targets fuzzing...
'''

import sys,logging,os,traceback,shelve,copy,random

from datetime import datetime
from string import atoi
from xml.dom import minidom

from Communication.communication import CommunicationManager
from Core.core import ProtocolManager, CommandsList
from ProtocolParser.protocolParser import ProtocolExtractor
from Utils.misc import error, tabs
from Utils.session import Session


########################################################################
# Initializing parameters
########################################################################

usage = "Usage: Malybuzz.py <target_host> <'tcp'|'udp'> <protocol> [options]\n"+\
				"Usage: Malybuzz.py <target_host> <'tcp'|'udp'> <protocol> -g <commands_xml_file> <output_file>\n"+\
				"Usage: Malybuzz.py -z <fuzzing_session_file> [options]\n"+\
				"Usage: Malybuzz.py -h\n\n"+\
				"\tOPTIONS\n\n"+\
				"\t-h : print this help.\n"+\
				"\t-v : set the verbose mode.\n"+\
				"\t-k : keep the fuzzing state when session is aborted,so it will be possible to resume it. The session\n"+\
				"\t     file is stored in the Restore folder.\n"+\
				"\t-b : set it if the protocol to fuzz returns a banner.\n"+\
				"\t-p <remote_port> : set the remote port.\n"+\
				"\t-s <source_port> : set the local port.\n"+\
				"\t-t1 <response_timeout> : set the response timeout in milliseconds after being sent a request. By default 100.000 ms.\n"+\
										"\t\t\t      If 0 no timeout.\n"+\
				"\t-t2 <ping_timeout> : set the timeout in milliseconds after being sent a ping probe. By default 2 ms.\n"+\
				"\t-t3 <between_time_requests> : set the time in milliseconds between fuzzing requests. By default 500 ms.\n"+\
				"\t-c <max_timeouts> : continue on timeout and set the maximum number of timeouts. If 0 no limit.\n"+\
				"\t-f <first_command> : set the first command to send in the specified protocol.\n"+\
				"\t-xc <xml_file> : set the commands configuration file we want to use to do the fuzzing. No sense with -z.\n"+\
				"\t-xr <xml_file> : set the responses configuration file we want to use to do the fuzzing. No sense with -z.\n"+\
				"\t-o <commands_file> : open a commands file and load it in the actual fuzzing session. It must be in the\n"+\
										"\t\t\t   'Restore/Commands' folder. No sense with -z.\n"+\
				"\t-l 1|2|3 : log the fuzzing session in the 'Logs' folder with the specified level.\n"\
											"\t\t1: Critical,error and warning messages.\n"\
											"\t\t2: Level 1 plus info messages.\n"\
											"\t\t3: Level 2 plus debug messages.\n"+\
				"\t-z <fuzzing_session_file> : resume a fuzzing session from a session file.\n"+\
				"\t-g <xml_file> <output_file> : generate the commands we want to send extracted from a xml file and save them\n"+\
													"\t\t\t\t in the Restore folder. No sense with -z.\n"

# Variables which define the session parameters
targetPort = None
sourcePort = "0"
responseTimeout = None
pingTimeout = None
betweenTime = None
maxNumberTimeouts = None
firstCommand = ""
verboseMode = 0
logMode = 0
logFile = None
logLevel = None
connection = None
banner = 0
# Necessary variables to resume or store the fuzzing session 
sessionMode = 0
savingMode = 0 
fuzzingSession = None
fuzzingSessionName = None
fuzzingSummary = None
resumingMode = 0
# Variables to manage the commands
generatingCommands = 0
loadingCommands = 0
commandsFileName = None
# Variables to store the configuration files names
xmlCommandsFile = None
xmlResponsesFile = None
xmlVariables = {}
# Temporal file to store the commands
tmpFileName = "commands"+str(random.random())
# Variable to store the network protocol: commands,responses,timeouts...
protocol = None
# Number of total commands
numCommands = 0
# Variable to define where the optional parameters begin
argCounter = 4




########################################################################
# Checking arguments and configuring session
########################################################################

# Check if the arguments number is ok
if len(sys.argv) < 4:
	if len(sys.argv) < 3 or sys.argv[1] != "-z":
		# The arguments number is not correct and not resuming session
		sys.exit(usage)
	else:
		fuzzingSessionName = sys.argv[2]
		if os.path.exists(fuzzingSessionName):
			sessionMode = 1
			resumingMode = 1
		else:
			sys.exit("File Error: The session file '"+fuzzingSessionName+"' does not exist."+"\n\n"+usage)
else:
	if sys.argv[1] != '-z':
		# We neither want to resume a session nor generate commands. Initializing more parameters.
		targetHost = sys.argv[1]
		# Transport layer protocol
		transportProto = sys.argv[2].upper()
		if transportProto != 'TCP' and transportProto != 'UDP':
			sys.exit("Syntax Error: Second argument must be 'tcp' or 'udp'."+"\n\n"+usage)
		# Application layer protocol
		applicationProto = sys.argv[3]	
		# Default configurations files
		xmlCommandsFile = "Protocols/Specifications/"+transportProto+"/"+applicationProto+"/"+applicationProto+".xml"
		xmlResponsesFile = "Protocols/Specifications/"+transportProto+"/"+applicationProto+"/"+applicationProto+"_states.xml"
		if sys.argv[4] == '-g':
			# We want to generate commands
			generatingCommands = 1
			# Log mode with level 3 activated by default
			logMode = 1
			logLevel = str(3)
			xmlCommandsFile = sys.argv[5]
		if not os.path.exists(xmlCommandsFile):
			sys.exit("File Error: The commands xml file '"+xmlCommandsFile+"' does not exist.")
		if not os.path.exists(xmlResponsesFile):
			sys.exit("File Error: The responses xml file '"+xmlResponsesFile+"' does not exist.")
	else:
		# We want to resume a session. Mandatory arguments (host,trasnport and application protocol) are not necessary in this case
		argCounter = 1
		fuzzingSessionName = sys.argv[argCounter+1]
		if os.path.exists(fuzzingSessionName):
			sessionMode = 1
			resumingMode = 1
			argCounter += 2
		else:
			sys.exit("File Error: The session file '"+fuzzingSessionName+"' does not exist."+"\n\n"+usage)

try:
	while argCounter<len(sys.argv) and not generatingCommands:
		if sys.argv[argCounter] == "-h":
			sys.exit(usage)
		elif sys.argv[argCounter] == "-v":
			verboseMode = 1
			argCounter += 1
		elif sys.argv[argCounter] == "-k":
			sessionMode = 1
			savingMode = 1
			argCounter += 1
		elif sys.argv[argCounter] == "-b":
			banner = 1
			argCounter += 1
		elif sys.argv[argCounter] == "-l":
			logMode = 1
			logLevel = sys.argv[argCounter+1]
			if logLevel.isdigit():
				argCounter += 2
			else:
				sys.exit("Syntax Error: The log level must be an integer between 1 and 3.\n\n"+usage)
		elif sys.argv[argCounter] == "-s":
			sourcePort = sys.argv[argCounter+1]
			if sourcePort.isdigit() and int(sourcePort) > 0 and int(sourcePort) < 65536:
				argCounter += 2
			else:
				sys.exit("Syntax Error: The source port must be an integer between 1 and 65535."+"\n\n"+usage)
		elif sys.argv[argCounter] == "-p":
			targetPort = sys.argv[argCounter+1]
			if targetPort.isdigit() and int(targetPort) > 0 and int(targetPort) < 65536:
				argCounter += 2
			else:
				sys.exit("Syntax Error: The remote port must be an integer between 1 and 65535."+"\n\n"+usage)
		elif sys.argv[argCounter] == "-t1":	
			if sys.argv[argCounter+1].isdigit():
				responseTimeout = int(sys.argv[argCounter+1])
				argCounter += 2
			else:
				sys.exit("Syntax Error: The response timeout must be an integer."+"\n\n"+usage)
		elif sys.argv[argCounter] == "-t2":
			if sys.argv[argCounter+1].isdigit():
				pingTimeout = int(sys.argv[argCounter+1])
				argCounter += 2
			else:
				sys.exit("Syntax Error: The ping timeout must be an integer."+"\n\n"+usage)
		elif sys.argv[argCounter] == "-t3":
			if sys.argv[argCounter+1].isdigit():
				betweenTime = int(sys.argv[argCounter+1])
				argCounter += 2
			else:
				sys.exit("Syntax Error: The time between requests must be an integer."+"\n\n"+usage)
		elif sys.argv[argCounter] == "-f":
			firstCommand = sys.argv[argCounter+1]
			argCounter += 2
		elif sys.argv[argCounter] == "-c":
			if sys.argv[argCounter+1].isdigit():
				maxNumberTimeouts = int(sys.argv[argCounter+1])
				argCounter += 2
			else:
				sys.exit("Syntax Error: The maximum number of timeouts must be an integer."+"\n\n"+usage)
		elif sys.argv[argCounter] == "-xc":
			xmlCommandsFile = sys.argv[argCounter+1]
			if os.path.exists(xmlCommandsFile):
				argCounter += 2
			else:
				sys.exit("File Error: The configuration file '"+xmlCommandsFile+"' does not exist."+"\n\n"+usage)
		elif sys.argv[argCounter] == "-xr":
			xmlResponsesFile = sys.argv[argCounter+1]
			if os.path.exists(xmlResponsesFile):
				argCounter += 2
			else:
				sys.exit("File Error: The configuration file '"+xmlResponsesFile+"' does not exist."+"\n\n"+usage)
		elif sys.argv[argCounter] == "-o":
			commandsFileName = sys.argv[argCounter+1]
			if os.path.exists(commandsFileName):
				loadingCommands = 1
				argCounter += 2
			else:
				sys.exit("File Error: The commands file '"+commandsFileName+"' does not exist."+"\n\n"+usage)
		else:
			# Option is not correct
			sys.exit("Syntax Error: Syntax not correct near argument '"+sys.argv[argCounter]+"'\n\n"+usage)	

except SystemExit,e:
	sys.exit(e)

except:
	sys.exit("Syntax Error: Syntax not correct near argument '"+sys.argv[argCounter]+"'\n\n"+usage)


try:
	if resumingMode:
		# Resuming an existing session
		file = shelve.open(fuzzingSessionName)
		fuzzingSession = file['fuzzingSession']
		file.close()
		# Getting the necessary parameters to other functions 
		extractorOptions = fuzzingSession.getExtractorOptions()
		transportProtoOptions =  fuzzingSession.getTransportOptions()
		applicationProtoOptions = fuzzingSession.getProtocolOptions()
		targetHost = fuzzingSession.getHost()
		# Getting the session protocols 
		transportProto = transportProtoOptions[0]	
		applicationProto = transportProtoOptions[1]
		# In this section we check if the input arguments are supplied. If not,we get them from the session data.
		# Otherwise,we update the options array which we'll use later.
		if verboseMode:
			extractorOptions[2] = 1
			transportProtoOptions[4] = 1
			applicationProtoOptions[3] = 1
		if not logMode:
			logMode = extractorOptions[3]
			logLevel = str(fuzzingSession.getLogLevel())
		else:
			extractorOptions[3] = logMode
		if not savingMode:
			savingMode = fuzzingSession.getSaveMode()
		if sourcePort != "0":
			transportProtoOptions[2] = atoi(sourcePort)
		else:
			sourcePort = str(transportProtoOptions[2])
		if targetPort == None:
			targetPort = fuzzingSession.getPort()
		if responseTimeout != None:
			transportProtoOptions[3] = responseTimeout
		else:
			responseTimeout = transportProtoOptions[3]
		if banner != 0:
			transportProtoOptions[6] = banner
		else:
			banner = transportProtoOptions[6]
		if pingTimeout != None:
			applicationProtoOptions[1] = pingTimeout
		else:
			pingTimeout = applicationProtoOptions[1]
		if betweenTime == None:
			betweenTime = fuzzingSession.getBetweenTime()
		if maxNumberTimeouts != None:
			applicationProtoOptions[2] = maxNumberTimeouts
		else:
			maxNumberTimeouts = applicationProtoOptions[2]
		if firstCommand != "":
			applicationProtoOptions[0] = firstCommand
		xmlCommandsFile = extractorOptions[0]
		xmlResponsesFile = extractorOptions[1]
		loadingCommands = 0
		applicationProtoOptions[5] = tmpFileName
		applicationProtoOptions[6] = resumingMode
		applicationProtoOptions[7] = loadingCommands
		xmlCommandsFile = applicationProtoOptions[8]
		xmlResponsesFile = applicationProtoOptions[9]
		# Checking if the session commands file exists 
		auxFileName = "Restore/Commands/"+fuzzingSession.getName()+"/"+fuzzingSession.getName()+"_commands"
		if os.path.exists(auxFileName):
			# Creating a soft link between the commands file and a temporal file which we'll use in the session
			os.link(auxFileName, "Tmp/"+tmpFileName)
		else:
			sys.exit("File Error: The commands file '"+auxFileName+"' does not exist")
	else:
		# Variables values in the xml files
		xmlVariables = {'REMOTE_ADDRESS':targetHost, 'REMOTE_PORT':targetPort, 'LOCAL_ADDRESS':None, 'LOCAL_PORT':None, 'TRANSPORT_PROTOCOL':transportProto, 'APP_PROTOCOL':applicationProto, 'ACTUAL_COMMAND':''}
		if not generatingCommands:
			# Neither resuming session nor generating commands
			
			# Initializing options arrays
			extractorOptions = [xmlCommandsFile, xmlResponsesFile, verboseMode, logMode, xmlVariables]
			transportProtoOptions = [transportProto, applicationProto, atoi(sourcePort), responseTimeout, verboseMode, logMode, banner]
			applicationProtoOptions = [firstCommand, pingTimeout, maxNumberTimeouts, verboseMode, logMode, tmpFileName, resumingMode, loadingCommands, xmlCommandsFile, xmlResponsesFile]
			# Setting the time between requests   
			if betweenTime == None:
				betweenTime = 500
			if loadingCommands:
				# If we use a file to obtain the commands,we create a soft link with the temporal file
				os.link(commandsFileName, "Tmp/"+tmpFileName)
			
	
	if logMode or sessionMode:
		# Setting the log file and/or session name
		prefix = ""
		date = str(datetime.today())[:-7]
		timestamp = date[:10]+'@'+date[11:]
		if not generatingCommands:
			# Checking the configuration file has the extension ".xml"
			sepIndex = xmlCommandsFile.rfind('/')
			extIndex = xmlCommandsFile.rfind('.xml')
			if extIndex == len(xmlCommandsFile)-4:
				prefix = xmlCommandsFile[sepIndex+1:-4]
			else:
				sys.exit("File Error: The configuration file '"+xmlCommandsFile+"' is not an xml file."+"'\n\n"+usage)
			sessionName = targetHost+"_"+prefix+timestamp
		else:
			prefix = sys.argv[6]
			sessionName = prefix+timestamp
			
		
		# Setting the log configuration
		if logMode:
			if logLevel == '1':
				loggingLevel = logging.WARNING
			elif logLevel == '2':
				loggingLevel = logging.INFO
			elif logLevel == '3':
				loggingLevel = logging.DEBUG
			else:
				sys.exit("Syntax Error: The log level must be an integer between 1 and 3.\n\n"+usage)
			logging.basicConfig(format='%(asctime)s %(levelname)s [%(name)s] %(message)s',filename="Logs/"+sessionName,filemode='w',level=loggingLevel)
			logFile = logging.getLogger("Malybuzz")
		
	# Initialization of the configuration files parser	
	if generatingCommands:
		generatingExtractorOptions = [xmlCommandsFile,xmlResponsesFile,1,1,xmlVariables]
		parser = ProtocolExtractor(generatingExtractorOptions)		
	else:
		parser = ProtocolExtractor(extractorOptions)

	# Getting the remote port if not specified with '-r' option
	if targetPort == None:
		targetPort = parser.getPort()
	xmlVariables = parser.getXmlVariables()
	xmlVariables['REMOTE_PORT'] = targetPort
	parser.setXmlVariables(xmlVariables)
	
	if generatingCommands:
		# Changing the configuration file by default with the supplied
		parser.setCommandsFile(minidom.parse(xmlCommandsFile))
		# Getting an array with commands parameters from configuration file
		commandsList = parser.parseCommands()
		dictIdName = commandsList[0]
		commandsIndexes = commandsList[1]
		generator = commandsList[2]
		remotePort = targetPort
		endLine = commandsList[3]
		fuzzingSummary = commandsList[4]
		# Printing the fuzzing summary
		print fuzzingSummary
		# Initialization of the commands list object with log mode activated
		commandsListObject = CommandsList(commandsList[2], None, 1, 0, 0)
		# Getting the variable which stores all the information about the commands we want to send
		commandsFiles = commandsListObject.getCommandsFile()
		# Setting the ids of the command we want to send 
		commandsFiles['commandIds'] = commandsListObject.getCommandIds()
		# Setting the dictionary with the last commands sent
		commandsFiles['lastCommands'] = commandsListObject.getLastCommands()
		# Setting the fuzzing session data the parser returns
		commandsFiles['dictIdName'] = dictIdName
		commandsFiles['commandsIndexes'] = commandsIndexes
		commandsFiles['remotePort'] = remotePort
		commandsFiles['endLine'] = endLine
		commandsFiles['fuzzingSummary'] = fuzzingSummary
		commandsFiles['indexCommandsFiles'] = commandsListObject.getIndexCommandsFiles()
		commandsFiles['countPerCommand'] = commandsListObject.getCountPerCommand()
		commandsFiles['commandsSent'] = commandsListObject.getCommandsSent()
		# Storing the commands
		if not os.path.exists("Restore/Commands/"+sessionName):
			os.mkdir("Restore/Commands/"+sessionName)
		else:
			error(logMode, logFile, "File Error: The commands folder 'Restore/Commands/"+sessionName+"' already exists.")							
		for id in commandsListObject.getCommandIds():
			for file in commandsFiles[id]:
			 	if os.path.exists("Tmp/"+file):
			 		newName = "Restore/Commands/"+sessionName+"/"+file
					if not os.path.exists(newName):
						# Linking the commands temporal file with the commands session file
						os.link("Tmp/"+file, newName)
						# Removing the temporal file
						os.remove("Tmp/"+file)
					else:
						error(logMode, logFile, "File Error: The commands file '"+newName+"' already exists.")
				else:
					error(logMode, logFile, "File Error: The temporal file 'Tmp/"+file+"' does not exist.")
					
		# Closing the open files
		commandsListObject.getActualCommandsFile().close()
		commandsFiles.close()
		# Getting the temporal file we've used in the commands generation 
		tmpFileName = commandsListObject.getTmpFile()
		# Checking for files existence 
		if os.path.exists("Tmp/"+tmpFileName):
			if os.path.exists("Restore/Commands/"+sessionName+"/"+sys.argv[6]):
				error(logMode, logFile, "File Error: The output file 'Restore/Commands/"+sessionName+"/"+sys.argv[6]+"' already exists.")
			else:
				# Linking the temporal file with the output file supplied
				os.link("Tmp/"+tmpFileName, "Restore/Commands/"+sessionName+"/"+sys.argv[6])
				# Removing the temporal file
				os.remove("Tmp/"+tmpFileName)
		else:
			error(logMode, logFile, "File Error: The temporal file 'Tmp/"+tmpFileName+"' does not exist.")
		sys.exit("Commands saved")
	else:
		# Initializing and getting the connection object
		communicationManager = CommunicationManager(targetHost,atoi(targetPort),transportProtoOptions)
		connection = communicationManager.getConnection()
		xmlVariables['LOCAL_ADDRESS'] = communicationManager.getLocalAddress()
		xmlVariables['LOCAL_PORT'] = communicationManager.getLocalPort()
		parser.setXmlVariables(xmlVariables)
		
		# Getting an array with commands parameters from configuration file
		if loadingCommands or resumingMode:
			# If we are loading commands or resuming the session we get them from the temporal files 
			if os.path.exists("Tmp/"+tmpFileName):
				commandsTmp = shelve.open("Tmp/"+tmpFileName)
				if loadingCommands:
					commandIds = commandsTmp['commandIds']
					commandsDir = commandsFileName[:commandsFileName.rfind('/')]
					commandsDir = commandsDir[commandsDir.rfind('/')+1:]
				else:
					commandIds = fuzzingSession.getCommandIds()
					commandsDir = fuzzingSession.getName()
				parseCommandsReturned = [commandsTmp['dictIdName'],commandsTmp['commandsIndexes'],None,commandsTmp['endLine'],commandsTmp['fuzzingSummary']]
				parser.setCommandsIds(parseCommandsReturned[1])
				# Creating the temporal files
				for id in commandIds:
					for file in commandsTmp[id]:
					 	if os.path.exists("Restore/Commands/"+commandsDir+"/"+file):
							if not os.path.exists("Tmp/"+file):
								# Linking the commands temporal file with the commands session file
								os.link("Restore/Commands/"+commandsDir+"/"+file,"Tmp/"+file)
						else:
							error(logMode, logFile, "File Error: The commands file 'Restore/Commands/"+commandsDir+"/"+file+"' does not exist.")
				commandsTmp.close()
			else:
				error(logMode, logFile, "File Error: The temporal file 'Tmp/"+tmpFileName+"' does not exist.")
		else:
			parseCommandsReturned = parser.parseCommands()	
	fuzzingSummary = parseCommandsReturned[4]
	# Printing the fuzzing summary
	print fuzzingSummary
	
	# Getting the response parameters of the protocol
	responsesList = parser.parseResponses()
	
	# Initializing and getting the protocol object
	protocol = ProtocolManager(connection, transportProto, parseCommandsReturned, responsesList, applicationProtoOptions)
	# Getting the CommandsList object
	commandsList = protocol.getCommandsList()
	# Getting the temporal file we've used in the commands generation
	tmpFileName = commandsList.getTmpFile()
	
	if resumingMode:
		# Printing the session information when resuming
		sessionInfo =  "\n\t\t\t---------------------"+"\n"
		sessionInfo += "\t\t\t|Session information|"+"\n"
		sessionInfo += "\t\t\t---------------------"+"\n\n"
		sessionInfo += "                      Target host: "+targetHost+"\n"
		sessionInfo += "                      Target port: "+targetPort+"\n"
		sessionInfo += "                      Source port: "
		if str(sourcePort) == "0":
			sessionInfo += "random\n"
		else:
			sessionInfo += sourcePort+"\n"
		sessionInfo += "               Transport protocol: "+transportProto+"\n"
		sessionInfo += "             Application protocol: "+applicationProto+"\n"
		sessionInfo += "                     Verbose mode: "
		if verboseMode:
			sessionInfo += "yes\n"
		else:
			sessionInfo += "no\n"
		sessionInfo += "                     Logging mode: "
		if logMode:
			sessionInfo += "yes\t"+"Level: "+logLevel+"\n"
		else: 
			sessionInfo += "no\n"
		sessionInfo += "                      Saving mode: "
		if savingMode:
			sessionInfo += "yes\n"
		else:
			sessionInfo += "no\n"
		sessionInfo += "                 Response timeout: "+str(responseTimeout)+"ms\n"
		sessionInfo += "                     Ping timeout: "+str(pingTimeout)+"ms\n"
		sessionInfo += "            Time between requests: "+str(betweenTime)+"ms\n"
		sessionInfo += "              Continue on timeout: "
		if maxNumberTimeouts != None:
			sessionInfo += "yes\t"+"Maximum number timeouts: "
			if maxNumberTimeouts == 0:
				sessionInfo += "no limit"+"\n"
			else:
				sessionInfo += str(maxNumberTimeouts)+"\n"
		else:
			sessionInfo += "no\n"
		if xmlCommandsFile.rfind('.xml') != -1:
			sessionInfo += "      Configuration commands file: "+xmlCommandsFile+"\n"
		if xmlResponsesFile.rfind('.xml') != -1:
			sessionInfo += "     Configuration responses file: "+xmlResponsesFile+"\n"
		if logMode:
			logFile.info("\n"+sessionInfo)
		print sessionInfo
		
		# Setting the new session name
		fuzzingSession.setName(sessionName)
		# Getting the commands infomation to resume: actual command,last commands sent,different commands we want to send...
		command = fuzzingSession.getCommand()
		commandsList.setCommandIds(fuzzingSession.getCommandIds())
		commandsList.setLastCommands(fuzzingSession.getLastCommands())
		commandsList.setIndexCommandsFiles(fuzzingSession.getIndexCommandsFiles())
		commandsList.setCountPerCommand(fuzzingSession.getCountPerCommand())
		commandsList.setCommandsSent(fuzzingSession.getCommandsSent())
		commandIdNameDict = parseCommandsReturned[0]
		# Setting the actual command in the stored session like the next command in the actual session
		commandsList.setNextCommand(command[0],command[1])
		# Printing information of the number of different commands sent and the total we want to send
		info = "Number of different commands sent:\n\n"
		for id in commandsList.getCommandIds():
			numCommands += commandsList.getCountPerCommand()[id]
			info += "\t\t\t\t"+commandIdNameDict[id]+tabs(commandIdNameDict[id], 15)+" --> "+str(commandsList.getCommandsSent()[id])+"\n"
		info += "\n"
		info += "                   Total commands: "+str(numCommands)
		if logMode:
			logFile.info("\n"+info)
		print info+"\n"
	
	# Printing the connection details
	print communicationManager.getConnectionData()	
	# Getting the variable with all the information related to the commands
	commandsFiles = commandsList.getCommandsFile()
	
	if sessionMode:
		if responseTimeout == None:
			transportProtoOptions[3] = 100000
		if pingTimeout == None:
			applicationProtoOptions[1] = 2
		# Creating a session object
		fuzzingSession = Session(sessionName)
		# Setting the actual session data 
		fuzzingSession.setHost(targetHost)
		fuzzingSession.setPort(targetPort)
		fuzzingSession.setBetweenTime(betweenTime)
		fuzzingSession.setSaveMode(savingMode)
		fuzzingSession.setLogLevel(logLevel)
		fuzzingSession.setExtractorOptions(extractorOptions)
		fuzzingSession.setTransportOptions(transportProtoOptions)
		fuzzingSession.setProtocolOptions(applicationProtoOptions)
		if savingMode:
			if not commandsFiles.has_key('dictIdName'):
				# If not set,we put the configuration file data in the commands dictionary
				commandsFiles['dictIdName'] = parseCommandsReturned[0]
				commandsFiles['commandsIndexes'] = parseCommandsReturned[1]
				commandsFiles['remotePort'] = targetPort
				commandsFiles['endLine'] = parseCommandsReturned[3]
				commandsFiles['fuzzingSummary'] = parseCommandsReturned[4]			
	if loadingCommands:
		# If we load the commands from a file,we copy(not modifying the file) the last commands and the commands ids in the actual session
		commandsList.setCommandIds(copy.copy(commandsFiles['commandIds']))
		commandsList.setLastCommands(copy.copy(commandsFiles['lastCommands']))
		commandsList.setIndexCommandsFiles(copy.copy(commandsFiles['indexCommandsFiles']))
		commandsList.setCountPerCommand(copy.copy(commandsFiles['countPerCommand']))
		commandsList.setCommandsSent(copy.copy(commandsFiles['commandsSent']))
		commandsList.setNextCommand(commandsList.getCommandIds()[0])
		
	########################################################################
	# Starting the fuzzing session
	########################################################################
	protocol.run(betweenTime/1000)


except:
	numCommands = 0
	commandsSent = 0
	if not generatingCommands:
		if protocol != None:
			# Getting the command list object
			commandsListObject = protocol.getCommandsList()
			# Printing information of the number of different commands sent and the total we want to send
			info = "\nNumber of different commands sent:\n\n"
			for id in commandsListObject.getCommandIds():
				numCommands += commandsListObject.getCountPerCommand()[id]
				info += "\t"+protocol.getCommandIdNameDict()[id]+tabs(protocol.getCommandIdNameDict()[id], 15)+"--> "+str(commandsListObject.getCommandsSent()[id])+"\n"
				commandsSent += commandsListObject.getCommandsSent()[id]
			if commandsSent > 0:
				info += "\n"
				info += "Total commands: "+str(numCommands)
				print info
				if logMode:
					logFile.info("\n"+info)
					print "Log file: 'Logs/"+sessionName+"'"
			else:
				print("No commands sent.")
			
			if savingMode:
				# Setting the actual command information in the session object to store it in a file 
				fuzzingSession.setCommand(commandsListObject.getActualId(),commandsListObject.getActualCommand())
				fuzzingSession.setLastCommands(commandsListObject.getLastCommands())
				fuzzingSession.setCommandIds(commandsListObject.getCommandIds())
				fuzzingSession.setIndexCommandsFiles(commandsListObject.getIndexCommandsFiles())
				fuzzingSession.setCountPerCommand(commandsListObject.getCountPerCommand())
				fuzzingSession.setCommandsSent(commandsListObject.getCommandsSent())
				if os.path.exists("Tmp/"+tmpFileName):
					if not os.path.exists("Restore/Commands/"+sessionName):
						os.mkdir("Restore/Commands/"+sessionName)
						for id in commandsListObject.getCommandIds():
							for file in commandsListObject.getCommandsFile()[id]:
							 	if os.path.exists("Tmp/"+file):
							 		newName = "Restore/Commands/"+sessionName+"/"+file
									if not os.path.exists(newName):
										# Linking the commands temporal file with the commands session file
										os.link("Tmp/"+file, newName)
										os.remove("Tmp/"+file)
									else:
										error(logMode, logFile, "File Error: The commands file '"+newName+"' already exists.")
								else:
									error(logMode, logFile, "File Error: The temporal file 'Tmp/"+file+"' does not exist.")
						if not os.path.exists("Restore/Commands/"+sessionName+"/"+sessionName+"_commands"):
							# Linking the commands temporal file with the commands session file
							os.link("Tmp/"+tmpFileName, "Restore/Commands/"+sessionName+"/"+sessionName+"_commands")
							os.remove("Tmp/"+tmpFileName)
						else:
							error(logMode, logFile, "File Error: The commands file 'Restore/Commands/"+sessionName+"/"+sessionName+"_commands' already exists.")
					else:
						error(logMode, logFile, "File Error: The commands folder 'Restore/Commands/"+sessionName+"/"+sessionName+"' already exists.")
				else:
					error(logMode, logFile, "File Error: The temporal file 'Tmp/"+tmpFileName+"' does not exist.")
					 		
				# Storing the session in a dictionary file
				file = shelve.open("Restore/"+sessionName)
				file['fuzzingSession'] = fuzzingSession
				file.close()
			if commandsListObject.getActualCommandsFile() != None:
				commandsListObject.getActualCommandsFile().close()

	# Removing other temporal files
	for file in os.listdir("Tmp"):
		os.remove("Tmp/"+file)
				 		
	if str(sys.exc_info()[0]).find("KeyboardInterrupt") != -1:
		# If we've caused a keyboard interruption (Ctrl+C,for example)
		if logMode:
			logFile.warning("Fuzzing session aborted by user: "+str(sys.exc_info()[0])+" "+str(sys.exc_info()[1]))
		sys.exit("Fuzzing session aborted by user.")
		
	elif str(sys.exc_info()[0]).find("SystemExit") != -1:
		# If the exit is forced
		if logMode and logFile != None:
			logFile.error("Exception raised: "+str(sys.exc_info()[0])+" "+str(sys.exc_info()[1]))
			exceptionList = traceback.format_tb(sys.exc_info()[2])
			exception = ""
			for i in range(len(exceptionList)):
				exception += exceptionList[i]
			logFile.debug("\n"+exception)
		sys.exit(sys.exc_info()[1])
		
	elif str(sys.exc_info()[0]).find("anydbm.error") != -1:
		# Shelve problem
		error(logMode, logFile, "File Error: The session file '"+fuzzingSessionName+"' has not a correct format.", str(sys.exc_info()[1]))

	else:
		# Not known error
		if logMode and logFile != None:
			logFile.error("Exception raised: "+str(sys.exc_info()[0])+" "+str(sys.exc_info()[1]))
			exceptionList = traceback.format_tb(sys.exc_info()[2])
			exception = ""
			for i in range(len(exceptionList)):
				exception += exceptionList[i]
			logFile.debug("\n"+exception)
			sys.exit("Fuzzing session aborted. Please, view the log file in level 3 for details.")
		sys.exit("Fuzzing session aborted.")

