
'''
[BETA] STUN protocol generator

@author: Michael Eddington
@version: $Id: stun.py 279 2007-05-08 01:21:58Z meddingt $
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

# $Id: stun.py 279 2007-05-08 01:21:58Z meddingt $

import re, struct, os
from Peach.generator import Generator
from Peach.Generators  import *
from Peach.Generators.dictionary  import *
from Peach.Generators.static  import *
from Peach.Generators.static  import Int8, Int16, Int32

#__all__ = ['StunAttribute',
#		   'StunHeader',
#		   'StunPayload',
#		   'StunPacket',
#		   'StunMappedAddressAttribute',
#		   'StunResponseAddressAttribute',
#		   'StunChangedAddressAttribute',
#		   'StunChangeRequestAttribute',
#		   'StunSourceAddressAttribute',
#		   'StunUsernameAttribute',
#		   'StunPasswordAttribute',
#		   'StunMessageIntegrityAttribute',
#		   'StunErrorCodeAttribute',
#		   'StunUnknownAttribute',
#		   'StunReflectedFromAttribute',
#		   'StunTransactionId']

class StunPayload(block.Block2):
	'''
	This generator creates a STUN payload.  A payload
	is a group of attributes
	'''
	
	pass


class StunPacket(Generator):
	'''
	This generator creats a STUN packet
	'''
	
	_header = None
	_payload = None
	
	def __init__(self, group, header, payload):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	header: StunHeader
		@param	header: Header generator
		@type	payload: StunPayload
		@param	payload: Payload generator
		'''
		self.setGroup(group)
		self._header = header
		self._payload = payload
	
	def next(self):
		done = 1
		
		try:
			self._header.next()
			done = 0
		except generator.GeneratorCompleted:
			pass
		
		try:
			self._payload.next()
			done = 0
		except generator.GeneratorCompleted:
			pass
		
		if done:
			raise generator.GeneratorCompleted("StunPacket")
	
	def getRawValue(self):
		return self._header.getValue() + self._payload.getValue()
	
	def getHeader(self):
		return self._header
	
	def setHeader(self, header):
		self._header = header
	
	def getPayload(self):
		return self._payload
	
	def setPayload(self, payload):
		self._payload = payload


class StunAttribute(SimpleGenerator):
	'''
	This generator creates a STUN attribute.
	'''
	
	MAPPED_ADDRESS		= 0x0001
	RESPONSE_ADDRESS	= 0x0002
	CHANGE_REQUEST		= 0x0003
	SOURCE_ADDRESS		= 0x0004
	CHANGED_ADDRESS		= 0x0005
	USERNAME			= 0x0006
	PASSWORD			= 0x0007
	MESSAGE_INTEGRITY	= 0x0008
	ERROR_CODE			= 0x0009
	UNKNOWN_ATTRIBUTES	= 0x000a
	REFLECTED_FROM		= 0x000b
	
	_type	= None	# 2 bytes
	_length	= None	# 2 bytes
	_generator	= None
	
	def __init__(self, group, type, length, value):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	type: integer
		@param	type: Attribute type
		@type	length: integer
		@param	length: If None length is auto based on value
		@type	value: Generator
		@param	value: Attribute value
		'''
		self.setGroup(group)
		if type != None:
			self._type = type
		if length != None:
			self._length = length
		if value != None:
			self._generator = value
	
	def _getValue(self):
		'''
		Override me!
		'''
		return self._generator.getValue()
	
	def getRawValue(self):
		value = self._getValue()
		
		if self._length == None:
			length = len(value)
		else:
			length = self._length
		
		return Int16(self._type, 0, 0).getValue() + Int16(length, 0, 0).getValue() + value


class StunMappedAddressAttribute(StunAttribute):
	'''
	MAPPED-ADDRESS
	
	The MAPPED-ADDRESS attribute indicates the mapped IP address and
	port.  It consists of an eight bit address family, and a sixteen bit
	port, followed by a fixed length value representing the IP address.
	
	 0                   1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|x x x x x x x x|    Family     |           Port                |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                             Address                           |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	
	The port is a network byte ordered representation of the mapped port.
	The address family is always 0x01, corresponding to IPv4.  The first
	8 bits of the MAPPED-ADDRESS are ignored, for the purposes of
	aligning parameters on natural boundaries.  The IPv4 address is 32
	bits.
	'''
	
	_empty = 0		# 1 byte
	_family = 0x01	# 1 byte
	_port = None	# 2 bytes
	_address = None	# 3 bytes
	
	def __init__(self, group, port, address, family = Static('\01')):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	family: Generator
		@param	family: Family (defaults to 0x01 IPV4)
		@type	port: Generator
		@param	port: Port number
		@type	address: Generator
		@param	address: 32bit IPv4 address
		'''
		self.setGroup(group)
		if port != None:
			self._port = port
		if address != None:
			self._address = address
		if family != None:
			self._family = family
		self._type = StunAttribute.MAPPED_ADDRESS
	
	def _getValue(self):
		return "%s%s%s" % (	  self._family.getValue(),
							  self._port.getValue(),
							  self._address.getValue() )
	
	def reset(self):
		self._port.reset()
		self._address.reset()
		self._family.reset()
		
	def next(self):
		done = 1
		
		try:
			self._family.next()
			done = 0
		except generator.GeneratorCompleted:
			pass
		
		try:
			self._port.next()
			done = 0
		except generator.GeneratorCompleted:
			pass
		
		try:
			self._address.next()
			done = 0
		except generator.GeneratorCompleted:
			pass
		
		if done:
			raise generator.GeneratorCompleted("StunMappedAddressAttribute")


