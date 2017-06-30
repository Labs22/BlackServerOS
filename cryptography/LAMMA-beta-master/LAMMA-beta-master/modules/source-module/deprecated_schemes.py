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
#    Plugin Name    :   deprecated_schemes.py
#    Plugin ID      :   sm-01
#    Plugin Purpose :   Searches for use of deprecated function and weak
#                       ciphers in a source code repository
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
import sys
from sys import argv, stdout



# --------------------------------------------------------------------------------
# Function      : usage
#
# Purpose       : Proivde a mini help on usage of this program
#
# Parameters    : None
#
# Returns       : None

def usage(script):
    print "\n\n  %s [-o] [-p]  " %script
    print "\n    Purpose  : Searches use of deprecated function and weak ciphers" 
    print "                 in a source code repository on --path and reports the "
    print "                 findings in file --out"
    print "\n"
    print "        -o [--out]       : stores the findings of the search in this file"
    print "        -p [--path]      : path of a source code repository"
    print " \n\n\n"

    sys.exit(2)


pass  # USAGE BLOCK



def log( message, sink) :

    sink.write(message)
pass


def check (pattern, header, dir, out_file):

    fout = stdout

    if(out_file!= ""):
        fout = open(out_file, "a+")
        if(not fout):
            "\n\n Filed to open file, skipping searching\n\n"
            return (2)
        pass
    pass


    print("\n\n\n\n ****** New Search ******* \n\n\n")


    r = re.compile(pattern, re.IGNORECASE)
    line_no = 0
    put_header = True
    for parent, dnames, fnames in os.walk(dir):
        for fname in fnames:
            filename = os.path.join(parent, fname)
            if os.path.isfile(filename):
                with open(filename) as f:
                    line_no = 1
                    for line in f:
                        line_no += 1
                        if r.search(line, re.IGNORECASE ):
                            if(put_header):
                                log( "\n\n\n", fout)
                                log( header, fout)
                                log( "\n\n\n", fout)
                                put_header = False
                
                            finding = (filename + "<" +str(line_no) +"> : "  +line)
                            log(finding, fout) 
                        pass
                    pass
                pass
            pass
        pass
    pass

pass



def rcheck (a,b,c,d):
    pass
pass



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
    out_file = ""
    this_script = argv[0]


    if len(argv) < 3:
        usage(this_script)
        sys.exit(2)
    pass  # if block

    # --- try block

    try:
        opts, args = getopt.getopt(argv[1:], "o:p:",["out=", "path="])


    except getopt.GetoptError:
        usage(this_script)
        sys.exit(2)
    pass  # TRY BLOCK

    #---- Process the arguments passed to the script

    for opt, arg in opts:

        if opt in ("-p", "--path"):
            path = arg 
        elif opt in ("-o", "--out"):
            out_file = arg 
        pass # IF BLOCK
    pass # FOR BLOCK


    if( path == ""):
        print "\n The source code directory path not specified Exiting \n\n"
        return (-2)
    pass


    #--- Sample check function ----
    #check( "search pattern", 
    #        "Description " +
    #        "\n\t Ref: ",
    #        path)

    #---- Chek for the useage of MDsum family
    check(  "md.?sum*\(|EVP_md.?\(", 
            "MD*Sum Family :\n\tThe entire family of MDSum (md3, md4, md5) are now considered Cryptographically insecure to use.\n\t Ref: https://en.wikipedia.org/wiki/MD5#Security", path, out_file)
      
    #---- Chek for the useage of SHAsum family
    check(  "sha.?sum*\(", 
            "SHA, SHA1 Hashes and HMAC : \n\tShaSum & SHA1sum are now considered insecure for hasing and HMAC schemes. but still in use for fingerprinting"+
            "\n\tRef : https://en.wikipedia.org/wiki/Secure_Hash_Algorithm",
            path, out_file)
    
    #---- Chek for the useage of ECB mode ciphers
    check(  "ecb_encrypt*\(", 
            "Electronic Code Book (ECB): \n\t ECB mode is an insecure Block Cipher mode"+
            "\n\t Ref: https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation#Electronic_Codebook_.28ECB.29",
            path, out_file)
 
    #---- Chek for the useage of CBC mode ciphers
    check(  "cbc\(|cbc_encrypt*\(", 
            "Cipher Block Chaining (CBC): \n\t CBC is an insecure block cipher scheme,  and susciptable to many Padding Oracle Attacks \n\t Ref:  CVE-2014-3566,  CVE-2013-0169, CVE-2016-2107",
            path, out_file)
 



pass # main





#-------------------------------------------------------------------------------
#
# Kick off the script

if __name__ == "__main__":

    main (sys.argv[0:])

pass # IF BLOCK
