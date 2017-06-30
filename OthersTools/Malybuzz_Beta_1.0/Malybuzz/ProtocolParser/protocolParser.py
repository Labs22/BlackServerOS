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
	protocolParser.py
	
	Module to parse the xml elements of configuration files to python structures
'''

import logging
from os import path

from xml.dom import minidom
from xml.parsers import expat

from Peach import group
from Peach.Generators import *
from Fuzzing.fuzzing import FuzzingFactory
from Utils.misc import *


class ProtocolExtractor:
	'''
	Parser to transform the data in the xml files to data structures in python
	'''
	
	# XML file with the protocol commands specification 
	_specifXml = ""
	# XML file with the protocol responses specification 
	_statesXml = ""
	# Actual XML file information
	_actualFile = ""
	_actualRoot = None
	# List with the ids of the commands we want to send
	_commandsIds = []
	_verbose = 0
	_logMode = None
	_logFile = None
	# Configuration files names
	_commandsFile = None
	_responsesFile = None
	# Commands ids in the commands file
	_ids = []
	# Types of xml variables
	XML_VARIABLE = 1 
	XML_FUNCTION = 2
	XML_ELEMENT = 3
	# Dictionary with the xml functions used to configure the xml commands file
	XML_FUNCTIONS = {'SIZE':'size'}
	_xmlVariables = {}
	

	def __init__(self, parametersList):
		'''
		Initialization method for the class
		
		@arg parametersList: A list with the necessary parameters to extract the data
		@type	parametersList: list
		'''
		
		self._commandsFile = parametersList[0]
		self._responsesFile = parametersList[1]
		self._verbose = parametersList[2]
		self._logMode = parametersList[3]
		# List with constant variables to use in the commands file
		self._xmlVariables = parametersList[4]
		
		try:
			if self._logMode:
				self._logFile = logging.getLogger("ProtocolExtractor")
			# Checking the existence of the commands file
			if path.exists(self._commandsFile):
				self._specifXml = minidom.parse(self._commandsFile)
			else:
				error(self._logMode, self._logFile, "File Error: The configuration file '"+self._commandsFile+"' does not exist.") 
			# Checking the existence of the responses file
			if path.exists(self._responsesFile):
				self._statesXml = minidom.parse(self._responsesFile)
			else:
				error(self._logMode, self._logFile, "File Error: The configuration file '"+self._responsesFile+"' does not exist.") 					
		except expat.ExpatError,e:
			# This error means the file is not an xml file or is not correct
			if self._specifXml == "":
				error(self._logMode, self._logFile, "File Error: The configuration file '"+self._commandsFile+"' is not properly formatted.")
			else:
				error(self._logMode, self._logFile, "File Error: The configuration file '"+self._responsesFile+"' is not properly formatted.")
		except:
			raise
			
	def parseCommands(self):
		'''
		This method parses the commands specification file of the protocol,and returns a list with a dictionary
		to associate command ids and command names, a list with the order of the commands to be sent and a GeneratorList
		with a list of commands to generate,among other things.
		
		@return: A list composed of a dictionary,a list,a GeneratorList, the port associated, the end of line specified,
						and the summary of the fuzzing session.
		@rtype:	list
		'''
		
		commandsGroup = group.Group()
		# Fuzzing object
		fuzzingFactory = FuzzingFactory()
		# List with the return data
		list = []
		# To associate a command id with his name
		dictCommands = {}
		# To printing purposes
		dictPrinting = {'com':'Command: ','arg':'Argument: ','field':'Field: ','field-value':'Field value: ','param':'Field parameter: ','param-value':'Field parameter value: ','body':'Body fuzzing: '}
		# To indicate if we must print the header
		printHeaderFuzz = True
		# Summary of the fuzzing session
		fuzzingSummary = ""
		# Permitted values for attributes
		permittedFuzzings = ["overflow","formatString","badString","badNumber","badPath","badIp","badHost","repeat","sql","binary"]
		permittedTypes = ["text","integer","float","date"]
		permittedSendValues = ["yes","no"]
		# String to replace the space character
		spaceChars = "##space##"
		# String to replace the end of line character
		endLineChars = "##intro##"
		# The end of line is normalized by the python xml parser: all combinations of \r and \n become \n
		normalizedXmlEndLine = '\n'
		# Variables to check the file structure
		commandsOk = 0
		endCommandOk = 0
		comOk = 0
		# Xml file elements
		xmlCommandsNode = None
		xmlCommandsChilds = None
		xmlComs = None
		endCommand = None
		endLine = None
		
		if self._verbose:
			print "Extracting commands from '"+self._commandsFile+"' file..."+"\n"
		if self._logMode:
			self._logFile.info("Extracting commands from '"+self._commandsFile+"' file...\n")
		self._actualFile = self._commandsFile
		
		########################################################################
		# Checking the existence of mandatory nodes in the xml file structure
		########################################################################
		
		# Checking the root node
		xmlRoot = self.searchChildNodes(self._specifXml,'protocol')
		if len(xmlRoot) == 0:
			error(self._logMode, self._logFile, "Parsing Error: The root node in the file '"+self._commandsFile+"' must be 'protocol'.")			
		if len(xmlRoot) > 1:
			error(self._logMode, self._logFile, "Parsing Error: The root node in the file '"+self._commandsFile+"' must be unique.")						
		xmlRoot = xmlRoot[0]
		self._actualRoot = xmlRoot
		xmlRootChilds = self.searchChildNodes(xmlRoot,None)
		# Checking the childs of root node
		for i in xmlRootChilds:
			if i.nodeName == 'commands':
				commandsOk = 1
				xmlCommandsNode = i
				xmlCommandsChilds = self.searchChildNodes(i,None)
				xmlComs = self.searchChildNodes(i,'com')
				for j in xmlCommandsChilds:
					if j.nodeName == 'end-command':
							endCommandOk = 1
							if j.lastChild == None:
								error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a valid value for the 'end-command' element.")										
							endCommand = j.lastChild.nodeValue
							if endCommand != "CR" and endCommand != "LF" and endCommand != "CRLF":	
								error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a valid value for the 'end-command' element: CR | LF | CRLF.")										
							endCommand = endFormat(j.lastChild.nodeValue)
					if j.nodeName == 'end-line':
							if j.lastChild == None:
								error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a valid value for the 'end-line' element.")										
							endLine = j.lastChild.nodeValue
							if endLine != "CR" and endLine != "LF" and endLine != "CRLF":	
								error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a valid value for the 'end-line' element: CR | LF | CRLF.")										
							endLine = endFormat(j.lastChild.nodeValue)
					if j.nodeName == 'com':
							comOk = 1
					if endCommandOk and comOk:
						break
			if commandsOk:
				break
		if not commandsOk:
			error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain one 'commands' element in the 'protocol' node.")
		if not endCommandOk:
			error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain one 'end-command' element in the 'commands' node.")
		if not comOk:
			error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain at least one 'com' element in the 'commands' node.")
			
		# Checking the commands node
		xmlCommands = xmlRoot.getElementsByTagName('commands')
		if len(xmlCommands) != 1:
			error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a unique 'commands' node.")
		xmlCommands = xmlCommands[0]
		
		# Checking the end of command characters
		xmlEndCommand = xmlRoot.getElementsByTagName('end-command')
		if len(xmlEndCommand) != 1:
			error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a unique 'end-command' node.")

		
		########################################################################
		# Getting the xml data
		########################################################################

		# Getting the global arguments
		globals = []
		globalsTmp = self.searchChildNodes(xmlCommands, 'global-arg')
		for i in globalsTmp:
			if self.checkAttribute(i, 'send', 'yes', permittedSendValues):
				globals.append(i)
				
		# Getting the request fields
		fields = []
		fieldsTmp = self.searchChildNodes(xmlCommands, 'field')
		for i in fieldsTmp:
			if i.lastChild == None:
				error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a value for each '"+i.nodeName+"' element.")
			if self.checkAttribute(i, 'send', 'yes', permittedSendValues):
				fields.append(i)
				
		# For each command we want to send...
		for i in xmlComs:
			self._ids.append(self.checkAttribute(i, 'id'))
			if self.checkAttribute(i, 'send', 'yes', permittedSendValues):
				# List of pairs [fuzzing type,data type]
				fuzz = []
				# List of pairs [element,element type] util to pritnting and parsing
				elementsList = []
				# List of the command elements separated by space 
				elements = ""
				index = 0
				# Global arguments for all commands.Supposed to be modified depending on the command.
				globalArgs = []
				# Variable indicating if we want to do some kind of fuzzing
				doFuzzing = False
				body = ""
				
				# Getting the command name and id
				name = self.searchChildNodes(i, 'com-name')
				if name == [] or len(name) > 1:
					error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain one 'com-name' node for each 'com' element.")
				name = name[0]
				if name.lastChild == None:
					error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a value for each 'com-name' element.")					
				id = self.checkAttribute(i, 'id')
				if self._commandsIds.count(id) != 0:
					error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a unique value for the 'id' attribute of a 'com' element.")					
				# Getting the fuzzing method and the type of the command name 
				fuzz.append([self.checkAttribute(name, 'fuzzing', None, permittedFuzzings, ',', 1),self.checkAttribute(name, 'type', None, permittedTypes)])
				if name.getAttribute('fuzzing') != "":
					doFuzzing = True
					
				# We must not add delimiter after the name command because is definded in the xml file
				command = id + name.lastChild.nodeValue
				elements = id + inputConverter(name.lastChild.nodeValue,normalizedXmlEndLine)
				elementsList.append([command[1:],"com"])
				# Adding the command in the lists
				if name.lastChild.nodeValue[-1].isalnum():
					dictCommands[id] = name.lastChild.nodeValue
				else:
					dictCommands[id] = name.lastChild.nodeValue[:-1]
				# Setting the xml variable to store the actual command in generation
				self._xmlVariables['ACTUAL_COMMAND'] = dictCommands[id]
				# Selecting the global arguments for this command
				for j in globals:
					if self.checkCommands(j, 'command', id):
						globalArgs.append(j)
				# Getting the arguments of this command
				args = globalArgs + self.searchChildNodes(i, 'arg')
				arguments = ""
				if args != []:
					for j in args:
						if j.lastChild == None:
							error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a value for each '"+j.nodeName+"' element.")
						argument = self.getElementValue(j.lastChild.nodeValue)
						arguments += argument
						elementsList.append([argument,"arg"])
						elements += " "+inputConverter(argument,normalizedXmlEndLine)
						# Getting the fuzzing method and the type of the command argument if necessary
						if self.checkCommands(j, 'fuzzCommand', id):
							fuzz.append([self.checkAttribute(j, 'fuzzing', None, permittedFuzzings, ',', 1),self.checkAttribute(j, 'type', None, permittedTypes)])
							if j.getAttribute('fuzzing') != "":
								doFuzzing = True
						else:
							fuzz.append(["",""])
				command += arguments
				if endLine != None:
					command += endLine
					elements += " "+endLine
					fuzz.append(["",""])
					elementsList.append(["",""])
				
				# For each field which we want to send with this command...
				for j in fields:
					if self.checkCommands(j, 'command', id):
						# Getting the field name
						name = self.searchChildNodes(j, 'field-name')
						if name == [] or len(name) > 1:
							error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain one 'field-name' node for each 'field' element.")
						name = name[0]
						if name.lastChild == None:
							error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a value for each '"+name.nodeName+"' element.")						
						field = name.lastChild.nodeValue
						# If the value is variable...
						field = self.getElementValue(field)
						elements += " "+inputConverter(field,normalizedXmlEndLine)
						elementsList.append([field,"field"])
						fieldLine = field
						# Getting the fuzzing method and the type of the field name if necessary
						if self.checkCommands(name, 'fuzzCommand', id):
							fuzz.append([self.checkAttribute(name, 'fuzzing', None, permittedFuzzings, ',', 1),self.checkAttribute(name, 'type', None, permittedTypes)])
							if name.getAttribute('fuzzing') != "":
								doFuzzing = True
						else:
							fuzz.append(["",""])
						
						# Getting the field value/s
						values = self.searchChildNodes(j, 'field-value')
						if values == []:
							error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain at least one 'field-value' node for each 'field' element.")
						for k in values:
							valueElement = self.searchChildNodes(k, 'value')
							if valueElement == [] or len(valueElement) > 1:
								error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain one 'value' node for each 'field-value' element.")
							valueElement = valueElement[0]
							if valueElement.lastChild == None:
								error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a value for each '"+k.nodeName+"' element.")						
							value = valueElement.lastChild.nodeValue
							# If the value is variable...
							value = self.getElementValue(value)
							# We must not add delimiter between value parts because is definded in the xml file
							elements += " "+inputConverter(value,normalizedXmlEndLine)
							fieldLine += value
							elementsList.append([value,"field-value"])
							
							# Getting the fuzzing method and the type of the field value if necessary
							if self.checkCommands(valueElement, 'fuzzCommand', id):
								fuzz.append([self.checkAttribute(valueElement, 'fuzzing', None, permittedFuzzings, ',', 1),self.checkAttribute(valueElement, 'type', None, permittedTypes)])
								if valueElement.getAttribute('fuzzing') != "":
									doFuzzing = True
							else:
								fuzz.append(["",""])
							
							# For each parameter of the field value
							parameters = self.searchChildNodes(k, 'parameter')
							if parameters != []:
								for m in parameters:
									if self.checkAttribute(m, 'send', 'yes', permittedSendValues):
										# If we really want to send this parameter with this field...
										if self.checkCommands(m, 'command', id):
											# Getting the parameter name
											name = self.searchChildNodes(m, 'param-name')
											if name == [] or len(name) > 1:
												error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain one 'param-name' node for each 'parameter' element.")
											name = name[0]
											if name.lastChild == None:
												error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a value for each '"+name.nodeName+"' element.")						
											paramName = name.lastChild.nodeValue
											# If the value is variable...
											paramName = self.getElementValue(paramName)
											paramName =	self.checkAttribute(name, 'beginChar') + paramName + self.checkAttribute(name, 'endChar')
											elements += " "+inputConverter(paramName,normalizedXmlEndLine)
											elementsList.append([paramName,"param"])
											fieldLine += paramName
											# Getting the fuzzing method and the type of the parameter name if necessary
											if self.checkCommands(name, 'fuzzCommand', id):
												fuzz.append([self.checkAttribute(name, 'fuzzing', None, permittedFuzzings, ',',1),self.checkAttribute(name, 'type', None, permittedTypes)])
												if name.getAttribute('fuzzing') != "":
													doFuzzing = True
											else:
												fuzz.append(["",""])
												
											# Getting the value of this parameter
											values = self.searchChildNodes(m, 'param-value')
											if values == [] or len(values) > 1:
												error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain one 'param-value' node for each 'parameter' element.")
											values = values[0]
											if values.lastChild == None:
												error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a value for each '"+values.nodeName+"' element.")						
											value = values.lastChild.nodeValue
											# If the value is variable...
											value = self.getElementValue(value)
											# We must not add delimiter between value parts because is definded in the xml file
											elements += " "+inputConverter(value,normalizedXmlEndLine)
											fieldLine += value
											elementsList.append([value,"param-value"])
											# Getting the fuzzing method and the type of the parameter value if necessary
											if self.checkCommands(values, 'fuzzCommand', id):
												fuzz.append([self.checkAttribute(values, 'fuzzing', None, permittedFuzzings, ',',1),self.checkAttribute(values, 'type', None, permittedTypes)])
												if values.getAttribute('fuzzing') != "":
													doFuzzing = True
											else:
												fuzz.append(["",""])
						if endLine != None:
							elements += " "+endLine
							fieldLine += endLine
							command += fieldLine
							fuzz.append(["",""])
							elementsList.append(["",""])
				if endCommand != None:
					elements += " "+endCommand
					command += endCommand
					fuzz.append(["",""])
					elementsList.append(["",""])
				# Getting the body element
				bodyElement = self.searchChildNodes(xmlCommandsNode, 'body')	
				if bodyElement != []:
					if len(bodyElement) > 1:
						error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a unique 'body' node.")
					bodyElement = bodyElement[0]
					if self.checkAttribute(bodyElement, 'send', 'yes', permittedSendValues):
						# If we really want to send the body with this command...
						if self.checkCommands(bodyElement, 'command', id):
							if bodyElement.lastChild == None:
								error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a value for the '"+bodyElement.nodeName+"' element.")						
							body = bodyElement.lastChild.nodeValue
							elements += " "+inputConverter(body,normalizedXmlEndLine)
							command += body
							elementsList.append(["","body"])
							
							# Getting the fuzzing method and the type of the parameter value if necessary
							if self.checkCommands(bodyElement, 'fuzzCommand', id):
								fuzz.append([self.checkAttribute(bodyElement, 'fuzzing', None, permittedFuzzings, ',', 1),self.checkAttribute(bodyElement, 'type', None, permittedTypes)])
								if bodyElement.getAttribute('fuzzing') != "":
									doFuzzing = True
							else:
								fuzz.append(["",""])
								
				########################################################################
				# Creating fuzzing blocks and printing fuzzing summary
				########################################################################
								
				if doFuzzing:
					printing = ""
					for j in fuzz:
						if elementsList[index][1] == "com":
							printing += dictPrinting[elementsList[index][1]]+elementsList[index][0]+"\n"
							# If we must do some kind of fuzzing
							if j[0] != "":
								printing += "Fuzzing: "+j[0]+"\n\n"
							else:
								printing += "\n"
						elif elementsList[index][1] == "body":
							# If we must do some kind of fuzzing
							if j[0] != "":
								printing += dictPrinting[elementsList[index][1]]+j[0]+"\n"
						else:
							# If we must do some kind of fuzzing
							if j[0] != "":
								tmpElement = elementsList[index][0]
								if len(tmpElement) > 1:
									if not tmpElement[0].isalnum():
										tmpElement = tmpElement[1:]
									if not tmpElement[-1].isalnum():
										tmpElement = tmpElement[:-1]
								printing += dictPrinting[elementsList[index][1]]+tmpElement+"\n"+"Fuzzing: "+j[0]+"\n\n"
						# Here we really add the fuzzing blocks for each type of fuzzing
						if j[0] != "":
								fuzzingTypes = j[0].split(",")
								elementType = j[1]
								if elementsList[index][1] != "body":
									delimiter = elementsList[index][0][-1]
									if delimiter.isalnum():
										delimiter = ''
								else:
									delimiter = ''
								for fuzzingMethod in fuzzingTypes:
									if self._logMode:
										self._logFile.debug("fuzzingBlock=fuzzingFactory.create('"+fuzzingMethod+"','"+elements+"',"+str(index)+",'"+delimiter+"','"+self._specifXml.getElementsByTagName('end-command')[0].lastChild.nodeValue+"','"+elementType+"')")
									# Obtaining the fuzzing block
									fuzzingBlock = fuzzingFactory.create(fuzzingMethod, elements, index, delimiter, endCommand, elementType)
									list.append(fuzzingBlock)
									self._commandsIds.append(id)
						index += 1
					if printing.find("Fuzzing") != -1:
						if printHeaderFuzz:
							fuzzingSummary = "\t\t"+"-----------------"+"\n"\
															 "\t\t"+"|Fuzzing Summary|"+"\n"\
															 "\t\t"+"-----------------"+"\n\n"
							printHeaderFuzz = False
						fuzzingSummary += printing+"\r\n"
					else:
						printing = ""
				
				# Default and valid command
				command = static.Static(command)
				# Adding the default and valid command in the list to continue with the protocol
				list.append(command)
				self._commandsIds.append(id)
		if fuzzingSummary != "":
			if self._logMode: 
				self._logFile.info("\n\n"+fuzzingSummary)
		commandsList = dictionary.GeneratorList(commandsGroup,list)		
		return [dictCommands,self._commandsIds,commandsList,endCommand,fuzzingSummary]


	def parseResponses(self):
		'''		
		This method returns a dictionary with the possible responses that the commands
		must wait for.These responses are extracted from the xml file 'protocol'_state.xml

		@return: a dictionary where the keys are the command id's and the values are lists
		with the possible responses.
		@rtype: dictionary
		''' 
		responsesList = {}
		# 'last' attribute permitted values
		permittedLastValues = ["yes","no"]
		self._actualFile = self._responsesFile
		
		if self._verbose:
			print "Extracting responses from '"+self._responsesFile+"' file..."+"\n"
		if self._logMode:
			self._logFile.info("Extracting responses from '"+self._responsesFile+"' file...\n")
		
		# Checking the root of the xml file
		xmlRoot = self.searchChildNodes(self._statesXml,'protocol-states')
		if len(xmlRoot) == 0:
			error(self._logMode, self._logFile, "Parsing Error: The root node in the file '"+self._responsesFile+"' must be 'protocol-states'.")			
		if len(xmlRoot) > 1:
			error(self._logMode, self._logFile, "Parsing Error: The root node in the file '"+self._responsesFile+"' must be unique.")						
		xmlRoot = xmlRoot[0]
		self._actualRoot = xmlRoot
		
		# Checking the unique childs for the root node
		xmlStates = self.searchChildNodes(xmlRoot,'state')
		if len(xmlStates) == 0:
			error(self._logMode, self._logFile, "Parsing Error: The file '"+self._responsesFile+"' must contain at least one 'state' element in the 'protocol-states' node.")			
		for i in xmlStates:
			listOk = []
			listErr = []
			id = self.checkAttribute(i, 'id')
			
			# Check for the existence of a command name
			name = self.searchChildNodes(i, 'name')
			if len(name) == 0 or len(name) > 1:
				error(self._logMode, self._logFile, "Parsing Error: The file '"+self._responsesFile+"' must contain one 'name' element for each 'state' node.")			
			if name[0].lastChild == None:
				error(self._logMode, self._logFile, "Parsing Error: The file '"+self._responsesFile+"' must contain a value for the 'name' elements.")							
			if self._commandsIds.count(id) > 0:
				respOk = self.searchChildNodes(i,'response-ok')
				respErr = self.searchChildNodes(i,'response-err')
				# For each good response
				for j in respOk:
					action = j.getAttribute('action')
					last = self.checkAttribute(j, 'last', None, permittedLastValues)
					next = j.getAttribute('next')
					listOk.append([j.lastChild.nodeValue,action,last,next])
				# For each erroneus response
				for j in respErr:
					action = j.getAttribute('action')
				 	last = self.checkAttribute(j, 'last', None, permittedLastValues)
				 	next = j.getAttribute('next')
					listErr.append([j.lastChild.nodeValue,action,last,next])
				responsesList[id] = [listOk,listErr]
		return responsesList
				
		
				
			
	def searchXmlElementByAttr(self, parent, attribute, attrValue):
		'''
			Method to find an element in the xml parent element by the value of one of the attributes
			
			@arg parent: element we must search
			@type parent: xml element
			
			@arg attribute: attribute to check
			@type attribute: string
			
			@arg attrValue: value of the attribute, filter of search
			@type attrValue: string
			
			@return: the xml element finded
			@rtype: xml element
		'''
		
		value = None
		# Searching the parent tree for the value of the element with the given attribute
		for child in self.searchChildNodes(parent, None):
			if self.checkAttribute(child, attribute, attrValue, None, None, 1) == True:
				if child.lastChild != None:
					value = child.lastChild.nodeValue
				else:
					error(self._logMode, self._logFile, "Parsing Error: The file '"+self._actualFile+"' must contain a value for the '"+child.nodeName+"' element.")							
			else:
				# Recursive call
				value = self.searchXmlElementByAttr(child, attribute, attrValue)
		return value
	
	
	def searchChildNodes(self, parent, childName):
		'''
			Method which returns the child elements of an xml element

			@arg parent: element we must search
			@type parent: xml element
			
			@arg childName: name of node we want to find
			@type childName: string
			
			@return: the xml element finded
			@rtype: xml element
		'''
		
		returnChilds = []
		childs = parent.childNodes
		for i in childs:
			# If Element type
			if i.nodeType == 1:
				if childName == None:
					returnChilds.append(i)
				else:
					if i.nodeName == childName:
						returnChilds.append(i)
		return returnChilds
	
	
	def checkAttribute(self, element, attribute, value = None, permittedValues = None, sep = None, optional = 0):
		'''
			Method to check and/or obtain an attribute of an xml element

			@arg element: element to check
			@type element: xml element
			
			@arg attribute: name of attribute to check
			@type attribute: string

			@arg value: if specified we'll check the real value of the attribute with this value
			@type value: string
			
			@arg permittedValues: a list of permitted values for an attribute
			@type permittedValues: list
			
			@arg sep: character for separate more than one value for an attribute
			@type sep: string
			
			@arg optional: variable to say if the attribute is optional or mandatory
			@type optional: integer
						
			@return: the value of the attribute or a boolean value indicating if the value is ok 
			@rtype: string or boolean
		'''
		
		attributeValue = element.getAttribute(attribute)
		if attributeValue != "":	
			# If the value of the attribute exists			
			if permittedValues != None:
				# Splitting the attribute value 
				if sep != None:
					attributeValues = attributeValue.split(sep)		
				else:
					attributeValues = [attributeValue]
				# Checking if each separated value is correct
				for i in attributeValues:
					if permittedValues.count(i) == 0:
						error(self._logMode, self._logFile, "Parsing Error: The element '"+element.nodeName+"' of the file '"+self._actualFile+"' have not a valid value ('"+i+"') for the attribute '"+attribute+"'.")				
					elif permittedValues.count(i) > 1:
						error(self._logMode, self._logFile, "Parsing Error: The element '"+element.nodeName+"' of the file '"+self._actualFile+"' contains repeated values for the attribute '"+attribute+"'.")				
			if value != None:
				# Checking if the attribute value is the same as the parameter
				if attributeValue == value:
					return True
				else:
					return False
			else:
				return str(attributeValue)
		else:
			# Value for the attribute not present
			if not optional:
				error(self._logMode, self._logFile, "Parsing Error: The element '"+element.nodeName+"' of the file '"+self._actualFile+"' have not a value for the mandatory attribute '"+attribute+"'.")
			else:
				return str(attributeValue)
	
	
	def checkCommands(self, element, commandAttribute, id):
		'''
			Method to check if the 'id' parameter is in the value of the 'commandAttribute' attribute 
			
			@arg element: element to check
			@type element: xml element
			
			@arg commandAttribute: name of attribute related to commands
			@type commandAttribute: string
			
			@arg id: command identification
			@type id: string
						
			@return: a boolean value indicating if 'id' is in 'commandAttribute'
			@rtype: boolean
		'''
		
		commands = element.getAttribute(commandAttribute)
		commands = commands.split(",")
		if commands == [''] or commands.count(id) == 1:
			return True
		elif commands.count(id) > 1:
			error(self._logMode, self._logFile, "Parsing Error: The file '"+self._actualFile+"' contains a repeated value in the attribute '"+commandAttribute+"' of a '"+element.nodeName+"' element.")
		else:
			return False


	def getPort(self):
		'''
			Method to obtain the default port of the protocol
			
			@return: default protocol port 
			@rtype: string
		'''
		xmlRoot = self.searchChildNodes(self._specifXml, 'protocol')
		if len(xmlRoot) == 0:
			error(self._logMode, self._logFile, "Parsing Error: The root node in the file '"+self._commandsFile+"' must be 'protocol'.")			
		if len(xmlRoot) > 1:
			error(self._logMode, self._logFile, "Parsing Error: The root node in the file '"+self._commandsFile+"' must be unique.")						
		portNode = self.searchChildNodes(xmlRoot[0], 'port')
		if len(portNode) == 0 or len(portNode) > 1:
			error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain one 'port' element in the 'protocol' node.")			
		if portNode[0].lastChild != None or not portNode[0].lastChild.nodeValue.isdigit():
			return str(portNode[0].lastChild.nodeValue)
		else:
			error(self._logMode, self._logFile, "Parsing Error: The file '"+self._commandsFile+"' must contain a numeric value for the 'port' element.")										


	def getElementValue(self,value):
		'''
			Method to check if the value is a variable: an element passed by command line, a function applied
			to another element in the xml file, or an element in the xml file.
			
			@arg value: element value to check
			@type value: string
			
			@return: the real value if the parameter passed is a varable.If not,the same value.
			@rtype: string
		'''
		
		variableType = self.isXmlVariable(value)
		if variableType == self.XML_ELEMENT:
			return self.getXmlElementValue(value)
		elif variableType == self.XML_VARIABLE:
			return self.getVariableValue(value)
		elif variableType == self.XML_FUNCTION:
			return self.getFunctionValue(value)
		else:
			return value


	def isXmlVariable(self,value):
		'''
			Method to check if the value is an xml variable 

			@arg value: element value to check
			@type value: string
			
			@return: the type of variable or false if is not an xml variable
			@rtype: boolean or integer
		'''
		if len(value) > 3:
			prefix = value[:3]
			if prefix == '$X:':
				return self.XML_ELEMENT
			elif prefix == '$V:':
				return self.XML_VARIABLE
			elif prefix == '$F:':
				return self.XML_FUNCTION
			else:
				return False
			
		
	def getXmlElementValue(self,variable):
		'''
			Method to obtain the value of an xml element with the variable passed

			@arg variable: element value to check
			@type variable: string
			
			@return: the real value of the variable
			@rtype: string
		'''
		
		value = variable[3:]
		end = value[value.find('$')+1:]
		value = value[:value.find('$')]
		# The element must be marked by an attribute 'name': name="BODY"
		returnValue = self.searchXmlElementByAttr(self._actualRoot, 'name', value)
		if returnValue == None:
			error(self._logMode, self._logFile, "Parsing Error: The file '"+self._actualFile+"' don't contain an element with '"+value+"' like attribute 'name'.")													
		if not returnValue[-1].isalnum():
			returnValue = returnValue[:-1]+end
		if self._logMode:
				self._logFile.debug("Getting value for xml element with '"+value+"' like attribute 'name': "+returnValue)
		return returnValue
			
			
	def getVariableValue(self,variable):
		'''
			Method to obtain the value of an internal variable with the variable passed

			@arg variable: element value to check
			@type variable: string
			
			@return: the real value of the variable
			@rtype: string
		'''
		
		value = variable[3:]
		end = value[value.find('$')+1:]
		value = value[:value.find('$')]
		possibleVariables = self._xmlVariables.keys()
		if possibleVariables.count(value) == 1:
			if self._logMode:
				self._logFile.debug("Getting xml variable '"+value+"': "+self._xmlVariables[value])
			return self._xmlVariables[value]+end
		else:
			error(self._logMode, self._logFile, "Parsing Error: The variable '"+variable+"' in the file '"+self._actualFile+"' is not valid.")
					
					
	def getFunctionValue(self,variable):
		'''
			Method to obtain the result of an xml function with the variable passed

			@arg variable: element value to check
			@type variable: string
			
			@return: the real value of the variable
			@rtype: string
		'''
		
		value = variable[3:]
		end = value[value.rfind('$')+1:]
		value = value[:value.rfind('$')]
		function = value[:value.find('(')]
		# List of possible functions to call
		possibleFunctions = self.XML_FUNCTIONS.keys()
		if possibleFunctions.count(function) == 1:
			# Getting the function object
			function = eval(self.XML_FUNCTIONS[function])
			# Getting the argument values
			arguments = value[value.find('(')+1:value.find(')')]
			arguments = arguments.split(',')
			for i in range(len(arguments)):
				arguments[i] = self.getElementValue(arguments[i])
			returnValue = function(arguments)
			if self._logMode:
				self._logFile.debug("Getting value for xml function '"+value+"': "+returnValue)
			return returnValue+end
		else:
			error(self._logMode, self._logFile, "Parsing Error: The function '"+variable+"' in the file '"+self._actualFile+"' is not valid.")		
	
	
	
	#############
	#  Getters  #
	#############
			
	def getCommandsFile(self):
		return self._specifXml
	
	def getStatesFile(self):
		return self._statesXml
	
	def getCommandsIds(self):
		return self._commandsIds

	def getXmlVariables(self):
		return self._xmlVariables

	#############
	#  Setters  #
	#############
	
	def setCommandsFile(self, specifXml):
		self._specifXml = specifXml
	
	def setStatesFile(self, statesXml):
		self._statesXml = statesXml
	
	def setCommandsIds(self,commandsIds):
		self._commandsIds = commandsIds

	def setXmlVariables(self,xmlVariables):
		self._xmlVariables = xmlVariables