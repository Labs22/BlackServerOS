#!/usr/bin/python

from argparse import ArgumentParser
from passlib.hash import nthash, lmhash
from subprocess import check_output, Popen, PIPE
from os import system, listdir, geteuid
from time import sleep
from shlex import split
import core.sethor as sethor
import hashlib
import binascii
import sys
import requests

""" utility application for working with Doozer.  This can be used
independent of Doozer, so long as you configure the IP and PORT
as shown below
"""

DOOZER_IP = sethor.DOOZER_IP 
DOOZER_PORT = sethor.DOOZER_PORT
LOG = 1
ERROR = 2

def genhash(password, htype):
    """ Generate one of our supported hashes
    """

    hsh = None
    if htype == "nt":
        hsh = nthash.encrypt(password)
    elif htype == "lm":
        hsh = lmhash.encrypt(password)

    return hsh


def msg(string, log=LOG):
    """ Format output message
    """

    if log is LOG:
        print '\033[1;32m[!] %s\033[0m' % string
    elif log is ERROR:
        print '\033[1;31m[-] %s\033[0m' % string


def run(options):
    """ 
    """

    if options.plaintext:
        if not options.hash_type:
            msg('You must specify a hash type for this option (-f)', ERROR)
            return

        # read file
        try:
            plaintexts = []
            with open(options.plaintext, "r") as f:
                plaintexts = [x.decode('ascii', 'ignore').strip() for x in f.readlines()]
        except Exception, e:
            msg('Error: %s' % e, ERROR)
            return

        generated = []
        msg("Generating hashes...")
        for password in plaintexts:
            hsh = genhash(password, options.hash_type)
            if hsh: 
                generated.append((password, hsh, options.hash_type))
            else:
                msg('Could not generate hash for %s:%s' % (password, options.hash_type), ERROR)
                sys.exit(1)

        msg("Generated %d hashes, inserting now..." % len(generated))
        insert_into_db(generated)

    if options.add_list:
        if not options.hash_type:
            msg('You must specify a hash type for this option (-f)', ERROR)
            return

        try:
            alist = []
            with open(options.add_list, "r") as f:
                alist = [x.strip() for x in f.readlines()]
        except Exception, e:
            msg('Error: %s' % e, ERROR)
            return

        msg("Parsing file...")
        generated = []
        for entry in alist:
            (pswd, hsh) = entry.split(" : ")
            if not pswd or not hsh:
                msg('Failed to parse entry %s, skipping.' % entry, ERROR)
                continue

            generated.append((pswd, hsh, options.hash_type))

        msg("Generated %d hashes, inserting now..." % len(generated))
        insert_into_db(generated)

    if options.gplaintext:
        if not test():
            msg("Failed to connect to doozer", ERROR)
            return

        try:
            r = requests.get("http://%s:%s/pwlist/" % (DOOZER_IP, DOOZER_PORT))
            if r.status_code == 200:
                with open("plaintext_list.txt", "w") as f:
                    f.write(r.content)

            msg("Password list written to plaintext_list.txt")

        except Exception, e:
            msg("Failed to fetch password list: %s" % e, ERROR)

    if options.kill:

        try:
            system("pkill -SIGKILL -f \"python monitor.py\"")
            system("pkill -SIGKILL -f \"manage.py\"")
        except:
            pass
        
        msg("Processes killed.")

    if options.startup:
        msg("Firing up server...")
        tmp = check_output("ps aux | grep monitor[.py] | awk '{print $2}'", shell=True).rstrip()
        
        if len(tmp) > 1:
            msg("It appears to already be running!")
        else:
            system("python monitor.py")
            Popen(["python","manage.py","runserver", "0.0.0.0:8000"], 
                            stdout=PIPE, stderr=PIPE, cwd="../")
            sleep(2)
            if test():
                msg("Doozer should now be up on port 8000")
            else:
                msg("Doozer isn't responding", ERROR)

    if options.archive:
        # just move all our sessions to the archive location

        _ = raw_input("Ensure all jobs are stopped before performing this! [enter]")

        to_archive = listdir(sethor.WORKING_DIR)
        msg("Preparing to archive %d jobs..." % len(to_archive))    
        system("mv %s/* %s" % (sethor.WORKING_DIR, sethor.SESSION_ARCHIVE))
        msg("All jobs moved.")


def insert_into_db(cred_list):
    """ Takes a list of tuples in the format (password, hash, type) and inserts
    them into the database
    """

    if not test():
        msg('Failed to connect to doozer', ERROR)
        sys.exit(1)

    for (idx, cred) in enumerate(cred_list):
        status = requests.post("http://{0}:{1}/submit/".format(DOOZER_IP, DOOZER_PORT),
                               data={"hash":cred[1], "val":cred[0], "hashtype":cred[2]})

        if status.status_code != 200:
            msg("Error inserting %s (HTTP %d)" % (cred, status.status_code), ERROR)
            sys.exit(1)

        p = 100 * float(idx) / float(len(cred_list))
        if p % 25.0 == 0:
            msg("{0}% complete.".format(p))

    msg("All hashes inserted successfully")


def test():
    """ Test the connection to the remote doozer box; returns true or false
    """

    success = False
    try:
        response = requests.post("http://%s:%s/check/" % (DOOZER_IP, DOOZER_PORT), 
                                data={"hash":"5f4dcc3b5aa765d61d8327deb882cf99"})
    except: 
        pass 
    else:
        if response.status_code == 200:
            success = True

    return success


def parse_args():
    """
    """

    hash_types = ["nt", "lm"]
    parser = ArgumentParser(usage="[options]")
    parser.add_argument("-a", help="Add cracked hashes to database."
                                   " Format should be password : hash",
                        action='store', dest='add_list', metavar='[file]')
    parser.add_argument("-p", help="Add a list of plaintext credentials to the database",
                    action='store', dest='plaintext', metavar='[file]')
    parser.add_argument("-f", help="Hash format to create if using -a",
                    action='store', dest='hash_type', choices=hash_types)
    parser.add_argument("-k", help="Kill Doozer", action='store_true',
                    dest='kill')
    parser.add_argument("--startup", help="Startup Doozer", action='store_true',
                    dest='startup')
    parser.add_argument("--plaintext", help='Pull a list of all plaintext passwords',
                    action='store_true', dest='gplaintext')
    parser.add_argument("--test", help="Test connection to Doozer",
                    action='store_true', dest='test')
    parser.add_argument("--archive", help="Archive all jobs",
                    action='store_true', dest='archive')

    options = parser.parse_args()
    if len(sys.argv) <= 1:
        parser.print_help()
        sys.exit(1)

    return options


if __name__ == "__main__":

    print '\n\t\033[1;34mdoozestop - Doozer utility\n\033[0m'

    if geteuid() != 0:
        msg("sudo me captain", ERROR)
        sys.exit(1)

    options = parse_args()

    if options.test:
        if not test():
            msg('Failed to connect to doozer', ERROR)
            sys.exit(1)
        else:
            msg("Successfully connected to Doozer")

    run(options)
