#!/usr/bin/env python
'''
Core modules shared by frontend applications
'''

import os
import shutil

def check_default_config (config_file, config_default):
    """Look for the config file and create from default if it isn't there"""
    if os.path.exists(config_file):
        pass
    else:
        print config_file + " not found; creating a new one from default"
        shutil.copyfile(config_default, config_file)



def check_config(param, configfile):
    """Check the config file and return the requested parameter"""
        
    fileopen = file(configfile, "r")
    for line in fileopen:
        # if the line starts with the param we want then we are set, otherwise if it starts with a # then ignore
        if line.startswith(param) != "#":
            if line.startswith(param):
                line = line.rstrip()
                # remove any quotes or single quotes
                line = line.replace('"', "")
                line = line.replace("'", "")
                line = line.split("=")
                return line[1]
    #return null if no matching line found
    print "Parameter " + param + " not found in config file " + configfile
    print "Check your config file against the default...Using a default value..."
    return ""