class StunResponseAddressAttribute(StunMappedAddressAttribute):
	'''
	RESPONSE-ADDRESS
	
	The RESPONSE-ADDRESS attribute indicates where the response to a
	Binding Request should be sent.  Its syntax is identical to MAPPED-
	ADDRESS.
	'''
	
	def __init__(self, group, port, address, family = 0x01):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	family: Generator
		@param	family: Family (defaults to 0x01 IPV4)
		@type	port: Generator
		@param	port: Port number
		@type	address: Generator
		@param	address: 32bit IPv4 address
		'''
		StunMappedAddressAttribute.__init__(self, group, port, address, family)
		self._type = StunAttribute.RESPONSE_ADDRESS
		

class StunChangedAddressAttribute(StunMappedAddressAttribute):
	'''
	CHANGED-ADDRESS
	
	The CHANGED-ADDRESS attribute indicates the IP address and port where
	responses would have been sent from if the "change IP" and "change
	port" flags had been set in the CHANGE-REQUEST attribute of the
	Binding Request.  The attribute is always present in a Binding
	Response, independent of the value of the flags.  Its syntax is
	identical to MAPPED-ADDRESS.
	'''
	
	def __init__(self, group, port, address, family = 0x01):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	family: Generator
		@param	family: Family (defaults to 0x01 IPV4)
		@type	port: Generator
		@param	port: Port number
		@type	address: Generator
		@param	address: 32bit IPv4 address
		'''
		StunMappedAddressAttribute.__init__(self, group, port, address, family)
		self._type = StunAttribute.CHANGED_ADDRESS


class StunChangeRequestAttribute(StunAttribute):
	'''
	CHANGE-REQUEST
	
	The CHANGE-REQUEST attribute is used by the client to request that
	the server use a different address and/or port when sending the
	response.  The attribute is 32 bits long, although only two bits (A
	and B) are used:
	
	 0                   1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 A B 0|
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	
	The meaning of the flags is:
	
	A: This is the "change IP" flag.  If true, it requests the server
	   to send the Binding Response with a different IP address than the
	   one the Binding Request was received on.
	
	B: This is the "change port" flag.  If true, it requests the
	   server to send the Binding Response with a different port than the
	   one the Binding Request was received on.
	'''
	
	CHANGE_IP = 4
	CHANGE_PORT = 2
	
	_value = None
	_changeIp = None
	_changePort = None
	
	def __init__(self, group, changeIp, changePort, value = None):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	changeIp: Generator
		@param	changeIp: changeIp value (0 or 4)
		@type	changePort: Generator
		@param	changePort: changePort value (0 or 2)
		@type	value: Generator
		@param	value: 32bit value (optional)
		'''
		self.setGroup(group)
		if changeIp != None:
			self._changeIp = changeIp
		if changePort != None:
			self._changePort = changePort
		if value != None:
			self._value = value
		self._type = StunAttribute.CHANGE_REQUEST
	
	def _getValue(self):
		if self._value == None:
			value = int(self._changeIp.getValue()) | int(self._changePort.getValue())
			
		return Int32(value, 0, 0).getValue()
	
	def next(self):
		done = 1
		
		try:
			self._changeIp.next()
			done = 0
		except generator.GeneratorCompleted:
			pass

		try:
			self._changePort.next()
			done = 0
		except generator.GeneratorCompleted:
			pass

		if self._value:
			try:
				self._value.next()
				done = 0
			except generator.GeneratorCompleted:
				pass
		
		if done:
			raise generator.GeneratorCompleted("StunChangeRequestAttribute")


class StunSourceAddressAttribute(StunMappedAddressAttribute):
	'''
	SOURCE-ADDRESS
	
	The SOURCE-ADDRESS attribute is present in Binding Responses.  It
	indicates the source IP address and port that the server is sending
	the response from.  Its syntax is identical to that of MAPPED-
	ADDRESS.
	'''

	def __init__(self, group, port, address, family = 0x01):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	family: Generator
		@param	family: Family (defaults to 0x01 IPV4)
		@type	port: Generator
		@param	port: Port number
		@type	address: Generator
		@param	address: 32bit IPv4 address
		'''
		StunMappedAddressAttribute.__init__(self, group, port, address, family)
		self._type = StunAttribute.SOURCE_ADDRESS


