#
# Malybuzz
# By Jose Miguel Esparza <josemiguel.esparza@gmail.com>
#												 <jesparza@s21sec.com>
# Copyright 2007 Jose Miguel Esparza
#
# This file is part of Malybuzz.
#
#		Malybuzz is free software; you can redistribute it and/or modify
#		it under the terms of the GNU General Public License as published by
#		the Free Software Foundation; either version 3 of the License, or
#		(at your option) any later version.
#
#		Malybuzz is distributed in the hope that it will be useful,
#		but WITHOUT ANY WARRANTY; without even the implied warranty of
#		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
#		GNU General Public License for more details.
#
#		You should have received a copy of the GNU General Public License
#		along with this program.	If not, see <http://www.gnu.org/licenses/>.
#

class Session:
	'''
		Class to store the fuzzing session
	'''

	_sessionName = None
	_remoteHost = None
	_remotePort = None
	_betweenTime = None
	_saveMode = None
	_logLevel = None
	_extractorOptions = None
	_transportOptions = None
	_protocolOptions = None
	_resumedSession = False
	_actualCommand = []
	_lastCommands = {}
	_commandIds = []
	_indexCommandsFiles = {}
	_filesPerCommand = {}
	_countPerCommand = {}
	_commandsSent = {}
	
	
	def __init__(self, sessionName):
		self._sessionName = sessionName


	#############
	#	Setters	#
	#############

	def setName(self, name):
		self._sessionName = name

	def setHost(self, host):
		self._remoteHost = host
		
	def setPort(self, port):
		self._remotePort = port

	def setBetweenTime(self, betweenTime):
		self._betweenTime= betweenTime
		
	def setSaveMode(self, saveMode):
		self._saveMode = saveMode
		
	def setLogLevel(self, logLevel):
		self._logLevel = logLevel
	
	def setExtractorOptions(self, extractorOptions):
		self._extractorOptions = extractorOptions
		
	def setTransportOptions(self, transportOptions):
		self._transportOptions = transportOptions

	def setProtocolOptions(self, protocolOptions):
		self._protocolOptions = protocolOptions

	def setCommand(self, idCommand, numCommand):
		self._actualCommand = [idCommand, numCommand]
		
	def setLastCommands(self, lastCommands):
		self._lastCommands = lastCommands
		
	def setCommandIds(self, commandIds):
		self._commandIds = commandIds
		
	def setIndexCommandsFiles(self, indexCommandsFiles):
		self._indexCommandsFiles = indexCommandsFiles
		
	def setCountPerCommand(self, countPerCommand):
		self._countPerCommand = countPerCommand
		
	def setCommandsSent(self, commandsSent):
		self._commandsSent = commandsSent


	#############
	#	Getters	#
	#############	

	def getName(self):
		return self._sessionName
	 
	def getHost(self):
		return self._remoteHost
	
	def getPort(self):
		return self._remotePort

	def getBetweenTime(self):
		return self._betweenTime

	def getSaveMode(self):
		return self._saveMode

	def getLogLevel(self):
		return self._logLevel
	
	def getExtractorOptions(self):
		return self._extractorOptions
		
	def getTransportOptions(self):
		return self._transportOptions

	def getProtocolOptions(self):
		return self._protocolOptions

	def getCommand(self):
		return self._actualCommand
		
	def getLastCommands(self):
		return self._lastCommands
	
	def getCommandIds(self):
		return self._commandIds
		
	def getIndexCommandsFiles(self):
		return self._indexCommandsFiles
		
	def getCountPerCommand(self):
		return self._countPerCommand
		
	def getCommandsSent(self):
		return self._commandsSent