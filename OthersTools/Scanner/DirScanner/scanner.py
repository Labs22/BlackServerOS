#
# mail me ajithkp560 [at] gmail.com
#
# Greets to TOF members Coded32, null void, X-Hund, Evis, John, Alex and all Unkown members
# Love to AMSTECK COLLEGE, Kalliasseri : Dheeraj, Ashwin, Jhelai, Arjun, Vipin, Jitendra
# Special Thanqs to Vishnu_Nath [W3P]
#
# Thanks: WEBSPLOIT FRAME WORK: for provide large list of Directory List
#
# For Exit use Ctrl+C
#
import time
import urllib2
from urllib2 import Request, urlopen, URLError
import socket
import sys
import httplib
from urlparse import urlparse

tot=0
found=0

BLUE = '\033[94m'
RED = '\033[91m'
GREEN = '\033[32m'


timeout = 10
socket.setdefaulttimeout(timeout)
print "\n"
try:
    print GREEN+"\t################################################################"
    print "\t# http://TerminalCoders.BlogSpot.iN      OpenFire-Security.Net #"
    print "\t#       ###############      ########       ############       #"
    print "\t#       #             #     ##      ##      #          #       #"
    print "\t#       ######   ######     ##      ##      #   ########       #"
    print "\t#            #   #          ##      ##      #   #              #"
    print "\t#            #   #          ##      ##      #   #####          #"
    print "\t#            #   #          ##      ##      #   #####          #"
    print "\t#            #   #          ##      ##      #   #              #"
    print "\t#            #   #          ##      ##      #   #              #"
    print "\t#            #####    [#]    ########   [#] #####              #"
    print "\t#                                                              #"
    print "\t# coded by Ajith KP                        DirDictionaryBruter #"
    print "\t#                          Greets to Coded32 and T.O.F members #"
    print "\t################################################################"
    var1=0
    var2=0
    site = raw_input(RED+"\nURL"+BLUE+"[http/https]"+RED+"> "+GREEN)
    parse=urlparse(site)
    name=parse.netloc
    l_file="logs/"+name+".log"
    try:
        try:
            create=open(l_file, "w")
        except(IOError):
            print(GREEN+"Failed to create file: "+name+".log")
        print (GREEN+"\n\tChecking website " + site + "...")
        conn = urllib2.Request(site)
        urllib2.urlopen(site)
        print RED+"\t[V]"+GREEN+" Yes... Server is Online."+GREEN
    except (urllib2.HTTPError) as Exit:
        print(GREEN+"\t[!] Oops Error occured")
        raw_input(GREEN+"\t[!] Possible problems: Server offline, Invalid URL,Internet offline")      
        exit()
    try:
        dirlist = open("files/directory", "r")
        adm_php = open("files/admin/php", "r")
        adm_asp = open("files/admin/asp", "r")
        adm_cgi = open("files/admin/cgi", "r")
        adm_jsp = open("files/admin/jsp", "r")
        adm_brf= open("files/admin/brf", "r")
        adm_cfm = open("files/admin/cfm", "r")
        php_my_adm = open("files/phpMyAdmin", "r")
        shells = open("files/shells", "r")
    except(IOError):
        print GREEN+"Some File Not Found!"
        exit()
    print(RED+"\n\nSelect Option:\n\n"+GREEN+"\t1 Directory Scan\n\t2 Admin Page Scan\n\t3 phpMyAdmin Scan\n\t4 Shell Scan")
    option=input(GREEN+"\n   >> "+RED)
    if option==1:
        dirs = dirlist.readlines()
    if option==2:
        print(RED+"\nSELECT FILE TYPE:\n\n"+GREEN+"\t1 PHP\n\t2 ASP\n\t3 JSP\n\t4 CGI\n\t5 CFM\n\t6 BRF")
        adm_opt=input(GREEN+"\n   >> "+RED)
        if adm_opt==1:
            dirs = adm_php.readlines()
        if adm_opt==2:
            dirs = adm_asp.readlines()
        if adm_opt==3:
            dirs = adm_jsp.readlines()
        if adm_opt==4:
            dirs = adm_cgi.readlines()
        if adm_opt==5:
            dirs = adm_cfm.readlines()
        if adm_opt==6:
            dirs = adm_brf.readlines()
    if option==3:
        dirs = php_my_adm.readlines()
    if option==4:
        dirs = shells.readlines()

    print(GREEN+"\n\n\t[+]"+BLUE+" Scanning " +RED+ site + "\n")
    for dirz in dirs:
        dirz=dirz.replace("\n","")
        dirz="/"+dirz
        url_z=site+dirz
        print(RED+"--> "+GREEN+"Checking "+ BLUE + url_z)
        req=Request(url_z)
        time.sleep(2)
        try:
            response = urlopen(req)
        except URLError, e:
            if hasattr(e, 'reason'):
                print(RED+"\t[x] "+GREEN+"Not Found")
                tot=tot+1
            elif hasattr(e, 'code'):
                print(RED+"\t[x] "+GREEN+"Not Found")
                tot=tot+1
        else:
            try:
                logs=open(l_file, "a+")
            except(IOError):
                print RED+"\t[x] Failed to create DirLogs.log"
            found_url=url_z
            print(RED+"\t>>>"+GREEN+" Found "+RED+found_url)
            logs.writelines(found_url+"\n")
            found=found+1
            tot=tot+1
            logs.close()
    
    print BLUE+"\t\nTotal scanned:",tot
    print GREEN+"\tFound:",found
    print RED+"\nFounded Logs are saved in %s.log, Read it" %(name)
    print GREEN+"\n"
except (httplib.HTTPResponse, urllib2.HTTPError, socket.error):
    print "\n\t[!] Session Cancelled; Error occured. Check internet settings"
    print RED+"\n\t[!] Session cancelled"
    print BLUE+"\t\nTotal scanned:",tot
    print GREEN+"\tFound:",found
    print RED+"\nFounded Logs are saved in"+GREEN+" %s"+RED+", Read it" %(l_file)
    print GREEN+"\n"
except (KeyboardInterrupt, SystemExit):
    print RED+"\n\t[!] Session cancelled"
    print BLUE+"\t\nTotal scanned:",RED,tot
    print GREEN+"\tFound:",RED,found
    print GREEN+"\nFounded Logs are saved in \""+RED+"/%s"%(l_file)+GREEN+"\" , Read it" 
    print GREEN+"\n"
