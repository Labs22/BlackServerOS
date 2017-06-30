#!/usr/bin/env python
'''
Crackclient makes xmlrpc calls to a crack server to automate password
cracking. Crackclient takes a file, a server:port combination, and a hash
type. The file and hash type are passed to the server and the server returns
an id. The id can be polled to get the results and to see if all of the
cracking processes are finished.

Acceptable hash types are defined in config file associated with
listening crackserver instance.

While crackserver.py is dependent upon core modules contained in other files,
crackclient.py is dependent only upon standard python modules

'''

import xmlrpclib
import argparse
import time
from modules.core import *

#------------------------------------------------------------------------------
# Get values from config file
#------------------------------------------------------------------------------
config_file = "config/crackclient.cfg"
config_default = "config/crackclient.default"
check_default_config(config_file, config_default)

server_ip = check_config("SERVER_IP", config_file)
if server_ip == "": server_ip = "127.0.0.1"

server_port = check_config("SERVER_PORT", config_file)
if server_port == "": server_port = "8000"

#------------------------------------------------------------------------------
# Configure Argparse to handle command line arguments
#------------------------------------------------------------------------------
desc = """Crackclient makes XMLRPC calls to a crack server to automate password
cracking. Crackclient takes a file, a server:port combination, and a hash type.
The file and hash type are passed to the server and the server returns an id.
The id can be polled to get the results and to see if all of the cracking
processes are finished."""

parser = argparse.ArgumentParser(description=desc)
parser.add_argument('file', action='store', default='hashes.txt',
                    help='Specify a hash file (default: hashes.txt)')
parser.add_argument('type', action='store', default='md5',
                    help='Specify the hash type (default: md5)')
parser.add_argument('-s', action='store', default=server_ip,
                    help='IP address that the remote server is listening on. (default: ' + server_ip + ')')
parser.add_argument('-p', action='store', default=server_port,
                    help='Port that the remote server is listening on. (default: ' + server_port +')')


#------------------------------------------------------------------------------
# Main Program
#------------------------------------------------------------------------------

args = parser.parse_args()

# Open connection to xmlrpc server
connect_addr = 'http://' + args.s+ ":" + args.p
try:
    s = xmlrpclib.ServerProxy(connect_addr)
except Exception, err:
    print "Error opening connection to server " + connect_addr + " - " + str(err)

#Upload hash file to server, send crack request to server and receive ID
with open(args.file, 'rb') as handle:
    binary_data = xmlrpclib.Binary(handle.read())
id, msg = s.crack(binary_data, args.type)

if id == 0:
    print msg
else:
    # Poll server for completion status and results using ID.
    complete = False
    wait = 10
    while True:
        time.sleep(wait)
        complete, results = s.results(id)
        if results != []:
            for r in results:
                print r.rstrip('\r\n')
        if complete: break    
