#!/usr/bin/python -B
# -*- coding: utf-8 -*-
#
#  camscan.py
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following disclaimer
#    in the documentation and/or other materials provided with the
#    distribution.
#  * Neither the name of the project nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

__version__ = '0.1'

SSH_DEFAULT_PORT = 22
DEFAULT_SLEEP_TIME = 0.5
CISCO_NEW_LINE = '\r\n'
CISCO_MORE_LINE = ' --More--'
REPORT_BANNER = "{0:<17}  {1:<15}  {2:<15}  {3:<4}"

import argparse
import getpass
import logging
from time import sleep
from binascii import unhexlify
import sys
import os

sshlib = None
try:
	import ssh
	sshlib = ssh
except ImportError:
	pass

try:
	import paramiko
	sshlib = paramiko
except ImportError:
	pass

if sshlib == None:
	print 'Must have python-ssh or python-paramiko installed.'
	sys.exit(1)

def macMatch(control, test, wildcards = True):
	control = control.lower().split(':')
	test = test.lower().split(':')
	if len(control) != 6 or len(test) != 6:
		raise Exception('malformed MAC')
	for i in xrange(0, 6):
		if wildcards and control[i] == '*':
			continue
		if control[i] != test[i]:
			return False
	return True

class CiscoSwitch:
	def __init__(self, host, username = None, password = None):
		if not username:
			username = raw_input('Username: ')
		if not password:
			password = getpass.getpass('Password: ')

		self.host = host
		self.port = SSH_DEFAULT_PORT
		self.username = username
		self.password = password

		self.ssh = None
		self.logger = logging.getLogger('switch')
		self.connect()

	def close(self):
		self.ssh.close()

	def connect(self):
		self.ssh = sshlib.SSHClient()
		self.ssh.set_missing_host_key_policy(sshlib.AutoAddPolicy())
		self.ssh.connect(self.host, port = self.port, username = self.username, password = self.password)
		self.logger.info('successfully connected to: ' + self.username + '@' + self.host + ':' + str(self.port))
		self.shell = self.ssh.invoke_shell()

		sleep(DEFAULT_SLEEP_TIME)
		buff = ''
		while self.shell.recv_ready():	# flush the buffer in case there is a banner
		    buff += self.shell.recv(512)

		if self.privileges_sync() != 15:
			if not self.privileges_elevate():
				self.logger.critical('could not obtain the necessary privileges')
				raise Exception('could not obtain the necessary privileges')

	def privileges_elevate(self):
		self.logger.info('elevating privileges')

		self.shell.send('enable\n') # don't change this to CRLF
		sleep(DEFAULT_SLEEP_TIME)
		buff = ''
		while self.shell.recv_ready():
			buff += self.shell.recv(512)

		output = filter(lambda x: x, buff.strip().split(CISCO_NEW_LINE))
		if not output:
			raise Exception('failed to obtain privilege level 15')
		if not output[-1].lower().startswith('password:'):
			raise Exception('failed to obtain privilege level 15')

		self.shell.send(self.password + '\n')
		sleep(DEFAULT_SLEEP_TIME)
		buff = ''
		while self.shell.recv_ready():
			buff += self.shell.recv(512)
		output = filter(lambda x: x, buff.strip().split(CISCO_NEW_LINE))

		retries = 3
		while 'access denied' in output[0].lower() and retries:
			self.logger.warning('enable password rejected')
			self.shell.send('enable\n') # don't change this to CRLF
			sleep(DEFAULT_SLEEP_TIME)
			buff = ''
			while self.shell.recv_ready():
				buff += self.shell.recv(512)

			output = filter(lambda x: x, buff.strip().split(CISCO_NEW_LINE))
			if not output:
				raise Exception('failed to obtain privilege level 15')
			if not output[-1].lower().startswith('password:'):
				raise Exception('failed to obtain privilege level 15')

			password = getpass.getpass('Enable Password For ' + self.username + '@' + self.host + ': ')
			self.shell.send(password + '\n')
			sleep(DEFAULT_SLEEP_TIME)
			buff = ''
			while self.shell.recv_ready():
				buff += self.shell.recv(512)
			output = filter(lambda x: x, buff.strip().split(CISCO_NEW_LINE))
			retries -= 1
		if self.privileges_sync() == 15:
			self.logger.info('successfully elevated to privilege level 15')
			return True
		return False

	def privileges_sync(self):
		cmd = self.execute_command('show privilege')
		if not cmd:
			raise Exception('failed to identify privilege level')
		if cmd[0].lower()[:27] != 'current privilege level is ':
			raise Exception('failed to identify privilege level')
		self.privilege_level = int(cmd[0].split()[-1])
		return self.privilege_level

	def execute_command(self, command):
		self.logger.info('running command: ' + str(command))

		self.shell.send(command + '\n')
		sleep(DEFAULT_SLEEP_TIME)
		buff = ''
		while self.shell.recv_ready():
			buff += self.shell.recv(512)
		data = filter(lambda x: x, buff.strip().split(CISCO_NEW_LINE))[1:]

		while data[-1].startswith(CISCO_MORE_LINE):
			del data[-1]
			self.shell.send(' \n')
			sleep(DEFAULT_SLEEP_TIME)
			buff = ''
			while self.shell.recv_ready():
				buff += self.shell.recv(512)
			while buff[0] == '\x08' or buff[0] == ' ':
				buff = buff[1:]
			data.extend(filter(lambda x: x, buff.strip().split(CISCO_NEW_LINE))[1:])
		return data

	def get_cam_table(self):
		cam_table = []

		raw_data = self.execute_command('show mac address-table')

		raw_data = raw_data[4:]
		prompt = raw_data.pop()
		while raw_data[-1] == prompt:
			raw_data.pop()
		raw_data.pop()

		if not raw_data:
			return None
		for line in raw_data:
			line = line.split()
			if len(line) != 4:
				self.logger.error('invalid line in CAM table found')
				return
			entry = {}
			entry['vlan'] = line[0]
			entry['mac'] = ':'.join([x.encode('hex') for x in unhexlify(line[1].replace('.', ''))])
			entry['type'] = line[2]
			entry['ports'] = line[3]
			if entry['ports'] != 'CPU':
				cam_table.append(entry)
		return cam_table

