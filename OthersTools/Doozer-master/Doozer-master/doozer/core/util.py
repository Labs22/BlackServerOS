from datetime import date, datetime
from core.log import ERROR, LOG
import core.sethor as sethor
import os
import requests
import sys

""" utility functions for Doozer 
"""

def timestamp():
    return '%s %s' % (date.today().isoformat(),
                        datetime.now().strftime('%I:%M%p'))


def msg(string, level=LOG):
    """
    """
        
    if level is ERROR:
        print '\033[1;31m[-] %s\033[0m' % string

    with open(sethor.log_file, "a+") as f:
        f.write("[%s] %s\n" % (timestamp(), string))


def update_doozer(settings):
    """ Once cracking is completed, parse up the pot file
    and push it all to the core doozer database via the exposed
    API
    """

    try:
        msg("updating doozer with cracked passwords...")
       
        # verify the pot file exists
        if not settings.pot_file or not os.path.exists(settings.pot_file):
            return

        with open(settings.pot_file) as pot_file:

            for line in pot_file.readlines():
                if line.count(':') < 2: continue

                if line.count(':') > 2:
                    htype = line.rsplit(':')[1]
                    hsh = line.split(':')[0]
                    pswd = ''.join(line.split(':')[1:-1])
                else:
                    (hsh, pswd, htype) = line.split(':')

                response = requests.post("http://{0}:{1}/submit/".format(
                                                        sethor.DOOZER_IP,
                                                        sethor.DOOZER_PORT),
                               data={"hash":hsh, "val":pswd, "hashtype":htype})

                if response.status_code != 200:
                    msg("FATAL: error updating doozer (HTTP %d)" % (response.status_code))
                    break

    except IOError, e:
        msg("No hashes were cracked, no update to doozer necessary.")
    except Exception, e:
        msg("FATAL: error updating doozer: %s" % e)
        sys.exit(1)


def check_doozer(value, htype):
    """ Check if a hash is in our database

        @value is the hash
        @htype is the type of hash
        @return is the resulting plaintext if found, or None
    """

    found = None
    response = requests.post('http://{0}:{1}/fetch/'.format(
                                                    sethor.DOOZER_IP,
                                                    sethor.DOOZER_PORT),
                            data={'hash':value, 'hashtype':htype})

    if not 'HashModel matching query does not exist' in response.content:
        found = response.content.rstrip()
        
    return found
