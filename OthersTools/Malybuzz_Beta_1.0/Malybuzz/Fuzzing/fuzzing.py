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
	fuzzing.py
	
	Module to create the fuzzing tests
'''

from Peach import group
from Peach.Generators import *
from Utils.misc import outputConverter

class FuzzingFactory:
	# Default character to repeat
	_charOverflow = "A"
	
	
	def create(self, fuzzType, elementsList, index, delimiter, endLine, type):
		'''
			Method to construct a fuzzing block to generate fuzzing requests
		
			@arg fuzzType: type of fuzzing
			@type fuzzType:  string

			@arg elementsList: all the elements in a requests separated by spaces			
			@type elementsList:  string
			
			@arg index: index of the element where we want to do the fuzzing
			@type index:  integer
			
			@arg delimiter: character to separate the element to fuzz and other elements
			@type delimiter:  string
			
			@arg endLine: character/s of end of line
			@type endLine:  string
			
			@arg type: data type of elemento to fuzz
			@type type:  string
			
			@return: a block to generate the fuzzing requests
			@rtype: Block
		'''
		
		# Block to generate the fuzzing requests
		fuzzingBlock = block.Block()
		# Fuzzing generator
		fuzzingCommand = []
		repeaterGroup = group.Group()
		elements = []
		# Part of the request before the fuzzing element
		commandPartBefore = ""
		# Part of the request after the fuzzing element
		commandPartAfter = ""
		# Command id
		id = elementsList[0]
		elementsList = elementsList[1:]
		
		try:
			# Parsing carriage return,new line and spaces characters
			lines = elementsList.split(endLine)
			prev = 0
			next = 0
			# Inserting an end-line character after each line of the request
			for i in range(len(lines)-1):
				if prev == 0:
					next = 1
				else:
					next = prev + 2
				lines.insert(next,endLine)
				prev = next
			# Obtaining lines without spaces
			for i in range(len(lines)):
				if i%2 == 0:
					elements += lines[i].split()
				else:
					elements += [endLine]
			# Replacing special character strings by their real character: "##space##" --> ' ',"##intro##" --> '\r\n'
			for i in range(len(elements)):
				tmp1 = elements[i]
				tmp2 = outputConverter(tmp1,endLine) 
				if tmp1 != tmp2:
					elements.pop(i)
					elements.insert(i,tmp2)
			fuzzingCommand.append(static.Static(id))
			# Setting the part before the fuzzing element
			for i in range(index):
				commandPartBefore += elements[i]
			fuzzingCommand.append(static.Static(commandPartBefore))
			
			# Adding the block depending on th type of fuzzing
			if fuzzType == 'overflow':
				# Creating a block with long chains of the character "charOverflow"
				fuzzingCommand.append(repeater.RepeaterOverflow(repeaterGroup,static.Static(self._charOverflow)))
			
			elif fuzzType == 'formatString':
				# Creating a block with different types of string to obtain an unexpected answer: \n,\r,%n,%x,&," ",'...etc 
				formatStringList = []
				formatStringList.append(data.BadFormatString())
				for i in data.BadFormatString()._strings:
					formatStringList.append(repeater.RepeaterOverflow(repeaterGroup,static.Static(i),5,10))
				fuzzingCommand.append(dictionary.GeneratorList(None,formatStringList))
			
			elif fuzzType == 'badString':
				# Creating a block with different types of string to obtain an unexpected answer: \n,\r,%n,%x,&," ",'...etc 
				fuzzingCommand.append(data.BadString(None))
			
			elif fuzzType == 'badNumber':
				# Creating a block with different bad numbers to obtain an unexpected answer
				fuzzingCommand.append(data.BadNumbers(None))
			
			elif fuzzType == 'badPath':
				# Creating a block with different bad paths to obtain an unexpected answer: ":","..\","..\\"...etc
				fuzzingCommand.append(data.BadPath(None))
			
			elif fuzzType == 'badIp':
				# Creating a block with different bad ip addresses to obtain an unexpected answer: "0.0.0.0","255.255.255.255"...etc
				fuzzingCommand.append(data.BadIpAddress(None))
			
			elif fuzzType == 'badHost':
				fuzzingCommand.append(data.BadHostname(None))
			
			elif fuzzType == 'repeat':
				# Creating a list of blocks repeating each character not alphanumeric of the command/argument several times and
				# each part of the command separated by space: "mail from::::::::::: pepe@hotmail.com","mailmailmailmail from: pepe@hotmail.com"
				repeatBlocksList = []
				# For each character of elemento to fuzz
				for i in range(len(elements[index])):
					# If is not a letter we repeat it
					if not elements[index][i].isalpha():
						repeatBlock = block.Block()
						repeatList = []
						firstPart = elements[index][:i]
						lastPart = elements[index][i+1:]
						repeatList.append(static.Static(firstPart))
						repeatList.append(repeater.Repeater(repeaterGroup,static.Static(elements[index][i]),20,5))
						repeatList.append(static.Static(lastPart))
						repeatBlock.setGenerators(repeatList)
						repeatBlocksList.append(repeatBlock)
				fuzzParts = elements[index].split()
				firstPart = ""
				# For each part of the element separated by spaces, we repeat it
				for i in range(len(fuzzParts)):
					repeatBlock = block.Block()
					repeatList = []
					if firstPart != "":
						repeatList.append(static.Static(firstPart))
					repeatList.append(repeater.Repeater(repeaterGroup,static.Static(fuzzParts[i]),1,10))
					if i < len(fuzzParts)-1:
						for j in range(i+1,len(fuzzParts)):
							repeatList.append(static.Static(fuzzParts[j]+" "))
					repeatBlock.setGenerators(repeatList)
					repeatBlocksList.append(repeatBlock)
					firstPart += fuzzParts[i]+" "
				fuzzingCommand.append(dictionary.GeneratorList(None,repeatBlocksList))
			
			elif fuzzType == 'sql':
				# Creating a block with different sql injections: 
				fuzzingCommand.append(data.SQLInjection(None))
			
			elif fuzzType == 'binary':
				# Creating a block with binary data:
				fuzzingCommand.append(data.HexData())
			
			# Adding the delimiter after the fuzzing element
			fuzzingCommand.append(static.Static(delimiter))
			# Setting the part after the fuzzing element
			for i in range(index+1,len(elements)):
				commandPartAfter += elements[i]
			fuzzingCommand.append(static.Static(commandPartAfter))
			fuzzingBlock.setGenerators(fuzzingCommand)
			return fuzzingBlock
		except:
			raise

		 
