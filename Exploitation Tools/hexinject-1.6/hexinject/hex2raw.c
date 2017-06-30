/*
 * hex2raw.c
 *
 *  Created on: 03/dec/2012
 *      Author: Acri Emanuele <crossbower@tuta.io>
 *  
 *  Convert hexstrings to raw data (and vice versa).
 *  Uses stdin and stdout.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hexstring.h"

#define VERSION "1.6"

#define BUFFER_SIZE 8192

/**
 * Convert standard input in hexstring format to raw format
 */
int convert_hexstr_to_raw_loop()
{
    char buffer[BUFFER_SIZE];
    
    while (!feof(stdin)) {
    
        /* Read hexstring */
        if( !fgets(buffer, BUFFER_SIZE, stdin) ) {
            continue;
        }

        /* Write raw output */
    	int size = 0;
    	uint8_t *raw = NULL;

    	raw = hexstr_to_raw(buffer, &size);

    	fwrite(raw, 1, size, stdout);
        fflush(stdout);

    	free(raw);
    }

    return 0;
}

/**
 * Convert standard input in raw format to hexstring format
 */
int convert_raw_to_hexstr_loop()
{
    uint8_t buffer[BUFFER_SIZE];
    int size;
    
    while (!feof(stdin)) {
        
        /* Read raw */
        size = fread(buffer, 1, BUFFER_SIZE, stdin);

        /* Write hexstring output */
        char *hexstr = raw_to_hexstr(buffer, size);
        
        fprintf(stdout, "%s\n", hexstr);
        fflush(stdout);

        free(hexstr);
    }

    return 0;
}

/**
 * Usage
 */
void usage(char *progname) {
    printf("Hex2Raw " VERSION " [convert hexstrings on stdin to raw data on stdout]\n"
           "written by: Emanuele Acri <crossbower@tuta.io>\n\n"
           "Usage:\n\t%s [-r|-h]\n\n"
           "Options:\n"
           "\t-r\treverse mode (raw to hexstring)\n"
           "\t-h\tthis help screen\n", progname);
    exit(0);
}

/*
 * Main function
 */
int main(int argc, char **argv) {

    int reverse = 0;

    // check arguments
    if(argc > 1) {

        // raw to hexstring
        if(!strcmp(argv[1], "-r")) {
            reverse = 1;
        }

        // unknown options
        else {
            usage(argv[0]);
        } 
    }
    
    // hexstring to raw data
    if(reverse == 0) {
        convert_hexstr_to_raw_loop();
    }

    // raw data to hexstring
    else {
        convert_raw_to_hexstr_loop();
    }

    return 0;
}
