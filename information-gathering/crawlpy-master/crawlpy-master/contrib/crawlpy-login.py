#!/usr/bin/env python
#-*- coding: utf-8 -*-

import sys
import getopt
import os.path
import json
import subprocess
import re

__author__ = "cytopia"
__email__ = "cytopia@everythingcli.org"
__license__ = "MIT"
__version__ = '0.2'
__date__ = '2016-08-15'



# Fix UTF-8 problems inside dict()
reload(sys)
sys.setdefaultencoding('utf8')


################################################################################
# File Class
################################################################################
class MyFile(object):

    #----------------------------------------------------------------------
    @staticmethod
    def read(path):
        fp = open(path)
        data = fp.read()
        fp.close()
        return data


################################################################################
# Json Class
################################################################################
class MyJson(object):

    #----------------------------------------------------------------------
    @staticmethod
    def _toAscii(input):
        if isinstance(input, dict):
            return {MyJson._toAscii(key): MyJson._toAscii(value) for key, value in input.iteritems()}
        elif isinstance(input, list):
            return [MyJson._toAscii(element) for element in input]
        elif isinstance(input, unicode):
            return input.encode('utf-8')
        else:
            return input

    #----------------------------------------------------------------------
    @staticmethod
    def validateFile(path):
        json_string = MyFile.read(path)
        return MyJson.validateString(json_string)

    #----------------------------------------------------------------------
    @staticmethod
    def validateString(json_string):
        try:
            json_object = json.loads(json_string)
        except ValueError, e:
            return False
        return True

    #----------------------------------------------------------------------
    @staticmethod
    def convertFile2dict(path):
        json_string = MyFile.read(path)
        return MyJson.convertString2dict(json_string)

    #----------------------------------------------------------------------
    @staticmethod
    def convertString2dict(json_string):
        # Remove unicide
        ujdict = json.loads(json_string)
        jdict = MyJson._toAscii(ujdict)
        return jdict



################################################################################
# Shell Class
################################################################################
class MyShell(object):

    #----------------------------------------------------------------------
    @staticmethod
    def which(program):
        def is_exe(fpath):
            return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

        fpath, fname = os.path.split(program)
        if fpath:
            if is_exe(program):
                return program
        else:
            for path in os.environ["PATH"].split(os.pathsep):
                path = path.strip('"')
                exe_file = os.path.join(path, program)
                if is_exe(exe_file):
                    return exe_file

        return None

    #----------------------------------------------------------------------
    @staticmethod
    def run(args, output, show_cmd=False, show_return=False, cmd_color='green'):

        if show_cmd:
            print MyShell.color(cmd_color) + '$ ' + ' '.join(args) + MyShell.color('reset')

        #retval = subprocess.call(args, shell=False, stdout=stdout)
        try:
            retval = 0
            output[0] = subprocess.check_output(args, shell=False)
        except subprocess.CalledProcessError as err:
            retval = err.returncode
            output[0] = err.output


        if show_return:
            print retval

        return retval


    #----------------------------------------------------------------------
    @staticmethod
    def color(color):
        if color == 'red':
            return '\033[0;31m'
        elif color == 'green':
            return '\033[0;32m'
        elif color == 'brown':
            return '\033[0;33m'
        elif color == 'blue':
            return '\033[0;34m'
        elif color == 'magenta':
            return '\033[0;35m'
        elif color == 'cyan':
            return '\033[0;36m'
        else:
            return '\033[0m'




################################################################################
# Function
################################################################################


#----------------------------------------------------------------------
def usage():
    filename = os.path.basename(sys.argv[0])

    print 'Usage: ' + filename + ' -C conf.json [-c cookie.txt] [-o output.html] [-y] [-v]'
    print '       ' + filename + ' -h'
    print '       ' + filename + ' -V'
    print
    print filename + ' will test whether or not the specified crawlpy config'
    print 'is valid and can successfully login.'
    print
    print 'You can optionally save a login session cookie (-c/--cookie) in wget format'
    print 'which can be used by tools such as sqlmap.'
    print
    print 'You can also store the html output from a successfull/unsuccessful login'
    print 'to file (-o/--output).'
    print
    print
    print "Required arguments:"
    print "  -C, --config=      Path to crawlpy json config."
    print "                         -C /path/to/conf.json"
    print "                         --config=/path/to/conf.json"
    print
    print "Optional arguments:"
    print "  -c, --cookie=      Path where to store the session cookie."
    print "                         -c /path/to/cookie.txt"
    print "                         --cookie=/path/to/cookie.txt"
    print
    print "  -o, --output=      Path where to store the html source after logging in."
    print "                         -o /path/to/login.html"
    print "                         --cookie=/path/to/login.html"
    print
    print "  -v, --verbose      Be more verbose."
    print
    print "  -y, --yes          Answer 'yes' to all questions."
    print
    print "System options:"
    print "  -h, --help         Show help."
    print "  -V, --version      Show version information."


#----------------------------------------------------------------------
def credits():
    filename = os.path.basename(sys.argv[0])
    print filename + ' v' + __version__ + ' (' + __date__ + ')'
    print __author__ + ' <' + __email__ + '>'


#----------------------------------------------------------------------
def check_requirements():

    if MyShell().which('wget') is None:
        print "wget is required, but not found."
        return False

    return True


