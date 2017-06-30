/*
 * hexinject.c
 *
 *  Created on: 11/mag/2010
 *      Author: Acri Emanuele
 */

#include "hexinject.h"

/**
 * Injection loop using hexstring format
 */
int inject_hexstr_loop(pcap_t *fp)
{
    char buffer[BUFFER_SIZE];

    while (!feof(stdin)) {

        /* Read hexstring */
        if( !fgets(buffer, BUFFER_SIZE, stdin) ) {
            continue;
        }

        /* Send down the packet */
        if (inject_hexstr(fp, buffer, options.no_cksum, options.no_size) != 0) {
            fprintf(stderr,"\nError sending the packet: %s\n", pcap_geterr(fp));
            return 1;
        }
    }

    return 0;
}

/*
 * Injection loop using raw format
 */
int inject_raw_loop(pcap_t *fp)
{
    uint8_t buffer[BUFFER_SIZE];
    size_t size;

    while (!feof(stdin)) {

        /* Read raw */
        size = fread(buffer, 1, BUFFER_SIZE, stdin);

        /* Send down the packet */
        if (inject_raw(fp, buffer, size, options.no_cksum, options.no_size) != 0) {
            fprintf(stderr,"\nError sending the packet: %s\n", pcap_geterr(fp));
            return 1;
        }
    }

    return 0;
}

/*
 * Sniffing loop with hexstring output
 */
int sniff_hexstr_loop(pcap_t *fp, int from_file)
{
    char *hexstr;
    
    while ( 1 ) {

        /* Count */
        if (options.count_on) {
            if (options.count <= 0) {
                break;
            }
        }

        /* Sniff and print a packet */
        if ((hexstr = sniff_hexstr(fp))) {
            printf("%s\n", hexstr);
            fflush(stdout);
            free(hexstr);

            /* Count */
            if (options.count_on) {
                options.count--;
            }
        } else if (from_file) {	 
	    /* File terminated */
	    return 0;
	}

        usleep(options.sleep_time);
    }

    return 0;
}

/*
 * Sniffing loop with raw output
 */
int sniff_raw_loop(pcap_t *fp, int from_file)
{
    const uint8_t *packet;
    size_t size;

    while ( 1 ) {

        /* Count */
        if (options.count_on) {
            if (options.count <= 0) {
                break;
            }
        }

        size = BUFFER_SIZE;

        /* Sniff and print a packet */
        if ((packet = sniff_raw(fp, &size))) {
            fwrite(packet, 1, size, stdout);
            fflush(stdout);

            /* Count */
            if (options.count_on) {
                options.count--;
            }
        } else if (from_file) {	 
	    /* File terminated */
	    return 0;
	}

        usleep(options.sleep_time);
    }

    return 0;
}

/*
 * Main function
 */
int main(int argc, char **argv) {

    pcap_t *fp;
    struct bpf_program bpf;
    char errbuf[PCAP_ERRBUF_SIZE];

    int ret_val;

    pcap_if_t *alldevsp;

    char *dev=NULL;
	
    /* Parse cmdline options */
    parseopt(argc, argv);

    /* in case of device listing */
    if(options.list_devices) {
	if(pcap_findalldevs(&alldevsp, errbuf) != 0) {
	    fprintf(stderr,"Unable to list devices: %s.\n", errbuf);
            return 1;
	}
	for (; alldevsp; alldevsp=alldevsp->next) {
	    printf("%s", alldevsp->name);
	    if (alldevsp->description != NULL)
		printf(" (%s)", alldevsp->description);
	    printf("\n");
	}
	return 0;
    }
    
    /* Network interface stuff */
    if(!options.pcap_file) {

	/* find a device if not specified */
	if(!options.device) {
	    dev = pcap_lookupdev(errbuf);
	    if (dev == NULL) {
		fprintf(stderr,"Unable to find a network adapter: %s.\n", errbuf);
		return 1;
	    }
	}
	else {
	    dev = options.device;
	}

	/* Create packet capture handle */
	if((fp = pcap_create(dev, errbuf)) == NULL) {
	    fprintf(stderr,"Unable to create pcap handle: %s\n", errbuf);
	    return 1;
	}

	/* Set snapshot length */
	if(pcap_set_snaplen(fp, BUFSIZ) != 0) {
	    fprintf(stderr,"Unable to set snapshot length: the interface may be already activated\n");
	    return 1;
	}

	/* Set promiscuous mode */
	if(pcap_set_promisc(fp, options.promisc) != 0) {
	    fprintf(stderr,"Unable to set promiscuous mode: the interface may be already activated\n");
	    return 1;
	}

	/* Set read timeout */
	if(pcap_set_timeout(fp, 1000) != 0) { // a little patch i've seen in freebsd ports: thank you guys ;)
	    fprintf(stderr,"Unable to set read timeout: the interface may be already activated\n");
	    return 1;
	}

	/* Set monitor mode */
	if(options.monitor) {
	    if(pcap_can_set_rfmon(fp)==0) {
		fprintf(stderr, "Monitor mode not supported by %s.\n", dev);
		return 1;
	    }

	    if((ret_val=pcap_set_rfmon(fp, 1)) != 0) {
		fprintf(stderr, "Unable to set monitor mode: the interface may be already activated.\n");
		return 1;
	    }
	}

	/* Activate interface */
	if(pcap_activate(fp) != 0) {
	    fprintf(stderr, "Unable to activate the interface: %s\n", pcap_geterr(fp));
	    return 1;
	}

    }

    /* PCAP File Stuff */
    else {

	/* Create packet capture handle */
	if((fp = pcap_open_offline(options.pcap_file, errbuf)) == NULL) {
	    fprintf(stderr,"Unable to open pcap file: %s\n", errbuf);
	    return 1;
	}

    }
    
    /* Apply filter */
    if ( options.filter ) {

        if(pcap_compile(fp, &bpf, options.filter, 0, 0) != 0) {
            fprintf(stderr, "Error compiling filter: %s\n", pcap_geterr(fp));
            return 1;
        }

        if(pcap_setfilter(fp, &bpf) != 0) {
            fprintf(stderr, "Error setting filter: %s\n", pcap_geterr(fp));
            return 1;
        }

    }

    /* Inject mode - Hexstring */
    if (options.inject && !options.raw) {
        ret_val = inject_hexstr_loop(fp);
    }
    
    /* Inject mode - Raw */
    else if (options.inject && options.raw) {
        ret_val = inject_raw_loop(fp);
    }
    
    /* Sniff mode - Hexstring */
    else if (options.sniff && !options.raw) {
        ret_val = sniff_hexstr_loop(fp, options.pcap_file ? 1 : 0);    
    }
    
    /* Sniff mode - Raw */
    else if (options.sniff && options.raw) {
        ret_val = sniff_raw_loop(fp, options.pcap_file ? 1 : 0);
    }

    pcap_close(fp);

    return ret_val;
}
