#!/usr/bin/env python
'''
core modules for the crackmanager.py applications
'''

import subprocess
import shlex
import threading
import os
import time
import re
import traceback
import copy

#-----------------------------------------------------------------------------
# CrackThread Class
#-----------------------------------------------------------------------------
class CrackThread(threading.Thread):
    """Takes an id, hash type, a hash list, and a list of commands. The hash
    list should be in username:hash format except for special cases. 
    The hash list is processed to extract usernames. After each command is run
    the results are processed, added to the results array, and the cracked hashes 
    are removed from the hash file."""
    
    def __init__(self, id, hash_type, hash_list, commands):
        threading.Thread.__init__(self)
        self.id = id
        self.hash_type = hash_type
        self.hash_list = hash_list
        self.commands = commands
        self.hash_file = id + '.hash'
        self.results = []
        self.hashes = {}
        self.complete = False
        
        print "Processing request " + id + " with hash type " + hash_type

        
    def __del__(self):
        """Remove the temporary hash file"""
        print "Cracking session " + self.id + " is complete..."
        for r in self.results:
            print r.rstrip('\r\n')
        os.remove(self.hash_file)
        
    def process_hash_list(self):
        """Process the file passed to us and extract the usernames, then write
        the file to disk for processing by the commands. Pwdump files and DCC
        files are special cases. The typical input should be username:hash"""
        
        #clear binary file data out of hash list and re-import the info
        #from the temporary hash file if the file is a text file
        self.hash_list = []
        if self.hash_type[:4] == 'wifi' or self.hash_type[:3] == 'ike':
            pass
        else:
            file = open(self.hash_file, 'rb')
            for line in file:
                self.hash_list.append(line.rstrip('\r\n'))
            file.close()
        
        #pwdump config file entries must start with "pwdump"
        if self.hash_type[:6] == 'pwdump':
            for line in self.hash_list:
                user, id, lm, ntlm, a, b, c = line.split(':')
                key = user
                if self.hash_type.lower()[-2:] == 'nt':
                    self.hashes[key] = ntlm.lower()
                else:
                    self.hashes[key] = lm.lower()
                    
        #john config file entries must start with "john"    
        elif self.hash_type[:4] == 'john':
            if self.hash_type.lower()[-2:] == 'v2':
                for line in self.hash_list:
                    user, id, domain, challenge, hash, salt = line.split(':')
                    key = user
                    self.hashes[key] = hash.lower()
            else:
                for line in self.hash_list:
                    user, id, domain, lm, ntlm, challenge = line.split(':')
                    key = user
                    if self.hash_type.lower()[-4:] == 'ntlm':
                        self.hashes[key] = ntlm.lower()
                    else:
                        self.hashes[key] = lm.lower()
            

        #wifi config file entries must start with "wifi"
        elif self.hash_type[:4] == 'wifi':
            #just bring in pcap file
            pass
            
        #ike-scan psk-crack config file entries must start with "ike"
        elif self.hash_type[:3] == 'ike':
            #just bring in scan file
            pass
        
        #domain cached cred config file entries must start with "dcc"
        elif self.hash_type == 'dcc':
            for line in self.hash_list:
                dcc, user = line.split(':')
                key = user
                self.hashes[key] = dcc.lower()
            
        else:
            hashes = []
            for line in self.hash_list:
                user, hash = line.split(':')
                hashes.append(hash.lower())                
                key = user
                self.hashes[key] = hash.lower()
                
            self.write_file(hashes)

    def process_hash(self, hash, user, password, plaintext=""):
        '''plaintext variable is for simple output where hashes are not stored in
        the hash table (such as wifi hashes); if this is the case, just append
        output in plaintext field to the results table and spit it out!
        '''
        
        if plaintext:
            self.results.append(plaintext)
        
        key = user
        for k,v in self.hashes.items():
            if v == hash or k == key:
                hash = v
                self.results.append(k+":"+password)
                del self.hashes[k]

            #remove found hashes from hash list
            for line in self.hash_list:
                if re.search(hash, line):
                    self.hash_list.remove(line)
                    
    
    def process_output(self, output):
        """Uses regular expressions to find hashes and passwords in results and
        passes them to either the process_hash or process_user function. 
        
        Results may return the following:
            -hash & password combination
            -user & password combination
            -single line result (designated plaintext)
        These are positional variables in the process_hash function.        
        """
        
        #replaced this regex because it was not correctly parsing passwords
        #Regex_rcracki_mt = "([A-Za-z0-9.]+)\s+(.?)\s+hex:.*"
        Regex_rcracki_mt = "([A-Za-z0-9.]+)\s+(.*?)\s+hex:.*"
        #replaced this regex because was excluding user names with non-alphanumeric chars
        #Regex_john = "([A-Za-z0-9.]*):(.*?):(.*?):.*?"
        Regex_john = "(.*?):(.*?):(.*?):.*?"
        Regex_hashcat_dcc = "([0-9a-f]{16,}):.*:(.*)"
        Regex_hashcat_standard = "([0-9a-f]{16,}):(.*)"
        Regex_pyrit = "(The password is .*)"
        Regex_pskcrack = "(.*matches.*)"

        if self.hash_type[:6] == 'pwdump':
            # All REs here should be for proccessing results of pwdump commands
            # RE for output from rcracki_mt
            for r in re.finditer(Regex_rcracki_mt, output):
                self.process_hash("",r.group(1), r.group(2))
            # RE for output from john the ripper
            for r in re.finditer(Regex_john, output):
                self.process_hash("",r.group(1), r.group(2))
        
        elif self.hash_type[:4] == 'john':
            # RE for output from john the ripper
            for r in re.finditer(Regex_john, output):
                self.process_hash("",r.group(1), r.group(2))
                
        elif self.hash_type[:4] == 'wifi':
            # RE for output from pyrit; pass output to plaintext variable instead of into hash table
            for r in re.finditer(Regex_pyrit, output):
                self.process_hash("","", "", r.group(1))
                
        elif self.hash_type[:3] == 'ike':
            # RE for output from ike-scan psk-crack; pass output to plaintext variable instead of into hash table
            for r in re.finditer(Regex_pskcrack, output):
                self.process_hash("","", "", r.group(1))
                    
        elif self.hash_type == 'dcc':
            #All REs here should be for processing results of dcc commands
            # RE for DCC output for hashcat family
            for r in re.finditer(Regex_hashcat_dcc, output):
                self.process_hash(r.group(1), "", r.group(2))
        else:
            # RE for standard output for hashcat family
            for r in re.finditer(Regex_hashcat_standard, output):
                self.process_hash(r.group(1), "", r.group(2))
        print output

    def write_file(self, hashes=None):
        
        if hashes == None:
            hashes = self.hash_list
            
        f = open(self.hash_file, 'w')
        for line in hashes:
            f.write(line + '\n')
        f.close()

    def fix_cmd(self, cmd):
        for c in xrange(len(cmd)):
            if cmd[c] == '{file}': cmd[c] = os.path.join(os.getcwd(),self.hash_file)
        return cmd 

    def run(self):
        """For each command, process the hash_list, modify the command to
        include the correct file name on disk, and run the command. Once the
        command is run, we process the output, which include updating the hash
        list to remove found hashes."""
                
        for cmd in self.commands:
            self.process_hash_list()
            cmd = self.fix_cmd(cmd)
            self.process_output(subprocess.check_output(cmd))
        self.complete = True


