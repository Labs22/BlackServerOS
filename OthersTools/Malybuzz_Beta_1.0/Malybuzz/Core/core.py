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
	core.py
	
	Module with the manager of the protocol and the commands list manager
'''

import os,sys,socket,logging,shelve,traceback,random,time

from string import atoi

from Peach import *
from Utils.misc import error, timeTest
from Protocols.Functions import *


class ProtocolManager:
	'''
		The main controller of the protocol: send commands, receive responses, analyze responses, jump to other command...
	'''
	
	# Connection object
	_connection = None
	_verbose = 0
	_count = 0
	# The generator usedto generate all the commands needed
	_commandsGenerator = []
	# Object of type CommandsList to manage the list of commands
	_commandsList = None
	# Relation between ids and names of commands
	_commandIdNameDict = None
	_commandNames = []
	# Characters of end of line
	_endLine = None
	# Names of the xml files
	_xmlFiles = []
	# Indexesof commands to send
	_sentCommandsIndexes = []
	_responses = {} 
	_pingTimeout = 0
	_numMaxTimeouts = 10
	# Time between requests
	_waitTime = None
	_transportProtocol = None
	# First ccommand to send
	_beginCommand = ""
	# Actual number of timeouts
	_numTimeouts = 0 
	# If we must continue sending commands after a timeout
	_continueOnTimeout = False
	# If we'll send the next command when a timeout happens
	_nextCommandOnTimeout = True
	_logMode = None
	_logFile = None

	def __init__(self, connection = None, transport = None, parseCommandsArray = None, responses = {}, parametersList = None):
		'''
		Initial method
		
		@arg connection: object to send and receive data in a network
		@type connection: Publisher
		@arg transport: transport layer protocol
		@type transport: string
		@arg parseCommandsArray: a list with data related to the protocol commands 
		@type parseCommandsArray: list
		@arg responses: the responses for each command of the protocol
		@type responses: dictionary
		@arg parametersList: configuration parameters
		@type parametersList: list
		'''
		
		# Setting all the class variables
		self._connection = connection
		self._responses = responses
		self._transportProtocol = transport
		self._beginCommand = parametersList[0]
		if parametersList[1] == None:
			# Default value
			self._pingTimeout = 2
		else:
			self._pingTimeout = parametersList[1]
		self._numMaxTimeouts = parametersList[2]
		if self._numMaxTimeouts != None and self._numMaxTimeouts != 1:
			self._continueOnTimeout = True
		self._verbose = parametersList[3]
		self._logMode = parametersList[4]
		self._commandIdNameDict = parseCommandsArray[0]
		self._commandNames = self._commandIdNameDict.values()
		self._sentCommandsIndexes = parseCommandsArray[1]
		self._commandsGenerator = parseCommandsArray[2]
		restoreMode = parametersList[6]
		loadingCommands = parametersList[7]
		self._xmlFiles = [parametersList[8], parametersList[9]]
		self._commandsList = CommandsList(self._commandsGenerator, parametersList[5], self._logMode, restoreMode, loadingCommands)
		self._endLine = parseCommandsArray[3]
		if self._logMode:
			self._logFile = logging.getLogger("ProtocolManager")

	
	def step(self):
		'''
			Method to continue with the next step of the protocol
		'''
		
		# If there is a response to be analyzed
		newResponse = False
		# While not ok wecontinue to send commands
		ok = False

		if self._count == 0 and self._beginCommand != "":
			# Looking for the command name we want to send 
			if self._commandNames.count(self._beginCommand) > 0:
				id = self._commandIdNameDict.keys()[self._commandNames.index(self._beginCommand)]
				self._commandsList.setNextCommand(id)
			else:
				error(self._logMode, self._logFile, "Syntax Error: Command specified like first command does not exist.", "Command: "+self._beginCommand)
		# Getting the next command to send
		command = self._commandsList.getCommand()
		id = command[:1]
		command = command[1:]
		# Getting the possible responses for this command
		responses = self._responses[id]
		# List with the commands we want to send after this command
		statesList = []

		while not ok:
			# While we've had some kind of problem sending or receiving...
			try:
				if self._logMode:
					self._logFile.debug("Sending command...")
				# Sending the command.Encoding is needed to send characters out of the standard ascii(7 bits)
				self._connection.send(command.encode('latin1'))
				if self._verbose:
					print "---------"
					print "|REQUEST|"
					print "---------"
					print command[:len(command)-2]+"\n"
				if self._logMode:
					self._logFile.info("\n\n---------\n|REQUEST|\n---------\n"+command[:len(command)-2]+"\n")
				# Checking time after sending the command
				timeFuzz = timeTest(self._connection.getHost(), self._connection.getPort(), self._transportProtocol)
				if timeFuzz > self._pingTimeout:
					print "Ping Timeout ("+str(timeFuzz)+"ms): possible positive fuzzing or bad configuration while sending command."
					if self._logMode:
						self._logFile.warning("Ping Timeout ("+str(timeFuzz)+"ms): possible positive fuzzing or bad configuration while sending command.")
					if self._continueOnTimeout:
						self._numTimeouts += 1
						if self._numTimeouts == self._numMaxTimeouts:
							if self._logMode:
								self._logFile.warning("Maximum number of timeouts reached")
							sys.exit("Maximum number of timeouts reached")
						if self._nextCommandOnTimeout:
							self._commandsList.nextCommand()
					else:
						# If not continue on timeout
						if self._logMode:
							self._logFile.warning("Fuzzing session aborted")
						sys.exit("Fuzzing session aborted")
				elif timeFuzz == -1:
					error(self._logMode, self._logFile, "Communication Error: Some kind of problem after sending commands to remote host.")
				else:
					ok = True
					if responses[0] != [] or responses[1] != []:
						# If we must wait for responses
						if self._logMode:
							self._logFile.debug("Receiving response...")
						# Receiving data
						response = self._connection.receive()
						if len(response) > 0:
							newResponse = True
					else:
						self._commandsList.nextCommand()
						break
					
			except SystemExit:
				raise
			
			except KeyboardInterrupt:
				if self._logMode:
					self._logFile.warning("Fuzzing session aborted by user")
				sys.exit("Fuzzing session aborted by user")
				
			except socket.error,e:
				# If error is Broken pipe or Connection reset by peer...
				if e[0] == 32 or e[0] == 104:
					print "Connection closed by remote host"
					print "Reconnecting..."
					if self._logMode:
						self._logFile.warning("Connection closed by remote host")
						self._logFile.warning("Reconnecting...")
					# Starting a new connection
					self._connection.start()
					# Setting the first command like the next command to send 
					self._commandsList.setLastGlobalCommand([self._sentCommandsIndexes[0],-1])
					self._commandsList.setNextCommand(self._sentCommandsIndexes[0])
					command = self._commandsList.getCommand()
					id = command[:1]
					command = command[1:]
					responses = self._responses[id]
				elif str(e) == "timed out":
					# If timeout
					if self._continueOnTimeout:
						self._numTimeouts += 1
						if self._numTimeouts == self._numMaxTimeouts:
							if self._logMode:
								self._logFile.warning("Maximum number of timeouts reached")
							sys.exit("Maximum number of timeouts reached")
						if self._nextCommandOnTimeout:
							self._commandsList.nextCommand()
							command = self._commandsList.getCommand()
							id = command[:1]
							command = command[1:]
					else:
						if self._logMode:
							self._logFile.info("Last command sent:\n"+command)
						print "Last command sent:\n"+command+"\n"
						error(self._logMode, self._logFile, "Fuzzing session aborted")
				else: 
					error(self._logMode, self._logFile, "Socket Error: \""+str(e[1])+"\" while connecting with the remote host.")
					
			except:
				error(self._logMode, self._logFile, "Error: "+str(sys.exc_info()[0])+" "+str(sys.exc_info()[1]))
				
		self._count += 1
		while newResponse:
			# Boolean to indicate if the response received is ok
			responseOk = False		
			# Getting the first line of the response,where it's used to be the response code 
			responseLine = response.split(self._endLine)[0]
			# Splitting the response line to check each part. The response code coul be in second position, for example
			responseLine = responseLine.split()
			for responseLineElement in responseLine:	
				# Check if the response code(first three characters) is in the responses list for this command
				for i in responses[0]+responses[1]:
					if responseLineElement.find(i[0]) != -1:
						# The response code is correct
						if self._verbose:
							print "----------"
							print "|RESPONSE|"
							print "----------"
							print response[:len(response)-2]+"\n"
						if self._logMode:
							self._logFile.info("\n\n----------\n|RESPONSE|\n----------\n"+response[:len(response)-2]+"\n")
						# If we must do an action with that response
						if i[1] != '':
							try:
								# Getting the function needed 
								function = eval(self._connection.getProtocol()+"."+i[1])
								# Executing the function
								ret = function(command, response, self._logMode, self._verbose, self._endLine, self._xmlFiles)
								if ret != None:
									# Sending the returned data
									if self._logMode:
										self._logFile.debug("Sending the returned data of function "+i[1])
										self._logFile.info("\n\n---------\n|REQUEST|\n---------\n"+ret)
									if self._verbose:
										print "---------"
										print "|REQUEST|"
										print "---------"
										print ret
									self._connection.send(ret)
									
							except KeyboardInterrupt:
								if self._logMode:
									self._logFile.warning("Fuzzing session aborted by user")
								sys.exit("Fuzzing session aborted by user")
								
							except:
								exceptionList = traceback.format_tb(sys.exc_info()[2])
								exception = ""
								for j in range(len(exceptionList)):
									exception += exceptionList[j]
								if self._logMode:
									self._logFile.debug("Function Error: Method "+self._connection.getProtocol()+"."+i[1]+" : "+str(sys.exc_info()[0])+" "+str(sys.exc_info()[1])+"\n"+str(exception)) 
								raise
						
						# We check if this is the last response
						if i[2] == "no":
							# We must wait for another response
							try:
								if self._logMode:
									self._logFile.debug("Receiving response...")
								response = self._connection.receive()
								
							except KeyboardInterrupt:
								if self._logMode:
									self._logFile.warning("Fuzzing session aborted by user")
								sys.exit("Fuzzing session aborted by user")
								
							except socket.error,e:
								# If a socket error occured while receiving, we'll send the next command
								newResponse = False
								response = ""
								# If error is Broken pipe or Connection reset by peer...
								if e[0] == 32 or e[0] == 104:
									print "Connection closed by remote host"
									print "Reconnecting..."
									if self._logMode:
										self._logFile.warning("Connection closed by remote host")
										self._logFile.warning("Reconnecting...")
									# Starting a new connection
									self._connection.start()
									# Setting the first command like the next command to send 
									self._commandsList.setLastGlobalCommand([self._sentCommandsIndexes[0],-1])
									self._commandsList.setNextCommand(self._sentCommandsIndexes[0])
									responseOk = True	
									break
								elif str(e) == "timed out":
									# If timeout
									if self._continueOnTimeout:
										self._numTimeouts += 1
										if self._numTimeouts == self._numMaxTimeouts:
											if self._logMode:
												self._logFile.warning("Maximum number of timeouts reached")
											sys.exit("Maximum number of timeouts reached")
										if self._nextCommandOnTimeout:
											self._commandsList.nextCommand()
										responseOk = True	
										break
									else:
										if self._logMode:
											self._logFile.info("Last command sent:\n"+command)
										print "Last command sent:\n"+command+"\n"
										error(self._logMode, self._logFile, "Fuzzing session aborted")
								else: 
									error(self._logMode, self._logFile, "Socket Error: \""+str(e[1])+"\" while receiving another response.")
									
							except:
								error(self._logMode, self._logFile, "Communication Error: Some kind of problem while receiving a new response.",str(sys.exc_info()[0])+" "+str(sys.exc_info()[1]))
						
						elif i[2] == "yes":   
							# If it's a unique response    
							newResponse = False
							
	          # Checking if we must send an specific command after the response
						if i[3] != "":
							if i[3].find(",") != -1:
								statesList = i[3].split(",")
							else:
								statesList.append(i[3])
							# Looking for the command/s found in the list of commands we want to send
							for j in statesList:
								if self._commandNames.count(j) > 0:
									# Setting the command id
									id = self._commandIdNameDict.keys()[self._commandNames.index(j)]
									if self._logMode:
										self._logFile.debug("Next command: "+j)
									self._commandsList.setNextCommand(id)
									responseOk = True	
									break
							else:
								error(self._logMode, self._logFile, "Parsing Error: Bad command \""+i[3]+"\" like next state found.")
						else:
							if i[2] == "yes":
								try:
									self._commandsList.nextCommand()
								except:
									raise
						responseOk = True	
						break
				if responseOk:
					break
			else:	
				# If the received response it's not normal, we continue with the next command
				print "Response not expected: \""+response[:len(response)-2]+"\""
				print "Possible positive fuzzing or bad protocol specification\n"
				print "Last command sent:\n"+command+"\n"
				if self._logMode:
					self._logFile.warning("Response not expected: \""+response[:len(response)-2]+"\"")
					self._logFile.warning("Possible positive fuzzing or bad protocol specification\n")
					self._logFile.info("Last command sent:\n"+command)
				newResponse = False
				self._commandsList.nextCommand()


	def run(self, waitTime):
		'''
			Method to start the fuzzing session
			
			@arg waitTime: time between requests in seconds
			@type waitTime: integer
		'''
		
		self._waitTime = waitTime
		
		try:
			while True:
				# While nor error neither final of commands list, we continue with the next step 
				self.step()
				# Waiting a time between requests
				time.sleep(self._waitTime)
		
		except generator.GeneratorCompleted, e:
			# We've reached the final of commands list
			if self._logMode:
				self._logFile.info("Percentage commands sent: 100%")
				self._logFile.warning("Fuzzing finished")
			print "Percentage commands sent: 100%"
			sys.exit("Fuzzing finished")
		
		except group.GroupCompleted:
			pass

		except:
			raise


	#############
	#  Getters  #
	#############
	
	def getCommandsList(self):
		return self._commandsList

	def getCommandIdNameDict(self):
		return self._commandIdNameDict
	
	#############
	#  Setters  #
	#############

	def setCommandsList(self,commandsList):
		self._commandsList = commandsList

	def setCommandIdNameDict(self,commandIdNameDict):
		self._commandIdNameDict	= commandIdNameDict
		
		
		
	
class CommandsList:
	'''
		Class to manage the commands list: next command, jumps to other command...
	'''
	
	_logMode = None
	_logFile = None
	_commandsGenerator = []
	# Actual number of file for a command
	_actualCommandsFile = None
	# Id of the actual command
	_actualId = 0
	# Number of command in the 
	_actualCommand = 0
	# Variable to check if we've reached the last file for a command
	_indexCommandsFiles = {}
	# Variable to store the name of commands files 
	_commands = None
	# Number of commands to send
	_totalCommands = 0
	# Number of generated requests per command
	_countPerCommand = {}
	# Number of commands sent
	_commandsSent = {}
	_totalCommandsSent = 0
	# Fuzzing progress variables
	_increment = 0
	_percentage = 0
	# Commands ids of commands we want to send
	_commandIds = []
	# Last command sent of all commands
	_lastGlobalCommand = []
	# Last command sent for each command
	_lastCommands = {}
	_tmpFile = "commands"+str(random.random())
	
	
	def __init__(self, generatorsList, tmpFileName, logMode, restoreMode, loadingCommands):
		'''
			Initial method
			
			@arg generatorsList: generator list to obtain the commands to send
			@type generatorsList: GeneratorList
			@arg tmpFileName: name of the temporal file used
			@type tmpFileName: string
			@arg logMode: parameter to say if we're in log mode or not
			@typelogMode : integer
			@arg restoreMode: parameter to say if we're in resuming mode or not
			@type restoreMode: integer
			@arg loadingCommands: indicates if we're loading commands
			@type loadingCommands: integer
		'''
		
		# Number of commands generated
		count = 0
		# Value of actual command generated
		value = None
		commandId = None
		# Temporal variable to store the commands generated
		tmpCommands = {}
		self._commandsGenerator = generatorsList
		if restoreMode or loadingCommands:
			self._tmpFile = tmpFileName
		self._commands = shelve.open("Tmp/"+self._tmpFile,writeback=True)
		self._logMode = logMode
		if self._logMode:
			self._logFile = logging.getLogger("CommandsList")
			
		if not restoreMode and not loadingCommands:
			try:
				if self._logMode:
					self._logFile.info("Generating commands...")
				print "Generating commands..."
				while True:
					# Looping until end of generation
					
					# Getting the next request
					value = self._commandsGenerator.getValue()
					commandId = str(value[:1])
					if self._commandIds.count(commandId) == 0:
						# If it's the first request of a command an initialization is needed
						self._commandIds.append(commandId)
						self._commands[commandId] = []
						self._lastCommands[commandId] = -1
						self._indexCommandsFiles[commandId] = 0
						self._commandsSent[commandId] = 0
						self._countPerCommand[commandId] = 0
						tmpCommands[commandId] = [] 
					# Storing temporally the commands
					tmpCommands[commandId].append(value)
					self._countPerCommand[commandId] += 1 
					count += 1
					if count%5000 == 0:
						if self._logMode:
							self._logFile.info("Commands generated: "+str(count))
							self._logFile.debug("Actual command in the generation process:\n"+value)
						print "Commands generated:",count				
					if self._countPerCommand[commandId]%5000 == 0:
						# Each 5000 requests of a command,these are stored in a file
						index = len(self._commands[commandId])
						self._commands[commandId].append(commandId+self._tmpFile+"_"+str(self._countPerCommand[commandId]))
						# Creating the temporal file of requests 
						commandsFile = shelve.open("Tmp/"+self._commands[commandId][index],writeback=True)
						commandsFile[commandId] = tmpCommands[commandId]
						commandsFile.close()
						tmpCommands[commandId] = []
					# Next step in the generation
					self._commandsGenerator.next()
					
			except generator.GeneratorCompleted, e:
				# If generation is finished,the requests in the temporal variables are stored in temporal files
				for id in self._commandIds:
					if tmpCommands[id] != []:
						index = len(self._commands[id])
						self._commands[id].append(id+self._tmpFile+"_"+str(self._countPerCommand[id]))
						commandsFile = shelve.open("Tmp/"+self._commands[id][index],writeback=True)
						commandsFile[id] = tmpCommands[id]
						commandsFile.close()
						tmpCommands[id] = []
				# Setting the first command id as the actual id
				self._actualId = self._commandIds[0]
				# Setting the actual file to get the requests
				self._actualCommandsFile = shelve.open("Tmp/"+self._commands[self._commandIds[0]][0],writeback=True)
				# Setting the actual last command,none
				self._lastGlobalCommand = [self._commandIds[0],-1]
				if self._logMode:
					self._logFile.info("All commands generated: "+str(count))
				print "All commands generated:",count,"\n"
				pass
			
			except KeyboardInterrupt:
				error(self._logMode, self._logFile, "Commands generation aborted by user")
				
			except:
				exceptionList = traceback.format_tb(sys.exc_info()[2])
				exception = ""
				for i in range(len(exceptionList)):
					exception += exceptionList[i]
				error(self._logMode, self._logFile, "Generating Error: Some kind of problem while managing the list of commands",str(sys.exc_info()[0])+" "+str(sys.exc_info()[1])+"\n Actual value: "+value+"\n"+exception)

			
	def nextCommand(self):
		'''
			Method to continue with the next command of the list
		'''
		if len(self._actualCommandsFile[self._actualId]) == self._actualCommand+1:
			# Last request of the command file
			if self._logMode:
				self._logFile.debug("Last request of the file: "+str(self._actualCommand)+"/"+str(len(self._actualCommandsFile[self._actualId])))
			if len(self._commands[str(self._actualId)]) == self._indexCommandsFiles[self._actualId]+1:
				# Last file for this command
				if self._logMode:
					self._logFile.debug("Last file for this command: "+str(self._indexCommandsFiles[self._actualId])+"/"+str(len(self._commands[str(self._actualId)])))
				if len(self._commandIds) == self._commandIds.index(self._actualId)+1:
					# Commands list final
					if self._logMode:
						self._logFile.debug("Last request to send: "+str(self._commandIds.index(self._actualId))+"/"+str(len(self._commandIds)))
					raise generator.GeneratorCompleted
				else:
					# Jump to next command
					if self._lastGlobalCommand == [self._actualId, self._actualCommand]:
						# If we've already sent this command,we go to the next command
						
						# Setting the new values for the variables
						self._actualId = self._commandIds[self._commandIds.index(self._actualId)+1]
						self._actualCommand = self._lastCommands[self._actualId]
						self._actualCommandsFile.close()
						self._actualCommandsFile = shelve.open("Tmp/"+self._commands[self._actualId][self._indexCommandsFiles[self._actualId]],writeback=True)
						self.nextCommand()
			else:
				# Jump to the next file for this command
				self._indexCommandsFiles[self._actualId] += 1
				self._actualCommandsFile.close()
				self._actualCommandsFile = shelve.open("Tmp/"+self._commands[self._actualId][self._indexCommandsFiles[self._actualId]],writeback=True)
				self._actualCommand = 0			
				self._lastCommands[self._actualId] = self._actualCommand
				self._commandsSent[str(self._actualId)] += 1
		else:
			# Jump to the next request in the file
			self._actualCommand += 1
			self._commandsSent[str(self._actualId)] += 1
			self._lastCommands[self._actualId] = self._actualCommand
			
			
	def getCommand(self):
		'''
			Method to obtain the actual command request to send
			
			@return: actual command request
			@rtype: string
		'''
		# Some poor lines to give the progress of the fuzzing
		if self._totalCommandsSent == 0:
			self._totalCommands = self.getTotalCommands()
			self._increment = self._totalCommands/10
		self._totalCommandsSent += 1
		if self._increment != 0 and self._totalCommandsSent%self._increment == 0 and self._percentage != 100:
			self._percentage += 10
			if self._logMode:
				self._logFile.info("Percentage commands sent: "+str(self._percentage)+"%")
			print "Percentage commands sent: "+str(self._percentage)+"%"
		# Update of the last command
		self._lastGlobalCommand = [self._actualId, self._actualCommand]
		return self._actualCommandsFile[self._actualId][self._actualCommand]


	def getTotalCommands(self):
		'''
			Gives the total number of commands to send
			
			@return: number of commands generated 
			@rtype: integer
		'''
		
		numCommands = 0
		for id in self.getCommandIds():
			numCommands += self.getCountPerCommand()[id]
		return numCommands
	 
	
	def setNextCommand(self, commandId, commandNum = None):
		'''
			Set the next command to send
			
			@arg commandId: command id of the desired next command
			@type commandId: string
			@arg commandNum: number of request for the desired command
			@type commandNum: integer
		'''
		
		if self._lastGlobalCommand == []:
			# Setting the last command by the first time
			if commandNum == None:
				self._lastGlobalCommand = [commandId, -1]
			else:
				self._lastGlobalCommand = [commandId, commandNum-1]
		if self._actualId != commandId:
			self._actualId = commandId
			if self._actualCommandsFile != None:
				# Closing the actual open file
				self._actualCommandsFile.close()
			# Opening the new commands file
			self._actualCommandsFile = shelve.open("Tmp/"+self._commands[str(self._actualId)][self._indexCommandsFiles[self._actualId]],writeback=True)
		if commandNum == None:
			self._actualCommand = self._lastCommands[commandId]
			self.nextCommand()
		else:
			self._actualCommand = commandNum-1


	#############
	#  Getters  #
	#############	
	
	def getCommandsSent(self):
		return self._commandsSent
	
	def getCommandsFile(self):
		return self._commands

	def getCommandIds(self):
		return self._commandIds

	def getLastCommands(self):
		return self._lastCommands

	def getIndexCommandsFiles(self):
		return self._indexCommandsFiles

	def getCountPerCommand(self):
		return self._countPerCommand

	def getCommandsSent(self):
		return self._commandsSent

	def getActualCommandsFile(self):
		return self._actualCommandsFile

	def getActualId(self):
		return self._actualId
	
	def getActualCommand(self):
		return self._actualCommand
	
	def getTmpFile(self):
		return self._tmpFile
	
	
	#############
	#  Setters  #
	#############
	
	def setCommandsSent(self,commandsSent):
		self._commandsSent = commandsSent
	
	def setCommandsFile(self,commands):
		self._commands = commands
		
	def setCommandIds(self,commandIds):
		self._commandIds = commandIds

	def setLastCommands(self,lastCommands):
		self._lastCommands = lastCommands

	def setCommandsFile(self,commands):
		self._commands = commands
		
	def setCommandIds(self,commandIds):
		self._commandIds = commandIds

	def setLastCommands(self,lastCommands):
		self._lastCommands = lastCommands

	def setIndexCommandsFiles(self,indexCommandsFiles):
		self._indexCommandsFiles = indexCommandsFiles

	def setCountPerCommand(self,countPerCommand):
		self._countPerCommand = countPerCommand

	def setCommandsSent(self,commandsSent):
		self._commandsSent = commandsSent
		
	def setActualCommandsFile(self,actualCommandsFile):
		self._actualCommandsFile = actualCommandsFile

	def setActualId(self,actualId):
		self._actualId = actualId
	
	def setActualCommand(self,actualCommand):
		self._actualCommand = actualCommand
		
	def setTmpFile(self,tmpFile):
		self._tmpFile = tmpFile
	
	def setLastGlobalCommand(self,lastGlobalCommand):
		self._lastGlobalCommand = lastGlobalCommand
