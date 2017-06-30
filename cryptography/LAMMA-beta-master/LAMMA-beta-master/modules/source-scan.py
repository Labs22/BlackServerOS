# Project LAMMA
#
#                         __    _____ _____ _____ _____
#                        |  |  |  _  |     |     |  _  |
#                        |  |__|     | | | | | | |     |
#                        |_____|__|__|_|_|_|_|_|_|__|__|
#
#                                     (beta)
#
#
#    Plugin Name    :   source_scan.py
#    Plugin ID      :   s-01
#    Plugin Purpose :   Invokes the scanning of source code repositories and libraries
#                       searcher for -  1.  Weak cipher implimentation
#                                       2.  Use of backdoored/deprecated schemes
#
#
#    Plugin Author  :   @ajithatti
#
#    Plugin Version :   0.0.1
#    Plugin Status  :   beta
#
################################################################################

# Include Section
import subprocess
import os
import os.path
import sys
import getopt
from sys import argv, stdout

# Plugin Informaiton

# Env Variable


# Plugin Mode

# Settings

# Report Findings



# --------------------------------------------------------------------------------
# Function      : usage
#
# Purpose       : Proivde a mini help on usage of this program
#
# Parameters    : None
#
# Returns       : None

def usage(script):
    print "\n\n     %s [-H] [-s] [-l] [-o] [-p]" %script
    print "\n       Purpose  : Scan a source code present in a repository present on --path"
    print "\n       with specified --script. Findings cane be stored in file specified by --out "
    print "\n               the results can be stored in a Repository for further comparision "
    print "\n"
    print "        -H [--help]      : prints this usage help"
    print "        -s [--script]    : scans the source code on givne Path with specified script"
    print "        -l [--list]      : list all the scripts on a specified path"
    print "        -p [--path]      : path of the source code to be scanned"
    print "        -o [--out]       : file to capture the findings of the scan"

    print " \n\n\n"

    sys.exit(2)


pass  # USAGE BLOCK






# --------------------------------------------------------------------------------
# Function      : list_all_source_scripts()
#
# Purpose       : list all the scripts in the source module
#
# Parameters    : path to be searched
#
# Returns       : None

def list_all_source_scripts(path):

    prefix = get_path_prefix()

    indir =  prefix + path

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




# --------------------------------------------------------------------------------
# Function      : get_path_prefix
#
# Purpose       : based on from where the script is invoked, it returns the cwd
#
# Parameters    : None
#
# Returns       : relative cwd


def get_path_prefix() :
    indir = os.getcwd()

    prefix = ""

    if (indir.endswith("LAMMA")):
        prefix = "modules/source-module/"

    if (indir.endswith("modules")):
        prefix = "source-module/"

    indir = indir + "/" + prefix

    return prefix
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

    out_file = ""
    in_file = ""
    path = ""
    skip_scan = False
    cmd_string = "python "
    sink = stdout
    other_opt = ""

    # ---- Show the usage for too few arguments

    print "We are in Source module"

    if len(argv) < 3:
        usage(this_script)
        sys.exit(2)
    pass  # if block

    # --- try block

    try:
        opts, args = getopt.getopt(argv[1:], "H:s:l:p:o:",
                                   ["help=", "script=", "list=", "path=", "out="])


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
            list_all_source_scripts(arg)

        elif opt in ("-p", "--path"):
            path = arg

        elif opt in ("-o", "--out"):
            out_file =  arg

        pass  # IF BLOCK
    pass  # FOR BLOCK

    # ---- collect the values and check


    # --- Check the scripts to be executed : flag [-s]

    if (script_set == ""):
        print ("\n\n>> No script[s] specified")
        print ("\n\t pls specify choice of your scripts to be executed")
        print ("\n\t\t -s  [script_name]")
        skip_scan = True
    else :
        cmd_string = cmd_string + " " + os.getcwd()+ "/" +get_path_prefix() + script_set

    pass  # if Block



    # -- Check for the Host+Port or Infile

    if (path == ""):
        print "\n\n>> Specify the path of stroe to be scanned"
        skip_scan = True
    else :
        cmd_string = cmd_string + " -p" + " " + path
    pass  # -- If block


    if(out_file != ""):
        cmd_string = cmd_string + " -o" + " " + out_file
    pass

    cmd_string = cmd_string + " " + other_opt

    # -- We have enouh configuration parameters to continue

    # -- Show the configuration before starting the scan ----

    print ("\n\n[+] Starting %s plugin\n\n " %script_set )

    print "\n\n\t Now Executing:  ", (cmd_string), "\n"


    subprocess.call(cmd_string, shell="false")

    print "[+] Exiting The Scan ...\n\n"


pass  # main

# -------------------------------------------------------------------------------
#
# Kick off the script

if __name__ == "__main__":
    main(sys.argv[0:])

pass  # IF BLOCK
