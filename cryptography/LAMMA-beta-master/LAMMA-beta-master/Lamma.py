from cmd2 import Cmd, make_option, options
import subprocess
from  modules import *



# Document Header

__author__      =   "Ajit Hatti"
__copyright__   =   "Left"
__credits__     =   ["Payatu Technologies - http://payatu.com", "Null Community - http://nullcon.net",
                     "SSLLabs - https://ssllabs.com"]
__license__     =   "Completely Open and free for all"
__version__     =   "0.0.1"
__maintainer__  =   "@ajithatti"
__status__      =   "BETA"



# Universal Variables

prog = "python"

remote_script   =    " ./modules/remote.py"
source_script   =    " ./modules/source-scan.py"
trust_script    =    " ./modules/trust-scan.py"
crypto_script   =    " ./modules/crypto-checks.py"



#import cmd

class main(Cmd):
    """\n\n LAMMA : The Crypto-Auditing Framework\n\n"""

    intro = """\n\n 
                                                       
                         __    _____ _____ _____ _____ 
                        |  |  |  _  |     |     |  _  |
                        |  |__|     | | | | | | |     |
                        |_____|__|__|_|_|_|_|_|_|__|__|
                                                      
                                    (BETA)


                Vulnerability Assessment and Auditing Framework 
                      for all the Crypto Implementations.


                           (An Open Source Project)

                                      by

                                SECURITY MONX
 
    \n\n"""

    prompt = 'LAMMA : '

    Cmd.colors = True
    Cmd.ruler = ""


    Cmd.doc_header = """Welcome to LAMMA : \n
Primary Commands
----------------------------------------------

remote  -   module to test a remote server
crypto  -   module to test the quality of basic cryptographic schemes
source  -   module to find use of weak, deprecated or backdoored cipher schemes in source code
trust   -   module to detect - insecure private keys, un-trusted certificates in a key/trust store


All commands supported by LAMMA Shell
---------------------------------------------
"""
    Cmd.misc_header = ""
    Cmd.undoc_header = """\n\nWant \'help\' or 'exit' LAMMA?
---------------------------------------------
"""

    def do_remote(self, line):
        """  Scans a remote host and reports the SSL/TLS configuration & applicable vulnerabilities\n
        """
        subprocess.call(prog + remote_script +" " + line, shell="false")


    def do_trust(self, line):
        """  Scans given trust/key stores for -  untrusted certs, insecure private keys,\n
        """
        subprocess.call(prog + trust_script + " " + line, shell="false")


    def do_crypto(self, line):
        """  Generates & analyzes the quality, strength of - keys, hashes, random number under various schemes\n
        """
        subprocess.call(prog + crypto_script +" " + line, shell="false")


    def do_source(self, line):
        """  Scans the source code for weak, deprecated or backdoored functions\n
        """
        subprocess.call(prog + source_script +" " + line, shell="false")



pass    # Class Main Close


if __name__ == '__main__':
    main().cmdloop()

