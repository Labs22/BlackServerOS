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
#    Plugin Name    :   remote.py
#    Plugin ID      :   r-10
#    Plugin Purpose :   Interface for remote module
#
#
#    Plugin Author  :   @ajithatti
#
#    Plugin Version :    0.0.1
#    Plugin Status  :   beta
#
################################################################################

# Include Section
import subprocess
import os
import os.path
import sys
import getopt
import rscan
from sys import argv, stdout

# Plugin Informaiton

# Env Variable


# Plugin Mode

# Settings

# Report Findings

#------------------------------------------------------------------------------
#Function      : main
#
# Purpose       : The main funciton of the script. Reads the configuration file for
#
# Parameters    : 1. argv     (list)    - list of command line arguments
#
# Returns       : None


def log(message, sink):
    sink.write(message)
pass



# --------------------------------------------------------------------------------
# Function      : usage
#
# Purpose       : Proivde a mini help on usage of this program
#
# Parameters    : None
#
# Returns       : None

def usage(script):
    print "\n\n    %s [-H] [-s] [-l] [-o] [-i] [-p] [-h] " %script
    print "\n    Purpose  : Scan a remote host with given plugin over SSL/TLS connection"
    print "\n"
    print "        -H [--help]      : prints this usage help"
    print "        -s [--script]    : scan the target with given script id or 'all','gen',  or 'reg'"
    print "        -l [--list]      : list all the plugins on a specified path"
    print "        -o [--out]       : reports are stored in this file else default file"
    print "        -i [--in]        : input file name with multiple IP:Port specified in each line"
    print "        -p [--port]      : port on which SSL or TLS connection is to be made"
    print "        -h [--host]      : IP or Domain name of the remote host to be connected"
    print " \n\n\n"

    sys.exit(2)


pass  # USAGE BLOCK


# --------------------------------------------------------------------------------
# Function      : list_all_trust_scripts()
#
# Purpose       : list all the scripts in the trust module
#
# Parameters    : path to be searched
#
# Returns       : None


def list_all_remote_scripts(path):
    indir = os.getcwd()

    prefix = ""

    if(indir.endswith("LAMMA")):
        prefix = "modules/remote-module/"

    if(indir.endswith("modules")):
        prefix = "remote-module/"

    indir = indir + "/" + prefix + path

    for root, dirs, filenames in sorted(os.walk(indir)):
        pos = indir.__len__()
        check_dir = root[pos:]
        if ( check_dir.find(".git") != -1):
            continue

        print "./" + root[pos:] + "/"

        for f in filenames:
            if f.endswith(".py"):
                if (f == "__init__.py"):
                    continue
                print "\t  ", f

    exit(0)


pass


# -------------------------------------------------------------------------------
# Function  : main
#
# Purpose   : The main funciton of the script. Reads the configuration file for
#
# Params    : 1. argv     (list)    - list of command line arguments
#
# Returns   : None

def main(argv):
    # **** Lets process the arguments *********

    # Initialize the variables

    this_script = argv[0]
    script_set = ""
    #report_path = "/home/evader/Desktop/PROJECTS/LAMMA/BETA/reports/"
    out_file = ""
    in_file = ""
    port = 0
    host = ""
    skip_scan = False
    multi_domain = False
    cmd_string = ""
    sink = stdout

    # ---- Show the usage for too few arguments

    if len(argv) < 3:
        usage(this_script)
        sys.exit(2)
    pass  # if block

    # --- try block

    try:
        opts, args = getopt.getopt(argv[1:], "H:s:l:o:i:p:h:",
                                   ["help=", "script=", "list", "out=", "in=", "port=", "host="])


    except getopt.GetoptError:
        usage(this_script)
        sys.exit(2)
    pass  # TRY BLOCK

    # ---- Process the arguments passed to the script

    for opt, arg in opts:
        if opt in ("-H", "--help"):
            usage(this_script)

        elif opt in ("-s", "--script"):
            script_set = arg

        elif opt in ("-l", "--list"):
            list_scripts = True
            list_all_remote_scripts(arg)

        elif opt in ("-o", "--out"):
            out_file = arg

        elif opt in ("-i", "--in"):
            in_file = arg

        elif opt in ("-d", "--debug"):
            debug = True
            debug_file = arg

        elif opt in ("-p", "--port"):
            port = arg

        elif opt in ("-h", "--host"):
            host = arg

        pass  # IF BLOCK
    pass  # FOR BLOCK

    # ---- collect the values and check


    # --- Check the scripts to be executed : flag [-s]

    if (script_set == ""):
        print ("\n\n>> No script[s] specified")
        print ("\n\t pls specify choice of your scripts to be executed")
        print ("\n\t\t -s [gen] [all] [reg] [script_name]")
        print ("\t\t\t gen - runs basic checks on SSL TLS connections")
        print ("\t\t\t all - runs all available scripts ")
        print ("\t\t\t reg - runs 'gen' + a group of frequently used scripts put in 'regular' directory")
        print ("\t\t\t script_name - runs only this specific script")

        skip_scan = True
    pass  # if Block


    # -- Check for the Host+Port or Infile

    if (in_file == ""):
        if (host == ""):
            print "\n\n>> Target host not specified"
            print ("\n\t pls specify target host name or IPv4")
            print ("\n\t\t -H  host-name or IPv4")
            skip_scan = True
        if (port == 0):
            print "\n\n>> Target port not specified"
            print ("\n\t pls specify target port")
            print ("\n\t\t -p   TCP/IP port number")
            skip_scan = True
        pass
    else:
        multi_domain = True
    pass

    if (skip_scan):
        print "\n\n>>   [Incomplete Configuration]     "
        print "\n\n[*]  Lamma Scanning Service [Aborted] .... \n\n\n"
        sys.exit(2)

    pass  # -- If block

    # -- We have enouh configuration parameters to continue

    # -- Show the configuration before starting the scan ----

    log("\n\n[*]  Lamma Scanning Service [Started] ... \n\n", sink)
    log ( "\n[+]  Parametrs set for this Scan : \n\n", sink)

    log ( "     Tests to Run => %s" %script_set, sink)

    if (multi_domain == False):

        print "     Target Host => ", host
        print "     Target port => ", port
        target = host
        cmd_string = cmd_string + " -h " + host
        cmd_string = cmd_string + " -p " + port

    else:
        log( "     Targets to be scanned are mentioned in file => %s" %in_file, sink)
        target = in_file
        # cmd_string = cmd_string + " -i "+ in_file

    if (out_file != ""):
        sink = open(out_file, "a+")
        print "     Reports will be stored in file => %s" %out_file
        cmd_string = cmd_string + " -o " + out_file
    pass

    # -- Configuration displayed --

    # -- Set the out file --

    if(out_file != ""):
        rscan.html_header(out_file, host)

    # **** Lets proceed with the scanning *********

    # Prepeare the command line arguments for the scripts to be invoked
    # cmd_args = (+++)

    # invoKe script

    if (multi_domain == True):
        print ("\n\n[+] Starting Multihost scnning\n\n ")
        rscan.multi_host(in_file, script_set, cmd_string)
    else:
        print ("\n\n[+] Starting the scan\n\n ")
        rscan.script_invoke(script_set, cmd_string)
    pass

    if (out_file != ""):
        rscan.html_footer(out_file)


    print "[+] Scaning complete...\n\n"


pass  # main

# -------------------------------------------------------------------------------
#
# Kick off the script

if __name__ == "__main__":
    main(sys.argv[0:])

pass  # IF BLOCK
