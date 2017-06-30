#!/usr/bin/env python
'''
Module for performing a standalone cracking operation rather than using the
client server capabilities of crackserver.py and crackclient.py. Uses the same
core modules as the crackserver applications for performing cracking operation.

Takes a file, a server:port combination, and a hash type. 

Acceptable hash types are defined in config file associated with
listening crackserver instance.

While crackserver.py is dependent upon core modules contained in other files,
crackclient.pl is dependent only upon standard python modules

'''

import argparse
from modules.core_crackserver import *
from modules.core import *

class hashlist:
    '''
    TODO: Find a more elegant way of doing this!!!
    
    Hashes that are passed to the cracking mechanism via RPC are passed using 
    xmlrpclib binary class, and are stored in a 'data' attribute. This dummy
    class is to allow us a local class in which to store the hashes in 
    a similarly named attribute so we can re-use the functionality
    
    Need to re-write the core module
    
    '''
    def __init__(self, data):
        self.data = data


#check to see if specified config file exists; if not copy default
config_file = "config/crack.cfg"
config_default = "config/crack.default"
check_default_config(config_file, config_default)


#------------------------------------------------------------------------------
# Configure Argparse to handle command line arguments
#------------------------------------------------------------------------------
desc = """Crack takes a file and a hash type."""

parser = argparse.ArgumentParser(description=desc)
parser.add_argument('file', action='store', default='hashes.txt',
                    help='Specify a hash file (default: hashes.txt)')
parser.add_argument('type', action='store', default='md5',
                    help='Specify the hash type (default: md5)')
parser.add_argument('-c', action='store', default='config/crack.cfg',
                    help='Configuration file. (default: config/crack.cfg)')


#------------------------------------------------------------------------------
# Main Program
#------------------------------------------------------------------------------

args = parser.parse_args()
htype = args.type
hashfile = open(args.file, "r")

hlist = hashlist(hashfile.read())

# Create new CrackManager object to handle cracking process.
try:
    c = CrackManager(args.c)
    print "CrackManager configured successfully using config file " + args.c
except Exception, err:
    print "CrackManager configuration unsuccessful:\n"
    print str(err)
    exit()
    
c.crack_passwords(hlist, htype)
