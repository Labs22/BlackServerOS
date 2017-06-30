#ifndef __ARGPARSER_H__
#define __ARGPARSER_H__

#include <stdio.h>
#include <string.h>

#include <getopt.h>

#include "hexinject.h"

#define VERSION "1.6"

/*
 * Cmdline options
 */
struct {
    int inject;                    // inject mode
    int sniff;                     // sniff mode
    int raw;                       // raw mode
    char *filter;                  // custom pcap filter
    char *device;                  // interface
    char *pcap_file;                // pcap file
    int count;                     // number of packets to capture
    int count_on;                  // enable count
    int sleep_time;                // sleep time in microseconds
    int list_devices;              // list all available network devices
    int no_cksum;                  // disable packet checksum
    int no_size;                   // disable packet size
    int promisc;                   // promiscuous mode
    int monitor;                   // enable monitor mode
} options;

/*
 * Program usage template
 */
const char usage_template[] =
    "HexInject " VERSION " [hexadecimal packet injector/sniffer]\n"
    "written by: Emanuele Acri <crossbower@tuta.io>\n\n"
    "Usage:\n"
    "   hexinject <mode> <options>\n"
    "\nOptions:\n"
    "  -s sniff mode\n"
    "  -p inject mode\n"
    "  -r raw mode (instead of the default hexadecimal mode)\n"
    "  -f <filter> custom pcap filter\n"
    "  -i <device> network device to use\n"
    "  -F <file> pcap file to use as device (sniff mode only)\n"
    "  -c <count> number of packets to capture\n"
    "  -t <time> sleep time in microseconds (default 100)\n"
    "  -I list all available network devices\n"
    "\nInjection options:\n"
    "  -C disable automatic packet checksum\n"
    "  -S disable automatic packet size fields\n"
    "\nInterface options:\n"
    "  -P disable promiscuous mode\n"
    "  -M put the wireless interface in monitor mode\n"
    "     (experimental: use airmon-ng instead of this...)\n"
    "\nOther options:\n"
    "  -h help screen\n";

/*
 * Program usage
 */
void usage(FILE *out, const char *error)
{
    fputs(usage_template, out);

    if(error)
        fputs(error, out);

    exit(1);
}

/*
 * Parser for command line options
 * See getopt(3)...
 */
int parseopt(int argc, char **argv)
{
    char ch;
    
    // cleaning
    memset(&options, 0, sizeof(options));
    
    // default options
    options.sleep_time = 100;
    options.promisc    = 1;
    
    const char *shortopt = "sprf:i:F:c:t:ICSPMh"; // short options
    
    while ((ch = getopt (argc, argv, shortopt)) != -1) {
        switch (ch) {
        
            case 's': // sniff mode
                options.sniff = 1;
                break;
            
            case 'p': // inject mode
                options.inject = 1;
                break;
                
            case 'r': // raw mode
                options.raw = 1;
                break;  
            
            case 'f': // custom filter
                options.filter = optarg;
                break;

            case 'i': // interface
                options.device = optarg;
                break;

            case 'F': // pcap file
                options.pcap_file = optarg;
                break;           
                
            case 'c': // packet count
                options.count = atoi(optarg);
                options.count_on = 1;
                break;

            case 't': // sleep time in microseconds
                options.sleep_time = atoi(optarg);
                break;

            case 'I': // list devices
                options.list_devices = 1;
                break;

            case 'C': // disable packet checksum
                options.no_cksum = 1;
                break;

            case 'S': // disable packet size
                options.no_size = 1;
                break;

            case 'P': // disable promiscuous mode
                options.promisc = 0;
                break;

            case 'M': // enable monitor mode
                options.monitor = 1;
                break;
 
            case '\377':
                /* Octal escape sequence character EOF of decimal value 255.
                   Needed on some systems to avoid the usage() screen. */
                goto GETOPT_END;

            case 'h': //help
                usage(stdout, NULL);

            case '?':
            default:
                usage(stderr, NULL);
        }
    }
GETOPT_END:
    
    // check mode
    if ( !options.inject && !options.sniff && !options.list_devices ) {
        usage(stderr, "\nError: no mode selected.\n");
    }

    if ( options.pcap_file && !options.sniff ) {
        usage(stderr, "\nError: using a pcap file is compatible only with sniff mode\n");
    }
    
    if ( options.inject && options.sniff ) {
        usage(stderr, "\nError: too many modes selected, see -s and -p options.\n");
    }
    
    return 1;
}

#endif /* __ARGPARSER_H__ */

