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
#    Plugin Name    :   track_private_key.py
#    Plugin ID      :   tm-01
#    Plugin Purpose :   Searches for private keys on a given path
#                        checks
#                            Private key is encrypted or not.
#                            Cehcks duplicate instances of the keys
#                             notes the locaion of the key
#
#    Plugin Author  :   @ajithatti
#
#    Plugin Version :   0.0.1
#    Plugin Status  :   beta
#
################################################################################



# Include Section
import os.path
import getopt
import os
import re
import hashlib
from tempfile import mkstemp
from shutil import move
from os import remove
import sys


#-- Global Variables

priv_key_search = "PRIVATE KEY-----"
pub_key_search = "PUBLIC KEY-----"




# --------------------------------------------------------------------------------
# Function      :   scan_private_keys()
#
# Purpose       :   stores the private keys in repo, if duplicate, appends path and
#                   increments the count
#
# Parameters    :   scan_path    -   path to search private keys
#                   repo_file    -   repository of keys
#


def scan_private_keys(scan_path, repo_file):
    pattern = "PRIVATE KEY-----"
    r = re.compile(pattern, re.IGNORECASE)

    encrypttern = "ENCRYPTED"
    encr = re.compile(encrypttern, re.IGNORECASE)

    line_no = 0
    key_found = False
    ENC_found = False
    path_exists = False

    print "path = ", scan_path
    print "repo_file = ", repo_file


    # --- Iterate from the root
    for parent, dnames, fnames in os.walk(scan_path):

        path_exists = True
        for fname in fnames:  # --- Through the files
            filename = os.path.join(parent, fname)
            if os.path.isfile(filename):  # --- File exists
                # ---- Search in the file
                key_found = False  # --- reset the flags
                ENC_found = False
                with open(filename) as f:  # --- file read
                    line_no = 1
                    for line in f:  # --- iterate line by line
                        line_no += 1
                        if (key_found):  # --- If private key
                            if encr.search(line, re.IGNORECASE):  # --- Check if file is encrypted or not
                                ENC_found = True
                                break;
                            else:
                                continue
                        if r.search(line, re.IGNORECASE):  # --- Check for file is a private key
                            key_found = True
                    pass
                pass
                if (key_found):  # --- Post assessing the file
                    if (ENC_found):
                        print ("\nKey-file [%s] is encrypted \n" %filename)
                        pass
                    else:
                        print ("\n[ALERT] Key-file [%s] is not encrypted\n" %filename)
                        pass

                    if (repo_file != ""):  # --- Lets collect the finger print of the keys
                        collect_keys(filename, repo_file, ENC_found)
                    pass
                pass
            pass
        pass
    pass

    if( not path_exists):
        print "\n\n>> Invalid path : %s\n\n" %scan_path
        return (-2)
    pass

    return 0
pass



# --------------------------------------------------------------------------------
# Function      :   replace()
#
# Purpose       :   stores the private keys in repo, if duplicate, appends path and
#                   increments the count
#
# Parameters    :   source_file_path    -   file to be written
#                   pattern             -   line to be replaced
#                   substring           -   new line
# Returns       :   None



def replace(source_file_path, pattern, substring):
    fh, target_file_path = mkstemp()
    with open(target_file_path, 'w') as target_file:
        with open(source_file_path, 'r') as source_file:
            for line in source_file:
                target_file.write(line.replace(pattern, substring))
    remove(source_file_path)
    move(target_file_path, source_file_path)


pass



# --------------------------------------------------------------------------------
# Function      :   collect_keys
#
# Purpose       :   stores the private keys in repo, if duplicate, appends path and
#                   increments the count
#
# Parameters    :   key_file    -   key file to be searched
#                   key_repo    -   repository of keys
#                   ENC         -   Flag, E if file is encrypted N if not
# Returns       :   None