#------------------------------------------------------------------------------
# CrackManager Class
#------------------------------------------------------------------------------
class CrackManager():

    def __init__(self, config):
        self.config = {}
        self.load_cfg(config)
        self.processes = {}

    def load_cfg(self, config):
        """Load configuration file. Blank lines and comments are skipped.
        Confirms each command exists but does not confirm the arguments to the
        command."""
        try:
            cfgfile = open(config, 'r')
            for line in cfgfile:
                if re.match('^$', line): continue
                if re.match('^#.*$', line): continue
                h, c = line.split('|')
                cmd = shlex.split(c.rstrip('\r\n'))
                
                # Split off the command so we can verify it exists.
                if os.path.exists(cmd[0]):
                    if h in self.config.keys():
                        self.config[h].append(cmd)
                    else:
                        self.config[h] = []
                        self.config[h].append(cmd)
                else:
                    raise Exception("Command {0} does not exist.".format(cmd[0]))
    
        except Exception, err:
            raise Exception("Error loading configuration file: \n{0}\n{1}\n".format(str(err), traceback.print_exc()))
            
    def crack_passwords(self, hlist, htype):
        """Accepts a hash file and hash type from the xmlrpc client. Creates an id
        and a CrackThread object and passes the id, file, and hash type to it.
        Returns the id so that results can be obtained later.
        
        If a hash type is not supported by the server then it returns id 0."""

        id = 0
        message = ''
        if htype in self.config.iterkeys():
            id = str(int(time.time()))
            message = "Request accepted by server."
            
            #write hash data passed to file
            with open(id + '.hash', "wb") as handle:
                handle.write(hlist.data)
            
            #copy config commands into new list so we don't overwrite the existing config
            #when replacing the {file} variable
            commands = copy.deepcopy(self.config[htype])
            
            self.processes[id] = CrackThread(id, htype, hlist, commands)
            self.processes[id].start()
        else:
            message = "Server does not support the hash type requested. Acceptable hash types are: " + str(sorted(self.config))

        return id, message

    def get_progress(self, id):
        """Accepts an id and provides the results for the CrackThread with that
        id. Gets a copy of the results and clears them to prevent duplicates.
        If the process is complete, it is removed from the process dictionary.
        Returns completion status and current results."""

        r = self.processes[id].results
        self.processes[id].results = []
        
        c = self.processes[id].complete
        if c:
            #If the thread is complete remove CrackThread from processes dict
            del(self.processes[id])
        
        return c, r
    
