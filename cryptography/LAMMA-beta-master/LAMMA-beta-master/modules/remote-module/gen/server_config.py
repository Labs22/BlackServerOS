# Project LAMMA
#
#                         __    _____ _____ _____ _____
#                        |  |  |  _  |     |     |  _  |
#                        |  |__|     | | | | | | |     |
#                        |_____|__|__|_|_|_|_|_|_|__|__|
#
#
#
#
#    Plugin Name    :   server_config.py
#    Plugin ID      :   rm-002
#    Plugin Purpose :   checks the remote server's SSL/TLS configuration
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
import sys
import getopt
import tempfile
from sys import stdout



# Plugin Informaiton


# ------------------------------------------------------------------------------
# Function      : log()
#
# Purpose       :   Logs the findings to a given Output Stream, it could be file or
#                   STDIO.
#                   All the output strings are catogarized in following types
#                   1.  General Findings
#                   2.  [INFO]  -   Additional information
#                   3.  [NOTE]  -   Note worthy information
#                   4.  [ALRT]  -   Findings with Low to Medium Severity
#                   5.  [WARN]  -   Findings from Medium to Critical severity
#
# Parameters    : 1. argv     (list)    - list of command line arguments
#
# Returns       : None


def log(message, sink):

   sink.write(message)

pass


# -------------------------------------------------------------------------------
# Function      : usage
#
# Purpose       : Proivdes a mini help on usage of this program
#
# Parameters    : None
#
# Returns       : None


def usage():
    print "\n\n    Purpose  :   This script connects the remote host over SSL/TLS and dumps the Session Information"
    print "\n\n    Usage    :"
    print "        -H [--help]      : prints this usage help"
    print "        -p [--port]      : port on which SSLTLS connection is to be made"
    print "        -h [--host]      : IP or Domain name of the remote host to be connected"
    print "        -o [--out]       : Out file in which findings will be reported"
    print " \n\n\n"

    sys.exit(2)
pass  # USAGE BLOCK



# -------------------------------------------------------------------------------
# Function      : main
#
# Purpose       : The main funciton of the script. Reads the configuration file for
#
# Parameters    : 1. argv     (list)    - list of command line arguments
#
# Returns       : None


def main(argv):
    # **** Lets process the arguments *********

    port = ""
    host = ""
    out_file = ""
    sink = stdout

    # ---- Show the usage for too few arguments

    if len(argv) < 2:
        usage()
        sys.exit(2)
    pass  # if block

    # --- try block

    try:
        opts, args = getopt.getopt(argv, "H:p:h:o:",
                                   ["help=", "port=", "host=", "out="])

    except getopt.GetoptError:
        usage()
        sys.exit(2)
    pass  # TRY BLOCK

    # ---- Process the arguments passed to the script

    for opt, arg in opts:
        if opt in ("-H", "--help"):
            usage()
        elif opt in ("-p", "--port"):
            port = arg
        elif opt in ("-h", "--host"):
            host = arg
        elif opt in ("-o", "--out"):
            out_file = arg
        pass  # IF BLOCK
    pass  # FOR BLOCK


    #--- NOTE This is hardcoded, Fix this
    ca_path = '/etc/ssl/certs/ca-certificates.crt'

    client_response = "HEAD / HTTP/1.0\r\n\r\n"

    if (int(port) != 443):  # --- IMAPs/POPs/SMTPs
        client_response = "QUIT\r\n"
    pass


    # ---- SSL Handshake

    if(out_file != ""):
        sink = open(out_file, "a+")
    else :
        sink = stdout

    log("\n--- Starting Server Config Checks for host - %s --- \n " %host, sink)

    with tempfile.TemporaryFile() as tmp:
        proc = subprocess.Popen(
            ['openssl', 's_client', '-quiet', '-connect', host + ':' + port, '-CAfile', ca_path],
            stderr=tmp, stdout=tmp, stdin=subprocess.PIPE
        )
        proc.communicate(client_response)

        if (proc):
            pass

        tmp.seek(0)

        # --- Find the certificate validity

        buff = str(tmp.read())
        print buff

        indx = buff.find("HTTP/")
        server_response = buff[indx:]

        log("\nServer Response :  \n\n%s" %server_response, sink)

        good_chain = True

        log("\n\nCertificate Chain validation :  \n\n" , sink)
        for line in buff.splitlines(True):
            rec = line.strip()
            if (rec.startswith('depth=')):
                for frag in rec.split(","):
                    item = frag.strip()
                    #print "--",item
                    pos = item.find("O =")
                    if(pos != -1):
                        log("\n Cert of %s :" % item[(pos+3):], sink)
                        break
                    else :
                        pos = item.find("CN =")
                        if (pos != -1) :
                            log("\n Cert of %s :" % item[(pos+4):], sink)
                            break
                        pass
                    pass
                pass
                continue
            pass

            if rec.startswith('verify'):
                if (rec.find("return:1") != -1):
                    log("\tis Valid ", sink)

                else:
                    log( line, sink)
                    good_chain = False
                    break
        pass

        if (good_chain):
            log ("\nCertificate Chain is verified & Trustable", sink)

        else:
            log ("\n[ALERT] Certificate Chain is not Trustable", sink)

    tmp = tempfile.NamedTemporaryFile(delete=True)

    log("\n--- Server Configuration checks Complete...", sink)

pass  # main

if __name__ == "__main__":
    main(sys.argv[1:])

pass  # IF BLOCK
