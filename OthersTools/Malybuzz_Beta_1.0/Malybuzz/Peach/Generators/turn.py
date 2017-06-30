
'''
[BETA] TURN protocol generator.  TURN is an extension of the STUN
protocol.  TURN packets are generated using the StunPacket top
level container with TURN message types and attributes as needed.

@author: Michael Eddington
@version: $Id: turn.py 279 2007-05-08 01:21:58Z meddingt $
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

# $Id: turn.py 279 2007-05-08 01:21:58Z meddingt $

import re, struct, os
from Peach							import generator
from Peach.Generators				import *
from Peach.Generators.dictionary	import *
from Peach.Generators.static		import *
from Peach.Generators.static		import Int8, Int16, Int32
from Peach.Generators.stun			import *

#__all__ = ['TurnAttribute',
#		   'TurnHeader',
#		  ]

class TurnHeader(StunHeader):
	'''
	This generator creates a TURN header.  A TURN header
	is 20 bytes long.
	'''
	
	ALLOCATE_REQUEST				= 0x0003
	ALLOCATE_RESPONSE				= 0x0103
	ALLOCATE_ERROR_RESPONSE			= 0x0113
	SEND_REQUEST					= 0x0004
	SEND_RESPONSE					= 0x0104
	SEND_ERROR_RESPONSE				= 0x0114
	DATA_INDICATION					= 0x0115
	SET_ACTIVE_DEST_REQUEST			= 0x0006
	SET_ACTIVE_DEST_RESPONSE		= 0x0106
	SET_ACTIVE_DEST_ERROR_RESPONSE	= 0x0116


class TurnAttribute(StunAttribute):
	'''
	This generator creates a STUN attribute.
	'''
	
	LIFETIME = 0x000d
	ALTERNATE_SERVER = 0x000e
	MAGIC_COOKIE = 0x000f
	BANDWIDTH = 0x0010
	DESTINATION_ADDRESS = 0x0011
	REMOTE_ADDRESS = 0x0012
	DATA = 0x0013
	NONCE = 0x0014
	REALM = 0x0015
	NAT_ADDRESS = 0x0080


class TurnNatAddressAttribute(StunMappedAddressAttribute):
	'''
	
	IP Family 1 byte
	IP Port 2 bytes
	IP Address 4 bytes
	
	'''
	
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
		StunMappedAddressAttribute.__init__(self, group, port, address, family);
		self._type = TurnAttribute.NAT_ADDRESS
	

class TurnLifetimeAttribute(TurnAttribute):
	'''
	LIFETIME
	
	The lifetime attribute represents the duration for which the server
	will maintain a binding in the absence of data traffic either from or
	to the client.  It is a 32 bit value representing the number of
	seconds remaining until expiration.
	
	 0                   1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                        Lifetime                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	
	'''
	
	def __init__(self, group, lifetime):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	lifetime: Generator
		@param	lifetime: Lifetime in seconds
		'''
		self.setGroup(group)
		if lifetime != None:
			self._generator = lifetime
		self._type = TurnAttribute.LIFETIME
	
	def _getValue(self):
		return self._generator.getValue()


class TurnAlternateServerAttribute(TurnAttribute, StunMappedAddressAttribute):
	'''
	ALTERNATE-SERVER
	
	The alternate server represents an alternate IP address and port for
	a different TURN server to try.  It is encoded in the same way as
	MAPPED-ADDRESS.
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
		self._type = TurnAttribute.ALTERNATE_SERVER


class TurnMagicCookieAttribute(TurnAttribute):
	'''
	MAGIC-COOKIE
	
	The MAGIC-COOKIE is used by TURN clients and servers to disambiguate
	TURN traffic from data traffic.  Its value ix 0x72c64bc6.
	
	 0                   1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|0|1|1|1|0|0|1|0|1|1|0|0|0|1|1|0|0|1|0|0|1|0|1|1|1|1|0|0|0|1|1|0|
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	'''
	
	_magicCookie = 0x72c64bc6	# 4 byte
	
	def __init__(self, group, magicCookie = None):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	magicCookie: Generator
		@param	magicCookie: Magic cookie, default is 0x72c64bc6
		'''
		self.setGroup(group)
		if lifetime != None:
			self._lifetime = lifetime
		self._type = TurnAttribute.MAGIC_COOKIE
	
	def _getValue(self):
		if isinstance(self._magicCookie, Generator):
			return Int32(self._magicCookie.getValue(), 0, 0).getValue()
		
		return Int32(self._magicCookie.getValue(), 0, 0).getValue()


