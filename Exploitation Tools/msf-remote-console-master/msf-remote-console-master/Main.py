#!/usr/bin/env python

try:
    import time
    import sys
    import threading
    from optparse import OptionParser
    from ReadInputThread import ReadInputThread
except ImportError as msg:
    print "[-] Library not installed: " + str(msg)
    print "[*] Try installing it with: pip install " + str(msg.message)
    sys.exit()

try:
    try:
      import readline
    except ImportError:
      import pyreadline as readline
except ImportError:
    print "[-] Readline module is not installed!"
    print "[*] Install on Linux with: pip install readline"
    print "[*] Install on Windows with: pip install pyreadline"
    sys.exit()

try:
    from MsfConsole import MsfConsole
    from metasploit.msfrpc import MsfRpcError
except ImportError as msg:
    print "[-] Missing library pymetasploit"
    print "[*] Please clone from \"git clone https://github.com/allfro/pymetasploit.git pymetasploit\""
    print "[*] \"cd pymetasploit && sudo python setup.py install\""
    sys.exit()

#
class Main:
    # Hardcoded credentials
    username = "msf"
    password = "msf"
    port = 55553
    host = "127.0.0.1"
    ssl = True

    # Variables
    msfconsole = None

    def __init__(self):
        ###
        # Command line argument parser
        ###
        parser = OptionParser()
        parser.add_option("-r", "--resource", action="store", type="string", dest="resource", help="Path to resource file")
        parser.add_option("-u", "--user", action="store", type="string", dest="username", help="Username specified on msfrpcd")
        parser.add_option("-p", "--pass", action="store", type="string", dest="password", help="Password specified on msfrpcd")
        parser.add_option("-s", "--ssl", action="store_true", dest="ssl", help="Enable ssl")
        parser.add_option("-P", "--port", action="store", type="string", dest="port", help="Port to connect to")
        parser.add_option("-H", "--host", action="store", type="string", dest="host", help="Server ip")
        parser.add_option("-c", "--credentials", action="store_true", dest="credentials", help="Use hardcoded credentials")
        parser.add_option("-e", "--exit", action="store_true", dest="exit", help="Exit after executing resource script")
        (options, args) = parser.parse_args()

        if len(sys.argv) is not 1 and options.credentials is None:
            if options.username is not None:
                self.username = options.username
            else:
                print "[*] Use default: username => msf"
                self.username = "msf"

            if options.password is not None:
                self.password = options.password
            else:
                print "[*] Use default: password => msf"
                self.password = "msf"

            if options.ssl is True:
                self.ssl = True
            else:
                print "[*] Use default: ssl => False"
                self.ssl = False

            if options.port is not None:
                self.port = options.port
            else:
                print "[*] Use default: port => 55553"
                self.port = 55553

            if options.host is not None:
                self.host = options.host
            else:
                print "[*] Use default: host => 127.0.0.1"
                self.host = "127.0.0.1"
        else:
            if self.host and self.port and self.password and self.ssl and self.username is None:
                print "[-] You have to specify all hardcoded credentials"
                sys.exit()
            print "[*] Using hardcoded credentials!"

        # Objects
        self.msfconsole = MsfConsole(self.username, self.password, self.port, self.host, self.ssl)

        # Connect to msfrpcd
        if self.msfconsole.connect() is False:
            sys.exit()

        # If -r flag is given
        if options.resource is not None:
            self.msfconsole.load_resource(options.resource)
            time.sleep(3)

            if options.exit is True:
                self.msfconsole.disconnect()
                sys.exit()

        # Add directory auto completion
        readline.parse_and_bind("tab: complete")

        # Go to main menu
        self.exec_menu('main_menu')

    # Executes menu function
    def exec_menu(self, choice):
        # If empty input immediately go back to main menu
        if choice == '':
            self.menu_actions['main_menu'](self)
        else:
            # Execute selected function out of dictionary
            try:
                self.menu_actions[choice](self)
            # If given input isn't in dictionary
            except KeyError:
                print '[-] Invalid selection, please try again.'
                time.sleep(1)
                self.menu_actions['main_menu'](self)

    # Main Menu
    def main_menu(self):
        try:
            # Create read thread
            readThread = ReadInputThread(self.msfconsole.get_path())
            readThread.start()

            try:
                while True:
                    # Get command user types in
                    command = readThread.get_command()

                    # If command is not empty break out of loop
                    if command:
                        break

                    # Run in background and read possible output from msfrpcd
                    if self.msfconsole.read_output():
                        # Found data to read
                        readThread.set_path(self.msfconsole.get_path())
            except ValueError:
                pass

            if command == "quit":
                self.msfconsole.disconnect()
                sys.exit()
            # If command not empty send it to msfrpcd
            if command:
                self.msfconsole.exec_command(command)

            # Go to this menu again
            self.exec_menu('main_menu')

        # Connection is only valid for 5 minutes afterwards request another one
        except MsfRpcError:
            print "[*] API token expired requesting new one..."
            if self.msfconsole.connect() is False:
                sys.exit()
            self.exec_menu('main_menu')
        except KeyboardInterrupt:
            self.msfconsole.disconnect()
            sys.exit()

    # Dictionary of menu entries
    menu_actions = {
        'main_menu': main_menu
    }

# Execute main
try:
    Main()
except KeyboardInterrupt:
    print "[*] Interrpted execution"
    exit(0)

except AttributeError:
    print "[-] You have to be connected to the server " + str(msg)
    exit(1)