class StunUsernameAttribute(StunAttribute):
	'''
	USERNAME
	
	The USERNAME attribute is used for message integrity.  It serves as a
	means to identify the shared secret used in the message integrity
	check.  The USERNAME is always present in a Shared Secret Response,
	along with the PASSWORD.  It is optionally present in a Binding
	Request when message integrity is used.
	
	The value of USERNAME is a variable length opaque value.  Its length
	MUST be a multiple of 4 (measured in bytes) in order to guarantee
	alignment of attributes on word boundaries.
	'''
	
	def __init__(self, group, username):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	username: Generator
		@param	username: Username
		'''
		self.setGroup(group)
		if username != None:
			self._value = username
		self._type = StunAttribute.USERNAME


class StunPasswordAttribute(StunAttribute):
	'''
	PASSWORD
	
	The PASSWORD attribute is used in Shared Secret Responses.  It is
	always present in a Shared Secret Response, along with the USERNAME.
	
	The value of PASSWORD is a variable length value that is to be used
	as a shared secret.  Its length MUST be a multiple of 4 (measured in
	bytes) in order to guarantee alignment of attributes on word
	boundaries.
	'''
	
	def __init__(self, group, password):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	password: Generator
		@param	password: Password
		'''
		self.setGroup(group)
		if value != None:
			self._value = username
		self._type = StunAttribute.PASSWORD