class TurnBandwidthAttribute(TurnAttribute):
	'''
	BANDWIDTH
	
	The bandwidth attribute represents the peak bandwidth, measured in
	kbits per second, that the client expects to use on the binding.  The
	value represents the sum in the receive and send directions.
	[[Editors note: Need to define leaky bucket parameters for this.]]
		
	 0                   1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                        Bandwidth                              |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	'''
	
	def __init__(self, group, bandwidth):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	lifetime: Generator
		@param	lifetime: Lifetime in seconds
		'''
		self.setGroup(group)
		if bandwidth != None:
			self._generator = bandwidth
		self._type = TurnAttribute.BANDWIDTH
	
	def _getValue(self):
		return self._generator.getValue()


class TurnDestinationAddressAttribute(StunMappedAddressAttribute):
	'''
	DESTINATION-ADDRESS
	
	The DESTINATION-ADDRESS is present in Send Requests and Set Active
	Destination Requests.  It specifies the address and port where the
	data is to be sent.  It is encoded in the same way as MAPPED-ADDRESS.
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
		self._type = TurnAttribute.DESTINATION_ADDRESS


class TurnRemoteAddressAttribute(StunMappedAddressAttribute):
	'''
	REMOTE-ADDRESS
	
	The REMOTE-ADDRESS is present in Data Indications.  It specifies the
	address and port from which a packet was received.  It is encoded in
	the same way as MAPPED-ADDRESS.
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
		self._type = TurnAttribute.REMOTE_ADDRESS


class TurnDataAttribute(TurnAttribute):
	'''
	DATA
	
	The DATA attribute is present in Send Requests and Data Indications.
	It contains raw payload data that is to be sent (in the case of a
	Send Request) or was received (in the case of a Data Indication).
	'''
	
	def __init__(self, group, data):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	data: Generator
		@param	data: Data to send through
		'''
		self.setGroup(group)
		if data != None:
			self._generator = data
		self._type = TurnAttribute.DATA
	
	def _getValue(self):
		return self._generator.getValue()


class TurnNonceAttribute(TurnAttribute):
	'''
	NONCE
	
	The NONCE attribute is present in Shared Secret Requests and Shared
	Secret Error responses.  It contains a sequence of qdtext or quoted-
	pair, which are defined in [6].
	'''
	
	_nonce = 0	# 4 byte
	
	def __init__(self, group, nonce):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	nonce: Generator
		@param	nonce: Nonce stuff
		'''
		self.setGroup(group)
		if nonce != None:
			self._nonce = nonce
		self._type = TurnAttribute.NONCE
	
	def _getValue(self):
		return self._nonce.getValue()


class TurnRealmAttribute(TurnAttribute):
	'''
	REALM
	
	The REALM attribute is present in Shared Secret Requests and Shared
	Secret Responses.  It contains text which meets the grammar for
	"realm" as described in RFC 3261, and will thus contain a quoted
	string (including the quotes).
	'''
	
	_realm = None
	
	def __init__(self, group, realm):
		'''
		@type	group: Group
		@param	group: Group to use
		@type	realm: Generator
		@param	realm: Realm string ("fooyah.com")
		'''
		self.setGroup(group)
		if realm != None:
			self._realm = realm
		self._type = TurnAttribute.REALM
	
	def _getValue(self):
		return self._realm.getValue()


def unittest():
	pass

# end