def collect_keys(key_file, key_repo, Enc):
    kf = open(key_file, "r+")

    # --- Generate file hash
    raw_hash = hashlib.sha1(kf.read())
    file_hash = raw_hash.hexdigest()

    kf.close()

    # print "\n\n The has of the file = \n\n", file_hash
    hash_patt = re.compile(file_hash)
    loc_patt = re.compile(key_file)

    if (Enc):
        en_flag = "[E]"
    else:
        en_flag = "[N]"

    # --- search the file in the repo
    key_found = False

    f = open(key_repo, "a+")  # --- file read

    for line in f:  # --- iterate line by line
        if hash_patt.search(line, re.IGNORECASE):  # ---

            key_found = True
            # --- If found, compare the path
            if loc_patt.search(line, re.IGNORECASE):  # ---
                print "\nduplicate instance\n"
            else:
                print "\nsame key different locattion\n"
                # ------
                p = line[line.find("<") + 1:line.find(">")]
                count = int(p) + 1
                old_count = "<" + p + ">"
                new_count = "<" + str(count) + ">"
                prefix = line.rstrip()
                prefix = prefix.replace(old_count, new_count)
                suffix = " +" + key_file + "\n"
                newline = prefix + suffix
                print "\nThe old record = ", line
                print " \nThe new record = ", newline
                # f.close()
                replace(key_repo, line, newline)
                # ------
                pass  # --- add path and count
            pass
            break
        pass
    pass
    if (key_found == False):
        entry = "<1>:  " + en_flag + "  {" + file_hash + "} +" + key_file + "\n"
        print entry
        f.write(entry)
        f.close()
    pass
    pass


pass



# --------------------------------------------------------------------------------
# Function      : usage
#
# Purpose       : Proivde a mini help on usage of this program
#
# Parameters    : None
#
# Returns       : None

def usage(this_script):
    print "\n\n    %s [-i] [-r] [-p]  " %this_script
    print "\n      Purpose  : Seacrch for insecurly stored private keys, & collect the"
    print "        findings in a common repository. This repository can be used to "
    print "        find multiple instances of a private keys, location and track them."
    print "\n"
    print "        -i [--in]      : in put a private key file to search similar instances"
    print "        -r [--repo]      : repository to note and compare the findings"
    print "        -p [--path]      : path to search privet key in it"
    print " \n\n\n"

    sys.exit(2)


pass  # USAGE BLOCK






# --------------------------------------------------------------------------------
# Function      :   main
#
# Purpose       :   scans a directory for private keys and notes the finding in the
#                    report file.
#
# Parameters    :   path    -   to search private kye
#                   repo    -   repository to note and compare the findings
#                   in      -   in put a private key file to search similar instances
#
# Returns       :   None


def main(argv):


    # Initialize the variables
    path = ""
    repo_file = ""
    in_key_file =""
    this_script = argv[0]

    print "We are in Trust module"

    if len(argv) < 3:
        usage(this_script)
        sys.exit(2)
    pass  # if block

    # --- try block

    try:
        opts, args = getopt.getopt(argv[1:], "r:i:p:",
                                   ["repo=", "in=", "path="])


    except getopt.GetoptError:
        usage(this_script)
        sys.exit(2)
    pass  # TRY BLOCK

    # ---- Process the arguments passed to the script

    for opt, arg in opts:

        if opt in ("-i", "--out"):
            in_key_file = arg

        elif opt in ("-p", "--path"):
            path = arg

        elif opt in ("-r", "--repo"):
            repo_file = arg
            print "repo file =" , repo_file
        pass  # IF BLOCK
    pass  # FOR BLOCK

    print "\n\n The path =", path

    # ---- collect the values and check

    ret = scan_private_keys(path, repo_file)

    # --- Check the scripts to be executed : flag [-s]
    return ret
pass


# -------------------------------------------------------------------------------
#
# Kick off the script

if __name__ == "__main__":
    main(sys.argv[0:])

pass  # IF BLOCK
