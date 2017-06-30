#!/usr/bin/env python
# Doozer 
# Author: Bryan Alexander
#         Craig Freyman ( original concept/implementation )
#
# This version of doozer modularizes attacks and adds support
# for a Django app and file monitor system.
#

from core.generic_hash import GenericHash
from core.util import msg
from argparse import ArgumentParser
from os.path import abspath
from core.log import ERROR
import core.sethor as sethor
import core.util as util
import os
import pkgutil
import platform
import sys


class Settings:
    """ Not only does this manage our settings, but it also manages our current
    state.
    """

    def __init__(self):

        # set by options
        self.session_home = "%s/%s" % (sethor.WORKING_DIR, "")
        self.session = None                         # name of our session (-s)
        self.pot_file = None

    def configure(self, options):
        """ Prerun function used to setup the environment.
        """

        # setup options
        self.session = options.session
        self.session_home = "%s/%s" % (sethor.WORKING_DIR, self.session)
        sethor.log_file = "%s/hor.log" % self.session_home
        self.hash_file = "%s/%s" % (self.session_home, abspath(options.pwdump).split('/')[-1])

        # local requirements fulfilled, run a check on the supplied paths
        self._path_check()
    
    def _path_check(self):
        """ Validates file/folder locations defined in main
        """

        bpath = None
        if not os.path.exists(sethor.OPHCRACK_TABLES):
            bpath = sethor.OPHCRACK_TABLES
        elif not os.path.exists(sethor.WORDLIST_DIR):
            bpath = sethor.WORDLIST_DIR
        elif not os.path.exists(sethor.HASHCAT_BINARY):
            bpath = sethor.HASHCAT_BINARY
        elif not os.path.exists(sethor.HASHCAT_DIR):
            bpath = sethor.HASHCAT_DIR

        if bpath:
            msg("%s is an invalid path." % bpath, ERROR)
            sys.exit(1)


def loadAttacks():
    """ Load all of our attack modules located in attacks/
    """

    fpath = [abspath("./attacks")]
    modules = list(pkgutil.iter_modules(fpath))
    attacks = []
    
    for module in modules:

        mod = module[0].find_module(module[1]).load_module(module[1])

        try:
            mod = mod.Hash()
            attacks.append(mod)
        except Exception, e:
            continue

    return attacks


def readData():
    """ Reads in the supplied hash file and builds a list, one line per
    """

    master_input = None
    try:
        with open(settings.hash_file, "r") as f:
            master_input = [x.strip() for x in f.readlines()]
    except Exception, e:
        msg("FATAL: %s" % e, ERROR)
        sys.exit(1)

    return master_input


def main():
    """ 
    """
    
    # load all of our hash modules and read data
    attacks = loadAttacks()
    data = readData()

    if len(data) <= 0:
        # why?
        return

    for attack in attacks:
        
        # our attack needs to know about its potential session first 
        attack.session = settings.session
        attack.session_home = settings.session_home
        attack.hash_file = settings.hash_file

        #
        # Iterate over each attack until check returns True; this means
        # that we've identified the hash type and are ready to start cracking
        #
        if attack.check(data[0]):
    
            for entry in data:
                # iterate over each value and let the attack module
                # determine if the hash is acceptable or not
                attack.check(entry, False)

            msg("Starting attack on type %s" % attack.type)

            # set our pot file
            settings.pot_file = "%s/%s/%s.pot" % (sethor.WORKING_DIR, 
                                                  settings.session,
                                                  attack.type)

            # go go gadget hashcat
            attack.run()
            break

    # update doozer database with cracked hashes
    util.update_doozer(settings)
    msg("Cracked %d passwords" % len(attack.cracked_hashes))


def parse_args():
    """ Parse arguments to doozer 
    """

    parser = ArgumentParser(usage="[options]")
    parser.add_argument("-p", help="pwdump file", action='store', dest='pwdump',
                        required=True)
    parser.add_argument("-s", help="Session name", action='store', dest='session')

    options = parser.parse_args()

    return options


if __name__ == "__main__":
    """
    """

    if platform.system() == 'Windows':
        sys.exit(0)

    if os.geteuid() is not 0:
        sys.exit(0)

    options = parse_args()
    settings = Settings()
    settings.configure(options)

    # run
    main()
    msg("Session %s successfully completed." % settings.session)