def main():
	parser = argparse.ArgumentParser(description = 'Cisco Switch MAC Anomaly Detector', conflict_handler = 'resolve')
	parser.add_argument('-v', '--version', action = 'version', version = parser.prog + ' Version: ' + __version__)
	parser.add_argument('-L', '--log', dest = 'loglvl', action = 'store', choices = ['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'], default = 'ERROR', help = 'set the logging level')
	parser.add_argument('-u', '--user', dest = 'ssh_user', action = 'store', required = False, help = 'default user to authenticate as')
	parser.add_argument('-p', '--pass', dest = 'ssh_pass', action = 'store', required = False, default = None, help = 'default password to authenticate with')
	parser.add_argument('-h', '--hosts', dest = 'host_file', action = 'store', required = True, type = argparse.FileType('r'), help = 'list of switches to scan')
	parser.add_argument('-m', '--macs', dest = 'mac_file', action = 'store', required = True, type = argparse.FileType('r'), help = 'list of MAC addresses to ignore')
	parser.add_argument('-r', '--report', dest = 'report_file', action = 'store', required = False, type = argparse.FileType('w'), default = 'report.csv', help = 'report CSV file')

	try:
		arguments = parser.parse_args()
	except IOError as error:
		if hasattr(error, 'filename'):
			print error.strerror + ': ' + error.filename
		else:
			print error.strerror
		return 1

	logging.basicConfig(level = getattr(logging, arguments.loglvl), format = "%(levelname)-8s %(message)s")
	logger = logging.getLogger('main')

	try:
		ssh_user = arguments.ssh_user
		if not ssh_user:
			ssh_user = raw_input('Switch Username: ')
		ssh_pass = arguments.ssh_pass
		if not ssh_pass:
			ssh_pass = getpass.getpass('Switch Password: ')
	except KeyboardInterrupt:
		return 1

	anomalies = 0
	switch_ip = arguments.host_file.readline().strip()
	while switch_ip:
		logger.info('now processing switch: ' + switch_ip)
		try:
			switch = CiscoSwitch(switch_ip, username = ssh_user, password = ssh_pass)
		except Exception as error:
			logger.error('received an error while connecting to: ' + switch_ip)
			switch_ip = arguments.host_file.readline().strip()
			continue
		cam_table = switch.get_cam_table()
		logger.debug('retreived ' + str(len(cam_table)) + ' entries from the CAM table')
		white_mac = arguments.mac_file.readline().strip()
		while white_mac:
			for entry in cam_table:
				if macMatch(white_mac, entry['mac']):
					continue
				if anomalies == 0:
					print REPORT_BANNER.format('MAC Address', 'Ports', 'Switch IP', 'VLAN')
					print REPORT_BANNER.format('-----------', '-----', '---------', '----')
				print REPORT_BANNER.format(entry['mac'], entry['ports'], switch_ip, entry['vlan'])
				arguments.report_file.write(",".join([entry['mac'], entry['ports'], switch_ip, entry['vlan']]) + os.linesep)
				anomalies += 1
			white_mac = arguments.mac_file.readline().strip()
		arguments.mac_file.seek(os.SEEK_SET, 0)
		arguments.report_file.flush()
		switch_ip = arguments.host_file.readline().strip()
		switch.close()
	print '\nIdentified ' + str(anomalies) + ' network anomalies.'
	return 0

if __name__ == '__main__':
	sys.exit(main())
