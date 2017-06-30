# Project LAMMA
#
#                         __    _____ _____ _____ _____
#                        |  |  |  _  |     |     |  _  |
#                        |  |__|     | | | | | | |     |
#                        |_____|__|__|_|_|_|_|_|_|__|__|
#
#                                   (beta)
#
#
#    Plugin Name    :   rscan.py
#    Plugin ID      :   rm-10
#    Plugin Purpose :   Invokes scanning of a given host with specialized plugins.
#
#
#    Plugin Author  :   @ajithatti
#
#    Plugin Version :   0.0.1
#    Plugin Status  :   beta
#
################################################################################

# Plugin Informaiton

# Env Variable

import os
import os.path
import subprocess
from sys import stdout

# Plugin Mode

# Settings

# Report Findings



# --------------------------------------------------------------------------------
# Function      : html_header()
#
# Purpose       : adds html header to a given file
#
# Parameters    : rep   - file
#
#
# Returns       : None




def html_header(rep, title):

    if(rep != stdout):
        rep_f = open(rep, "a+")
    else:
        rep_f = rep
    pass

    header = "<html> <title>" + title + " </title> <body><PRE>"
    rep_f.write(header)

    if(rep!= stdout):
        rep_f.close()
    return 0


pass


# --------------------------------------------------------------------------------
# Function      : html_footer()
#
# Purpose       : adds html footer to a given file
#
# Parameters    : rep   - file
#
# Returns       : None





def html_footer(rep):

    if(rep != stdout) :
        rep_f = open(rep, "a+")
    else :
        rep_f = rep
    pass

    footer = "</PRE></body> </html>"
    rep_f.write(footer)

    if(rep != stdout):
        rep_f.close()
    return 0


pass


# --------------------------------------------------------------------------------
# Function      : kick_dir()
#
# Purpose       : Iterates through directory and kics's the python scripts
#
# Parameters    :   indir   :   current directory location
#                    param   :   script parameter
#
# Returns       : None




def kick_dir(indir, param):
    prog = "python "

    print "[+] Kick Starting...", indir
    for root, dirs, filenames in sorted(os.walk(indir)):
        for f in filenames:
            if f.endswith("__init__.py"):
                continue
            if f.endswith(".py"):
                cmd = (prog + root + "/" + f + " " + param)
                print "\n\n\t Now Executing:  ", (cmd), "\n"
                subprocess.call(cmd, shell="false")
                # subprocess.Popen(cmd, stdout=f, stderr=e, stdin=subprocess.PIPE) #, stdout=f )
    pass


pass


# --------------------------------------------------------------------------------
# Function      : script_invoke()
#
# Purpose       : Invokes other scripts
#
# Parameters    :   script_dir  :   path to script or directory to be invoked
#                   param       :   Parameters to the script
# Returns       : None




# --- Check if the script path points to a single script


def script_invoke(script_dir, param):
    prog = "python "

    indir = os.getcwd()
    indir = indir + "/modules/remote-module/"

    if (script_dir.endswith(".py")):

        cmd = (prog + indir + script_dir + " " + param)
        print "Executing : ", cmd
        subprocess.call(cmd, shell="false")
        # subprocess.Popen(cmd, stdout=f, stderr=e, stdin=subprocess.PIPE) #, stdout=f )

    elif (script_dir.endswith("all")):  # if all are to be executed
        print "\n\n Kick %s \n\n" % script_dir
        kick_dir(indir + "gen", param)
        kick_dir(indir + "reg", param)
        kick_dir(indir + "others", param)

    else:
        print "\n\n Kick %s \n\n" % script_dir
        kick_dir(indir + script_dir, param)


pass


# --------------------------------------------------------------------------------
# Function      : multi_host_scan()
#
# Purpose       : Invokes scanning scripts for multiple IP, hosts one after the other
#
# Parameters    :   in_file     :   file containing multi host information
#                   script_set  :   scripts to be invoked
#                    cmd_string  :   command paramerters to be passed
# Returns       : None


def multi_host_scan(in_file, script_set, cmd_string):
    pass


pass


