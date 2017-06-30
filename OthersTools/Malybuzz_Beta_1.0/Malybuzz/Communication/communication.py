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
	communication.py
	
	Communication module  
'''

import logging

from Peach.Publishers	import *
from Utils.misc import error, timeTest

class CommunicationManager:
	'''
		Class for the management of the communications
	'''
	
	_connection = None
	_socket = None
	_localAddress = None
	_localPort = None
	_remoteAddress = None
	_remotePort = None
	_transportProtocol = None
	_applicationProtocol = None
	_connectionData = None
	_logMode = None
	_logFile = None
	
	def __init__(self, targetHost, targetPort, transportProtoOptions):
		'''
			Initial method
			
			@arg targetHost: remote host address to connect
			@type targetHost: string
			
			@arg targetPort: remote port
			@type targetPort: integer
			
			@arg transportProtoOptions: configuration parameters of the connection
			@type transportProtoOptions: list
		'''
		
		hostResponse = ""
		self.setTransportProtocol(transportProtoOptions[0])
		self.setApplicationProtocol(transportProtoOptions[1])
		self._logMode = transportProtoOptions[5]
		
		try:
			if self._logMode:
				self._logFile = logging.getLogger("CommunicationManager")
				
			if self._transportProtocol == 'TCP':
				self._connection = tcp.Tcp(targetHost,targetPort,transportProtoOptions)
				
			elif self._transportProtocol == 'UDP':
				self._connection = udp.Udp(targetHost,targetPort,transportProtoOptions)
			
			# Checking the time while connecting the target	
			time = timeTest(targetHost, targetPort, self._transportProtocol)
			if time == -1:
				error(self._logMode, self._logFile, "Communication Error: Some kind of problem before sending commands to remote host")
			
			if self._transportProtocol == 'TCP':
				hostResponse = "Host responding in "+str(time)+" ms"+"\n"
			if self._logMode:
				self._logFile.info("Host responding in "+str(time)+" ms")
			
			# Starting the connection
			self.setConnectionData(hostResponse+self._connection.start())
		
			# Setting the variables values depending on the connection data
			self._socket = self._connection.getSocket()
			networkLocalData = self._socket.getsockname()
			if networkLocalData != []:
				self.setLocalAddress(networkLocalData[0])
				self.setLocalPort(networkLocalData[1])
			self.setRemoteAddress(targetHost)
			self.setRemotePort(targetPort)
			
		except:
			raise

		
	#############
	#  Getters  #
	#############
		
	def getConnection(self):
		return self._connection
	
	def getSocket(self):
		return self._connection.getSocket()
	
	def getLocalAddress(self):
		return self._localAddress
	
	def getLocalPort(self):
		return self._localPort
			
	def getRemoteAddress(self):
		return self._remoteAddress
		
	def getRemotePort(self):
		return self._remotePort

	def getTransportProtocol(self):
		return self._transportProtocol

	def getApplicationProtocol(self):
		return self._applicationProtocol
	
	def getConnectionData(self):
		return self._connectionData


	#############
	#  Setters  #
	#############
 
	
	def setLocalAddress(self,localAddress):
		self._localAddress = localAddress
	
	def setLocalPort(self,localPort):
		self._localPort = localPort
			
	def setRemoteAddress(self,remoteAddress):
		self._remoteAddress = remoteAddress
		
	def setRemotePort(self,remotePort):
		self._remotePort = remotePort

	def setTransportProtocol(self,transportProtocol):
		self._transportProtocol = transportProtocol

	def setApplicationProtocol(self,applicationProtocol):
		self._applicationProtocol = applicationProtocol
		
	def setConnectionData(self,connectionData):
		self._connectionData = connectionData
	