class StunMessageIntegrityAttribute(StunAttribute):
	'''
	MESSAGE-INTEGRITY
	
	The MESSAGE-INTEGRITY attribute contains an HMAC-SHA1 [13] of the
	STUN message.  It can be present in Binding Requests or Binding
	Responses.  Since it uses the SHA1 hash, the HMAC will be 20 bytes.
	The text used as input to HMAC is the STUN message, including the
	header, up to and including the attribute preceding the MESSAGE-
	INTEGRITY attribute. That text is then padded with zeroes so as to be
	a multiple of 64 bytes.  As a result, the MESSAGE-INTEGRITY attribute
	MUST be the last attribute in any STUN message.  The key used as
	input to HMAC depends on the context.
	'''
	
	def __init__(self, group, hmac):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	hmac: Generator
		@param	hmac: HMAC-SHA1
		'''
		self.setGroup(group)
		if value != None:
			self._value = hmac
		self._type = StunAttribute.MESSAGE_INTEGRITY


class StunErrorCodeAttribute(StunAttribute):
	'''
	ERROR-CODE
	
	The ERROR-CODE attribute is present in the Binding Error Response and
	Shared Secret Error Response.  It is a numeric value in the range of
	100 to 699 plus a textual reason phrase encoded in UTF-8, and is
	consistent in its code assignments and semantics with SIP [10] and
	HTTP [15].  The reason phrase is meant for user consumption, and can
	be anything appropriate for the response code.  The lengths of the
	reason phrases MUST be a multiple of 4 (measured in bytes).  This can
	be accomplished by added spaces to the end of the text, if necessary.
	Recommended reason phrases for the defined response codes are
	presented below.
	
	To facilitate processing, the class of the error code (the hundreds
	digit) is encoded separately from the rest of the code.
	
	  0                   1                   2                   3
	  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 |                   0                     |Class|     Number    |
	 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	 |      Reason Phrase (variable)                                ..
	 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	
	The class represents the hundreds digit of the response code.  The
	value MUST be between 1 and 6.  The number represents the response
	code modulo 100, and its value MUST be between 0 and 99.
	
	The following response codes, along with their recommended reason
	phrases (in brackets) are defined at this time:
	
	400 (Bad Request): The request was malformed.  The client should not
		 retry the request without modification from the previous
		 attempt.
	
	401 (Unauthorized): The Binding Request did not contain a MESSAGE-
		 INTEGRITY attribute.
	
	420 (Unknown Attribute): The server did not understand a mandatory
		 attribute in the request.
	
	430 (Stale Credentials): The Binding Request did contain a MESSAGE-
		 INTEGRITY attribute, but it used a shared secret that has
		 expired.  The client should obtain a new shared secret and try
		 again.
	
	431 (Integrity Check Failure): The Binding Request contained a
		 MESSAGE-INTEGRITY attribute, but the HMAC failed verification.
		 This could be a sign of a potential attack, or client
		 implementation error.
	
	432 (Missing Username): The Binding Request contained a MESSAGE-
		 INTEGRITY attribute, but not a USERNAME attribute.  Both must be
		 present for integrity checks.
	
	433 (Use TLS): The Shared Secret request has to be sent over TLS, but
		 was not received over TLS.
	
	500 (Server Error): The server has suffered a temporary error. The
		 client should try again.
	
	600 (Global Failure:) The server is refusing to fulfill the request.
		 The client should not retry.
	'''
	
	pass

class StunUnknownAttribute(StunAttribute):
	pass

class StunReflectedFromAttribute(StunMappedAddressAttribute):
	'''
	REFLECTED-FROM
	
	The REFLECTED-FROM attribute is present only in Binding Responses,
	when the Binding Request contained a RESPONSE-ADDRESS attribute.  The
	attribute contains the identity (in terms of IP address) of the
	source where the request came from.  Its purpose is to provide
	traceability, so that a STUN server cannot be used as a reflector for
	denial-of-service attacks.
	
	Its syntax is identical to the MAPPED-ADDRESS attribute
	'''
	
	def __init__(self, group, port, address, family = 0x01):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	family: Generator
		@param	family: Family (defaults to 0x01 IPV4)
		@type	port: Generator
		@param	port: Port number
		@type	address: Generator
		@param	address: 32bit IPv4 address
		'''
		StunMappedAddressAttribute.__init__(self, group, port, address, family)
		self._type = StunAttribute.REFLECTED_FROM
	

class StunTransactionId(Generator):
	'''
	Generate a valid transaction id
	'''
	
	_tid = None
	
	def __init__(self, group):
		self.setGroup(group)
		self._tid = self.getRandomTID()
	
	def getRandomTID(self):
		return os.urandom(16)
	
	def next(self):
		self._tid = self.getRandomTID()
	
	def getRawValue(self):
		return self._tid
	
class StunHeader(Generator):
	'''
	This generator creates a STUN header.  A STUN header
	is 20 bytes long.
	'''
	
	BINDING_REQUEST					= 0x0001
	BINDING_RESPONSE				= 0x0101
	BINDING_ERROR_RESPONSE			= 0x0111
	SHARED_SECRET_REQUEST			= 0x0002
	SHARED_SECRET_RESPONSE			= 0x0102
	SHARED_SECRET_ERROR_RESPONSE	= 0x0112
	
	_messageType = None		# 2 bytes
	_length = None			# 2 bytes
	_transactionId = None	# 18 bytes
	
	def __init__(self, group, messageType, transactionId, length):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	messageType: Generator
		@param	messageType: Message type we are generating
		@type	transactionId: Generator
		@param	transactionId: Transaction Id
		@type	length: Generator
		@param	length: Length of STUN payload
		'''
		
		self.setGroup(group)
		if messageType != None:
			self._messageType = messageType
		if transactionId != None:
			self._transactionId = transactionId
		if length != None:
			self._length = length
	
	def getRawValue(self):
		return Int16(self._messageType.getValue(), 0, 0) + Int16(self._length.getValue(), 0, 0).getValue() + self._transactionId.getValue()
	
	def reset(self):
		pass
	
	def unittest():
		pass
	unittest = staticmethod(unittest)


def unittest():
	pass


# end
