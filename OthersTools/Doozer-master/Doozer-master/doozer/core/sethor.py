""" doozer settings file; all constants should be set here.
"""

# general doozer settings
VERSION = "2.0"
DOOZER_IP = "127.0.0.1"
DOOZER_PORT = 8000
# this is the directory we monitor for hash files
MONITOR_DIR = None 
# location of doozer.py
DOOZER = None 
# this is where sessions are archived; currently we dont automate this
SESSION_ARCHIVE = None 

# cracking settings for hashcat/ophcrack
HASHCAT_BINARY = None
HASHCAT_DIR = None 
# location of crack wordlists
WORDLIST_DIR = None 
# where sessions are moved to
WORKING_DIR = None 

# monitor.py settings
MONITOR_CHECK = 10

OPHCRACK_TABLES = None 
