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
#    Plugin Name    :   scan_ssl.py
#    Plugin ID      :   r-01
#    Plugin Purpose :   scans a remote SSL/TLS server and carries out following
#                        checks
#                            Protocol Support
#                            Cipher Suites
#                            Analyze server certificate
#                               Certificate verification type (EV/OV/DV)
#                               Subject, Issuer info,
#                               Alternate subject names
#                               Self-Signed or not
#                               Cert signing scheme
#
#
#    Plugin Author  :   @ajithatti
#
#    Plugin Version :   0.0.1
#    Plugin Status  :   beta
#
################################################################################

# Include Section

import sys
import getopt
import tempfile


# from socket import socket
import socket
from sys import argv, stdout

# Include modules specifi to pyOpenssl
import OpenSSL as OpenSSL

import OpenSSL.crypto as crypto
from OpenSSL.SSL import SSLv23_METHOD, TLSv1_METHOD, TLSv1_2_METHOD, Context, Connection




# Plugin Informaiton

# -------------------------------------------------------------------------------
# Function      :   cert_timeline_analysis()
#
# Purpose       :   Based on certificate "start_date", lists out SSL/TLS vulnerabilities
#                   which can compromise the Private key.
#
# Parameters    :   certificate_start_date
#
# Returns       :   True if any vulnerability is applicable else false


def cert_timeline_analysis (cert_start_date, sink):


    change_cert = False
    vuln_dict = {   "20160103" :    "DROWN : Decrypting RSA using Obsolete and Weakened eNcryption. \n\t\t Applicable CVES - CVE-2016-0800, CVE-2016-0703, CVE-2015-0293, CVE-2016-0704 \n",

                    "20140304" :    "HEART BLEED :The buffer-over-read vulnerability of the Heartbeat module \n\t\t Applicable CVEs - CVE-2014-0160 \n"
                }


    print "\nCertificate Time Line Analysis : \n"
    for vdate in vuln_dict:
        if (int(cert_start_date) <= int(vdate)):

            change_cert = True
            log ("\t[ALERT] The private key might be compromised due to  :\n\t %s" %vuln_dict[vdate], sink)
        pass
    pass

    if(change_cert):
        log ("\t[NOTE] Certificate Timeline analysis is an experimental feture. For precaution you can renew your private key and the certificate\n", sink )
    else:
        log ("\t[INFO] No vulnerability found in recent time which could have compromised your private keys\n", sink)

    return change_cert

pass






# Plugin Informaiton

# -------------------------------------------------------------------------------
# Function      : usage
#
# Purpose       : Proivde a mini help on usage of this program
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
    print "        -o [--out]       : also downloads the PDF packages "
    print "        -c [--capath]    : locaiton of trusted root certificates. Linux default path is set by default"
    print " \n\n\n"

    sys.exit(2)


pass  # USAGE BLOCK



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



# ------------------------------------------------------------------------------
# Function      : cert_verification()
#
# Purpose       : Determines the verification type of the certificate, Namely
#                   EV  -   Extended Verificaiton
#                   OV  -   Organisation Verification
#                   DV  -   Domain Verification
#
# Parameters    : policy    - x509 certificate policy object
#
# Returns       : String - Type of the cert (EV|DV|OV|null)

def cert_verification_type(policy):
    # print "\n\n The policy of the cert :", policy, "\n\n"

    if (policy.find("2.23.140.1.2.1") != -1):
        return "DV"
    elif (policy.find("2.23.140.1.2.2") != -1):
        return "OV"
    else:
        return ""


pass

# def cert_verify_cb(conn, x509, errno, errdepth, retcode):
#  """
#  callback for certificate validation
#  should return true if verification passes and false otherwise
#  """
#  if errno == 0:
#    if errdepth != 0:
#      # don't validate names of root certificates
#      return True
#    else:
#      #if x509.get_subject().commonName != HOST:

#        return False
#  else:
#    return False
pass


def get_x509_val(x509_name, label):
    for tag in x509_name:
        if (tag[0] == label):
            return tag[1]


pass




# ------------------------------------------------------------------------------
# Function      : main()
#
# Purpose       : The main funciton of the script. Reads the configuration file for
#
# Parameters    : 1. argv     (list)    - list of command line arguments
#
# Returns       : None