#----------------------------------------------------------------------
def get_arguments(argv):

    # Parse command line arguments
    try:
        opts, args = getopt.getopt(argv, 'C:c:o:vyhV', ['config=', 'cookie=', 'output=', 'verbose', 'yes', 'help', 'version'])
    except getopt.GetoptError:
        print "Invalid argument(s)"
        usage()
        sys.exit(2)

    # Get values from command line arguments
    for opt, arg in opts:
        if opt in ("-C", "--config"):
            config = arg
        elif opt in ("-c", "--cookie"):
            cookie = arg
        elif opt in ("-o", "--output"):
            output = arg
        elif opt in ("-v", "--verbose"):
            verbose = True
        elif opt in ("-y", "--yes"):
            yes = True
        elif opt in ("-h", "--help"):
            usage()
            sys.exit()
        elif opt in ("-V", "--version"):
            credits()
            sys.exit()
        else:
            print "Invalid argument: " + opt
            usage()
            sys.exit(2)

    # Check existance of command line arguments
    if 'config' not in locals():
        print "Missing -C, --config argument"
        usage()
        sys.exit(2)

    # Set default values
    if 'cookie' not in locals():
        cookie = False
    if 'output' not in locals():
        output = False
    if 'verbose' not in locals():
        verbose = False
    if 'yes' not in locals():
        yes = False

    # Return values
    return config, cookie, output, verbose, yes






################################################################################
# Main Entry Point
################################################################################


if __name__ == "__main__":

    # Retrieve cmd arguments
    config, cookie, output, verbose, yes = get_arguments(sys.argv[1:])


    # Check requirements
    if not check_requirements():
        sys.exit(2)

    # Check if config file exists
    if not os.path.isfile(config):
        print "Specified config file does not exist: " + config
        sys.exit(2)

    # Check valid json
    if not MyJson.validateFile(config):
        print "Invalid JSON data in: " + config
        sys.exit(2)


    # 4. Read JSON config into dict()
    jdict = MyJson.convertFile2dict(config)


    # 5. Set up base
    base_url = jdict['proto'] + '://' + jdict['domain']
    login_url = base_url + jdict['login']['action']

    post_data = []
    for key,val in jdict['login']['fields'].iteritems():
        post_data.append(key + '=' + val)


    # Cookie/Output files
    file_output = output if output else '/tmp/login.html'
    file_cookie = cookie if cookie else '/tmp/cookie.txt'


    # Ask what to do if file exists and not '--yes' was specified
    if os.path.isfile(file_output) and not yes:
        answer = None
        while answer != 'y' and answer != 'Y':
            answer = raw_input('Output file already exists. Overwrite? [y/n]? ')

            if answer == 'Y' or answer == 'y' or answer == 'Yes' or answer == 'yes':
                break
            elif answer == 'N' or answer == 'n':
                print "aborting..."
                sys.exit(0)

    # Ask what to do if file exists and not '--yes' was specified
    if os.path.isfile(file_cookie) and not yes:
        answer = None
        while answer != 'y' and answer != 'Y':
            answer = raw_input('Cookie file already exists. Overwrite? [y/n]? ')

            if answer == 'Y' or answer == 'y' or answer == 'Yes' or answer == 'yes':
                break
            elif answer == 'N' or answer == 'n':
                print "aborting..."
                sys.exit(0)



    wget_create_session = [
        'wget',
        '--quiet',
        '--keep-session-cookies',
        '--save-cookies',
        file_cookie,
        '-O',
        '-',
        login_url
    ]



    # Initial wget
    if verbose:
        print MyShell().color('blue') + '[1] Creating initial session request' + MyShell().color('reset')

    output = ['']
    MyShell().run(wget_create_session, output, show_cmd=verbose, show_return=True)

    if jdict['login']['csrf']['enabled']:
        if verbose:
           print MyShell().color('blue') + '[2] Extracting CSRF key' + MyShell().color('reset')

        csrf_key = jdict['login']['csrf']['field']
        # Prepare regex
        re1 = "name=(\"|')%s(\"|').*value=(\"|')(.*)(\"|')" % (csrf_key)
        re2 = "value=(\"|')(.*)(\"|').*name=(\"|')%s(\"|')" % (csrf_key)
        # Search
        r1 = re.search(re1, output[0])
        r2 = re.search(re2, output[0])

        if r1:
            csrf_val = r1.group(4)
        elif r2:
            csrf_val = r2.group(2)
        else:
            print "Error, no such html attribute found"
            csrf_val = ''

        # Show extracted key
        if verbose:
            print "key: %s | val: %s" % (csrf_key, csrf_val)

        post_data.append(csrf_key + '=' + csrf_val)
    else:
        print MyShell().color('blue') + '[2] No CSRF key extraction' + MyShell().color('reset')


    wget_login = [
        'wget',
        '--quiet',
        '--content-on-error',
        '--keep-session-cookies',
        '--load-cookies',
        file_cookie,
        '--save-cookies',
        file_cookie,
        '--post-data',
        '&'.join(post_data),
        '-O',
        file_output,
        login_url
    ]

    # Login wget
    if verbose:
        print MyShell().color('blue') + '[3] Submitting POST login' + MyShell().color('reset')
    MyShell().run(wget_login, output, show_cmd=verbose, show_return=True)

    # Inspect source code
    if verbose:
        print MyShell().color('blue') + '[4] Evaluating login page source' + MyShell().color('reset')
    source = MyFile.read(file_output)


    retval = 0
    if jdict['login']['failure'] in source:
        print "[FAILED] Login failed"
        retval = 2
    elif os.path.getsize(file_output) > 0:
        print "[OK] Login successful"
        retval = 0
    else:
        print "[FAILED] Result page has 0 Bytes"
        retval = 2


    if cookie:
        print "[OK] Session cookie created: " + file_cookie
    else:
        os.unlink(file_cookie)

    if output:
        print "[OK] Output file saved: " + file_output
    else:
        os.unlink(file_output)

    sys.exit(retval)

