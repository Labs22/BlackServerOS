
'''
Some default file publishers.  Output generated data to a file, etc.

@author: Michael Eddington
@version: $Id: file.py 279 2007-05-08 01:21:58Z meddingt $
'''

#
# Copyright (c) 2005-2006 Michael Eddington
# Copyright (c) 2004-2005 IOActive Inc.
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

# $Id: file.py 279 2007-05-08 01:21:58Z meddingt $

from Peach.publisher import Publisher

#__all__ = ['File', 'FilePerIteration']

class File(Publisher):
	'''
	Publishes generated data to a file.  No concept of receaving data
	yet.
	'''
	
	_filename = None
	_fd = None
	_state = 0	# 0 -stoped; 1 -started
	
	def __init__(self, filename):
		'''
		@type	filename: string
		@param	filename: Filename to write to
		'''
		self.setFilename(filename)
	
	def getFilename(self):
		'''
		Get current filename.
		
		@rtype: string
		@return: current filename
		'''
		return self._filename
	def setFilename(self, filename):
		'''
		Set new filename.
		
		@type filename: string
		@param filename: Filename to set
		'''
		self._filename = filename
	
	def start(self):
		if self._state == 1:
			raise Exception('File::start(): Already started!')
		
		if self._fd != None:
			self._fd.close()
		
		self._fd = open(self._filename, "w+b")
		self._state = 1
	
	def stop(self):
		if self._state == 0:
			raise Exception('File::stop(): Already stopped!')
		
		self._fd.close()
		self._fd = None
		self._state = 0
	
	def send(self, data):
		self._fd.write(data)
	
	def receive(self):
		'''
		Currently not implemented.
		'''
		pass


class FilePerIteration(File):
	'''
	This publisher differs from File in that each round
	will generate a new filename.  Very handy for generating
	bogus content (media files, etc).
	'''
	
	_roundCount = 0
	_origFilename = None
	
	def __init__(self, filename):
		'''
		@type	filename: string
		@param	filename: Filename to write to should have a %d in it
		someplace :)
		'''
		self._origFilename = filename
		self.setFilename(filename % self._roundCount)
	
	def send(self, data):
		File.send(self, data)
		self.next()
	
	def next(self):
		self._roundCount += 1
		if self._state == 1:
			self.stop()
			self.setFilename(self._origFilename % self._roundCount)
			self.start()
		else:
			self.setFilename(self._origFilename % self._roundCount)
	
	def reset(self):
		self._roundCount = 0
		if self._state == 1:
			self.stop()
			self.setFilename(self._origFilename % self._roundCount)
			self.start()
		else:
			self.setFilename(self._origFilename % self._roundCount)
		

		
def unittest():
	pass

# end

