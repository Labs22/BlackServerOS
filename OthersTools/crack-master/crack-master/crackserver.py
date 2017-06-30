#!/usr/bin/env python
'''
Launches an XMLRPC listener for password cracking requests from crackclient.py
'''

import SimpleXMLRPCServer as sxml
import argparse
from modules.core import *
from modules.core_crackserver import *

#------------------------------------------------------------------------------
# Get values from config file
#------------------------------------------------------------------------------

#check to see if specified config file exists; if not copy default
config_file = "config/crackserver.cfg"
config_default = "config/crackserver.default"
check_default_config(config_file, config_default)

server_ip = check_config("SERVER_IP", config_file)
if server_ip == "": server_ip = "127.0.0.1"

server_port = check_config("SERVER_PORT", config_file)
if server_port == "": server_port = "8000"

crack_config = check_config("CRACK_CONFIG", config_file)
if crack_config == "": crack_config = "config/crack.cfg"

config_file = "config/crack.cfg"
config_default = "config/crack.default"
check_default_config(config_file, config_default)

#------------------------------------------------------------------------------
# Configure Argparse to handle command line arguments
#------------------------------------------------------------------------------
desc = """ Crackserver launches a XMLRPC server to handle password cracking 
requests."""

parser = argparse.ArgumentParser(description=desc)
parser.add_argument('-l', action='store', default=server_ip,
                    help='IP address to listen on. (default: ' + server_ip + ')')
parser.add_argument('-p', action='store', default=server_port,
                    help='Port to listen on. (default: ' + server_port +')')
parser.add_argument('-c', action='store', default=crack_config,
                    help='Configuration file. (default: ' + crack_config + ')')

args = parser.parse_args()

#------------------------------------------------------------------------------
# Main Program
#------------------------------------------------------------------------------

# Create new CrackManager object to handle cracking process.
try:
    c = CrackManager(args.c)
    print "CrackManager configured successfully using config file " + args.c
except Exception, err:
    print "CrackManager configuration unsuccessful:\n"
    print str(err)
    exit()
    
try:
    server = sxml.SimpleXMLRPCServer((args.l, int(args.p)),
        requestHandler=sxml.SimpleXMLRPCRequestHandler)
    print "XMLRPC server configuration successful; listening on " + args.l+":"+args.p
except Exception, err:
    print "XMLRPC server configuration unsuccessful:\n"
    print str(err)
    exit()

# Register CrackManager functions to be used with by XMLRPC client.
server.register_introspection_functions()
server.register_function(c.crack_passwords, 'crack')
server.register_function(c.get_progress, 'results')
server.serve_forever()
