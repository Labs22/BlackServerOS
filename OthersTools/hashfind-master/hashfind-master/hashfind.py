#!/usr/bin/env python

"""
  hashfind

    author: Dimitri Fousekis (@rurapenthe0)
    author: @m3g9tr0n

    Licensed under the GNU General Public License Version 2 (GNU GPL v2),
        available at: http://www.gnu.org/licenses/gpl-2.0.txt

    (C) 2015 Dimitri Fousekis (@rurapenthe0) and @m3g9tr0n

    TODO:
    Please send a tweet to @rurapenthe0 or @m3g9tr0n with any suggestions.
    
"""

#import our required libraries
import argparse
import re
import sys
import os
import fileinput

#initialize the command-line parser
parser = argparse.ArgumentParser(prog='hashfind',description='Hashfind Multiple Password Hash Extractor v0.1 Copyright (C) 2015 Dimitri Fousekis (@rurapenthe0), @m3g9tr0n',formatter_class=lambda prog: argparse.HelpFormatter(prog,max_help_position=27),epilog='''
                                    Hashfind is a program that can extract password hashes, emails or credit-card numbers out of files.
                                    This program is provided as-is and without guarantee.
                                    Twitter: @rurapenthe0 ''')
parser.add_argument('-m','--mode', dest='hashoption',metavar='<1-37>',help="1: MD5/MD4/MD2/NTLM, 2: mySQL (old)/LM, 3: Joomla, 4: VBulletin, 5: phpBB3-MD5, 6: Wordpress-MD5, 7: Drupal 7, 8: Unix-md5 (old), 9: SHA512Crypt, 10: E-Mails, 11: Credit Cards, 12: SHA256, 13: SHA384, 14: SHA512, 15: SHA1, 16: Blowfish, 17:DES, 18:MD5-apr, 19:MD5-Sun, 20:sha256(Unix), 21:AIX-MD5, 22:AIX-SSHA256, 23:AIX-SSHA512, 24:AIX-SSHA1, 25:OS X v10.8 / v10.9, 26:IPMI2 RAKP HMAC-SHA1, 27:Sybase ASE, 28:Cisco-8, 29:Cisco-9, 30:Django (PBKDF2-SHA256), 31:nsldap, SHA-1(Base64), Netscape LDAP SHA, 32:Django (SHA-1)6, 33:MSSQL(2005), 34:MSSQL(2000), 35:SSHA-512(Base64), 36:LDAP {SSHA512}, 37:PHPS, 38:Mediawiki B type" )
parser.add_argument('-i','--input', dest='filename',metavar='<filename>',help="The file to search for matches.")
parser.add_argument('-o','--output', dest='outfilename',metavar='<filename>', help="The output file to right results to.")
parser.add_argument('-n','--nowrite', dest="quietmode",action="store_true", help="Output to STDOUT instead of a file and suppress all info messages.")
parser.add_argument('-s','--stdin', dest="stdinmode",action="store_true", help="Read from STDIN.")
parser.set_defaults(q=False)
args = parser.parse_args()
process_as_dir=False
process_as_stdin=False


#check the user supplied inputs
hashoption_integer = int(args.hashoption)

if args.stdinmode and args.filename:
                print "[-] Error, cannot read from STDIN and file. Choose one."
                sys.exit(-1)

if (not args.filename) and (not args.stdinmode):
                print "[-] No inout file/dir specified. Use -i or -s."
                sys.exit(-1)

if (args.quietmode) and (args.outfilename):
                print "[-] Cannot combine STDOUT mode and output write to file. Choose one."
                sys.exit(-1)

if (hashoption_integer > 0 ) and (hashoption_integer < 38):
                                            if not args.quietmode: print "[+] Hash option ok."
else:
        print "[-] Invalid option for hash type -m."
        sys.exit(-1)

if (not args.stdinmode) and (os.path.isdir(args.filename)):
                                process_as_dir = True
                                if not args.quietmode: print "[+] Directory detected, recursive mode is on."
if not process_as_dir:
    if (not args.stdinmode) and (os.path.isfile(args.filename)):
                                process_as_dir = False
                                if not args.quietmode: print "[+] File detected, recursive mode is off."
    else:
        if not args.stdinmode:
            print "[-] Unknown file/dir specified with -i."
            sys.exit(-1)
        else: process_as_stdin = True

if not args.quietmode:
        if not args.outfilename:
                                print "[-] No output file specified. Use -o or -n for stdout."
                                sys.exit(-1)
        try:
            outputhandler = open(args.outfilename,'a')
            print "[+] Output file opened, saving to "+args.outfilename+"."
        except:
                print "[-] Error, could not open output file for writing."
                sys.exit(-1)