def main(argv):
    # **** Lets process the arguments *********
    port = 0
    host = ""
    out_file = ""
    ca_path = ""
    sink = stdout       # By default dump all the findings on screen

    # ---- Show the usage for too few arguments

    if len(argv) < 2:
        usage()
        sys.exit(2)
    pass  # if block

    # --- try block

    try:
        opts, args = getopt.getopt(argv, "H:p:h:o:c",
                                   ["help=", "port=", "host=", "out=", "capath="])

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
        elif opt in ("-c", "--capath"):
            ca_path = arg
        pass  # IF BLOCK
    pass  # FOR BLOCK


    if (out_file != ""):
        sink = open(out_file, "a+w")

    # ---- Get the path of CA-Root Certs

    if (ca_path == ""):
        ca_path = '/etc/ssl/certs/ca-certificates.crt'  # Default CA path on many Linux machines.

    # ---- SSL Handshake

    with tempfile.TemporaryFile() as tmp:
        tmp.seek(0)
        print  tmp.read()
    tmp = tempfile.NamedTemporaryFile(delete=True)

    #--- Start the scanning process

    log(("\n\n--- Scanning host : %s  started ---\n\n" %host), sink)

    # ---- Prepare Context for SSL Handshake

    try:
        soc = socket.socket()
        soc.connect((host, int(port)))

    except socket.error, err:
        log(("Socket Error Encountered : %s" % err), sink)
        exit(1)
    pass

    # --- Connect to remote host

    soc_context = Context(SSLv23_METHOD)
    soc_ssl = Connection(soc_context, soc)
    soc_ssl.set_connect_state()
    soc_ssl.set_tlsext_host_name(argv[1])

    try:
        soc_ssl.do_handshake()
    except OpenSSL.SSL.Error, msg:
        print " \n\n Unable to complet the SSL Handshake %s" % msg
        exit(1)
    pass

    #--- Get the remote host name

    rhost = soc.getpeername()

    log(("\nRemote Host name :" + host), sink)
    log(("\nRemote Host IPv4 :" + rhost[0]), sink)
    log(("\nRemote Host Port :" + str(rhost[1])), sink)


    #--- Get and Analyse Server Certificate

    cert = soc_ssl.get_peer_certificate()
    cipher = soc_ssl.get_cipher_name()

    log(("\nCipher Suite used : " + cipher), sink)


    #--- Get Subject Info

    subject_comps = cert.get_subject().get_components()
    subject_name = cert.get_subject().commonName

    if (not subject_name):
        subject_name = get_x509_val(subject_comps, "O")

    log("\nSubject Name = " + subject_name, sink)

    subject_email = cert.get_subject().emailAddress


    if (subject_email):
        log("\n[NOTE] Information Disclosure, Subject's Email ID <%s> found in certificate"  %subject_email, sink)


    #--- Get issuer Info

    issuer_name = cert.get_issuer().commonName
    if (not issuer_name):
        issuer_name = get_x509_val(issuer_comps, "O")
    log("\nIssuer Name = " + issuer_name, sink)

    if (subject_name == issuer_name):
        log("\n\n[NOTE] The certificate is SelfSigned, Please ensure it is from a trusted source\n\n", sink)
    pass

    # -- Dumps the Peer Certificate obtained over the connection ---

    cert_starts = cert.get_notBefore()
    cert_ends = cert.get_notAfter()

    log("\nStart Date : " + cert_starts, sink)

    cert_timeline_analysis(cert_starts[:8], sink)

    log("\nEnd Date   : " + cert_ends, sink)


    #--- Time line Analyize the certificate for applicable CVEs

    # cert_timeline_analysis(cert_starts)


    # print "\nSerial Number", cert.get_serial_number()

    sign_algo = cert.get_signature_algorithm()

    # --- Check for Algorithm Signing Strength -----

    if (sign_algo.find("sha1With") != -1 or sign_algo.find("shaWith") != -1):

        log("\n\n[ALERT] \"%s\" is a weaker Hashing scheme to used to sign this certificate \n\n" %sign_algo, sink)

    else:
        log("\nSignature Algorithm : " + sign_algo, sink)
    pass

    # --- Extract the Extensions from the Cert ---

    ext_count = cert.get_extension_count()

    for i in range(0, ext_count):
        cert_ext = cert.get_extension(index=i)
        ext_tag = cert_ext.get_short_name()

        if (ext_tag == "subjectAltName"):
            names = cert_ext.__str__()
            names = names.replace(",", "\n\t")
            names = names.replace("DNS:", "")
            log(("\n" + ext_tag + ":\n\t" + names), sink)

            # --- Verify if the Host-name we are connected to is in SAN list
            if (names.find(host) == -1):
                log(("\n\n[ALERT] Subject Name dosn't Match the Host Name \n\n"), sink)


        elif (ext_tag == "certificatePolicies"):
            cert_type = cert_verification_type(str(cert_ext))
            if (cert_type == ""):
                if (issuer_name.find(" EV ") != -1 or issuer_name.find("Extended Validation") != -1):
                    cert_type = "EV"
                else:
                    cert_type = "Cant Determine"
                pass
            pass
            log(("\n[INFO] The Certificates Verification type : " + cert_type), sink)
    pass

    # --- Cert pub key

    pub_key = cert.get_pubkey()
    pub_key_size = pub_key.bits()

    if ((pub_key_size < 2048) and (pub_key.type() == OpenSSL.crypto.TYPE_RSA)):
        log("\n\n[ALERT] Public Key Size [%s] and it is considered insecure" % str(pub_key_size), sink)
    else:
        log("\nPublic Key Size [%s]" % str(pub_key_size), sink)
    pass

    log(("\n\n--- End of the SSL Scan --- \n\n"), sink)


    #---Close the sink
    sink.close()


pass  # main

if __name__ == "__main__":
    main(sys.argv[1:])

pass  # IF BLOCK