#begin engine definitions here

def r_md5(mydata):
            results = re.search(r'(^|[^a-fA-F0-9])[a-fA-F0-9]{32}([^a-fA-F0-9]|\$)',mydata,re.M|re.I)
            if results:
                results2 = re.search(r'[a-fA-F0-9]{32}',results.group())
                if args.quietmode: print results2.group()
                else: outputhandler.write(str(results2.group().rstrip('\n'))+'\r\n')

def r_mysqlold(mydata):
            results = re.search(r'[0-7][0-9a-f]{7}[0-7][0-9a-f]{7}',mydata,re.M|re.I)
            if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')


def r_joomla(mydata):
    results = re.search(r'([0-9a-zA-Z]{32}):(\w{32})',mydata)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_vbulletin(mydata):
    results = re.search(r'([0-9a-zA-Z]{32}):(\S{3,32})',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_phpbb3(mydata):
    results = re.search(r'\$H\$\S{31}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_wordpressmd5(mydata):
    results = re.search('\$P\$\S{31}',mydata,re.M)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_drupal(mydata):
    results = re.search(r'\$S\$\S{52}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_unixmd5old(mydata):
    results = re.search(r'\$1\$\w{8}\S{22}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_sha512crypt(mydata):
        results = re.search(r'\$6\$\w{8}\S{86}',mydata,re.M|re.I)
        if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_email(mydata):
    results = re.search('[a-zA-Z0-9.#?$*_-]+@[a-zA-Z0-9.#?$*_-]+',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_cc(mydata):
    #Visa
    results = re.search(r'4[0-9]{3}[ -]?[0-9]{4}[ -]?[0-9]{4}[ -]?[0-9]{4}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

    #Mastercard
    results = re.search(r'5[0-9]{3}[ -]?[0-9]{4}[ -]?[0-9]{4}[ -]?[0-9]{4}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

    #American Express
    results = re.search(r'\b3[47][0-9]{13}\b',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

    #Diners
    results = re.search(r'\b3(?:0[0-5]|[68][0-9])[0-9]{11}\b',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

    #JCB
    results = re.search(r'\b(?:2131|1800|35\d{3})\d{11}\b',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')
    #Discover
    results = re.search(r'6011[ -]?[0-9]{4}[ -]?[0-9]{4}[ -]?[0-9]{4}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')
     
    # AMEX
    results = re.search(r'3[47][0-9]{2}[ -]?[0-9]{6}[ -]?[0-9]{5}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_sha1(mydata):
      results = re.search(r'(^|[^a-fA-F0-9])[a-fA-F0-9]{40}([^a-fA-F0-9]|$)',mydata,re.M|re.I)
      if results:
                results2 = re.search(r'[a-fA-F0-9]{40}',results.group())
                if args.quietmode: print results2.group()
                else: outputhandler.write(str(results2.group().rstrip('\n'))+'\r\n')

def r_sha512(mydata):
      results = re.search(r'(^|[^a-fA-F0-9])[a-fA-F0-9]{128}([^a-fA-F0-9]|$)',mydata,re.M|re.I)
      if results:
                results2 = re.search(r'[a-fA-F0-9]{128}',results.group())
                if args.quietmode: print results2.group()
                else: outputhandler.write(str(results2.group().rstrip('\n'))+'\r\n')

def r_sha256(mydata):
      results = re.search(r'(^|[^a-fA-F0-9])[a-fA-F0-9]{64}([^a-fA-F0-9]|$)',mydata,re.M|re.I)
      if results:
                results2 = re.search(r'[a-fA-F0-9]{64}',results.group())
                if args.quietmode: print results2.group()
                else: outputhandler.write(str(results2.group().rstrip('\n'))+'\r\n')

def r_sha384(mydata):
      results = re.search(r'(^|[^a-fA-F0-9])[a-fA-F0-9]{96}([^a-fA-F0-9]|$)',mydata,re.M|re.I)
      if results:
                results2 = re.search(r'[a-fA-F0-9]{96}',results.group())
                if args.quietmode: print results2.group()
                else: outputhandler.write(str(results2.group().rstrip('\n'))+'\r\n')

def r_blowfish(mydata):
    results = re.search(r'\$2a\$10\$\S{53}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_des(mydata):
    results = re.search(r'\S{13}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_md5apr(mydata):
    results = re.search(r'\$apr1\$\w{8}\S{22}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_md5sun(mydata):
    results = re.search(r'\$md5\$rounds\=904\$\w{16}\S{23}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_sha256unix(mydata):
    results = re.search(r'\$5\$\w{8}\$\S{43}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_aixmd5(mydata):
    results = re.search(r'\{smd5\}\S{8}\$\S{22}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_aixssha256(mydata):
    results = re.search(r'\{ssha256\}06\$\S{16}\$\S{43}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_aixssha512(mydata):
    results = re.search(r'\{ssha512\}06\$\S{16}\$\S{86}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')
                
def r_aixssha1(mydata):
    results = re.search(r'\{ssha1\}06\$\S{16}\$\S{27}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n') 

# OS X v10.8 / v10.9
def r_osx(mydata):
    results = re.search(r'\$ml\$\w{5}\$\w{64}\$\w{128}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_hmacsha1(mydata):
    results = re.search(r'([0-9a-fA-F]{130}):(\w{40})',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_sybase(mydata):
	results = re.search(r'(0x\w{84})',mydata,re.M|re.I)
	if results:
				if args.quietmode: print results.group()
				else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_cisco8(mydata):
    results = re.search(r'\$8\$\S{14}\$\S{43}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')
                
def r_cisco9(mydata):
    results = re.search(r'\$9\$\S{14}\$\S{43}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')
                
def r_djangosha256(mydata):
    results = re.search(r'pbkdf2_sha256\$20000\$\S{57}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_nsldap(mydata):
    results = re.search(r'\{SHA\}\S{28}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')
    results = re.search(r'\{SSHA\}\S{40}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_djangosha1(mydata):
    results = re.search(r'sha1\$\w{5}\$\w{40}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_mssql2005(mydata):
	results = re.search(r'(0x\w{52})',mydata,re.M|re.I)
	if results:
				if args.quietmode: print results.group()
				else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_mssql2000(mydata):
	results = re.search(r'(0x\w{92})',mydata,re.M|re.I)
	if results:
				if args.quietmode: print results.group()
				else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')
				
def r_ldapssha512(mydata):
    results = re.search(r'\{SSHA512\}\S{96}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

def r_phps(mydata):
    results = re.search(r'\$PHPS\$\w{14}\$\w{32}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')
                
def r_mediawiki(mydata):
    results = re.search(r'\$B\$\w{8}\$\w{32}',mydata,re.M|re.I)
    if results:
                if args.quietmode: print results.group()
                else: outputhandler.write(str(results.group().rstrip('\n'))+'\r\n')

#primary option storage and mappings to engines

processor = {1: r_md5,
             2: r_mysqlold,
             3: r_joomla,
             4: r_vbulletin,
             5: r_phpbb3,
             6: r_wordpressmd5,
             7: r_drupal,
             8: r_unixmd5old,
             9: r_sha512crypt,
             10: r_email,
             11: r_cc,
             12: r_sha256,
             13: r_sha384,
             14: r_sha512,
             15: r_sha1,
             16: r_blowfish,
	         17: r_des,
	         18: r_md5apr,
	         19: r_md5sun,
	         20: r_sha256unix,
	         21: r_aixmd5,
			 22: r_aixssha256,
			 23: r_aixssha512,
			 24: r_aixssha1,
			 25: r_osx,
			 26: r_hmacsha1,
			 27: r_sybase,
			 28: r_cisco8,
			 29: r_cisco9,
			 30: r_djangosha256,
			 31: r_nsldap,
			 32: r_djangosha1,
			 33: r_mssql2005,
			 34: r_mssql2000,
			 35: r_ldapssha512,
			 36: r_phps,
			 37: r_mediawiki}


#main program loop

def main():
   if (process_as_dir) and (not process_as_stdin):
     for subdir, dirs, files in os.walk(args.filename):
            for file in files:
                thefile = os.path.join(subdir, file)
                for mydata in fileinput.input(thefile):
                    processor[int(args.hashoption)](mydata)
     if args.quietmode: sys.exit()
     else:
                print "[+] Done dir process."
                outputhandler.close()

   elif (not process_as_dir) and (not process_as_stdin):
        fileinput.close()
        thefile = fileinput.input(args.filename)
        for mydata in thefile:
             processor[int(args.hashoption)](mydata.rstrip())

        if args.quietmode: sys.exit()
        else:
                print "[+] Done file process."
                outputhandler.close()
   elif process_as_stdin:
        if not args.quietmode: print "[+] Reading from stdin.."
        for mydata in sys.stdin:
            processor[int(args.hashoption)](mydata)
        if args.quietmode: sys.exit()
        else:
                print "[+] Done stdin process."
                outputhandler.close()



if __name__ == "__main__":
    main()


