/**
 * HEXSTRING HEADER
 * 
 * Functions used to convert, and manipulate hexstrings.
 */

#ifndef HEXSTRING_H_
#define HEXSTRING_H_

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>

/**
 * Byte multiplier: a printed byte uses 3 characters
 */
#define BYTE_MULT 3

/**
 * Convert a 2-bytes string to an uint8_t integer (e.g. "1A\0" -> 0x1A)
 *
 * @param str hexstring to convert, of expected size BYTE_MULT
 * @return converted uint8_t integer
 */
uint8_t str_to_hex(char *str) {

     assert(str != NULL);

     uint8_t hex = 0;

     if(isdigit(str[0])) {
         hex |= (str[0] & 0x0F) << 4;
     } else {
         hex |= ((str[0]+0x09) & 0x0F) << 4;
     }

     if(isdigit(str[1])) {
         hex |= (str[1] & 0x0F);
     } else {
         hex |= ((str[1]+0x09) & 0x0F);
     }

     return hex;
}

/**
 * Convert an uint8_t integer to a 2-bytes hexstring (e.g. 0x1A -> "1A\0")
 *
 * @param hex uint8_t integer to convert
 * @param str destination hexstring, of expected size BYTE_MULT
 */
void hex_to_str(uint8_t hex, char *str) {

    assert(str != NULL);

    str[0] = (hex & 0xF0) >> 4;
    str[1] = (hex & 0x0F);
    str[2] = '\0';

    if(str[0]<0x0A) {
        str[0] |= 0x30;
    } else {
        str[0] |= 0x40;
        str[0] -= 0x09;
    }

    if(str[1]<0x0A) {
        str[1] |= 0x30;
    } else {
        str[1] |= 0x40;
        str[1] -= 0x09;
    }
}

/**
 * Check if the character is hexadecimal.
 *
 * @param c character to check
 * @return
 * - 1 if hexadecimal
 * - 0 if not hexadecimal
 */
int is_hex(char c) {
    if ((c >= 'A' && c <= 'F') || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
        return 1;
    return 0;
}

/**
 * Return the length of the hexstring.
 *
 * @param hexstr input string
 * @return hexstr length
 */
int hexstr_size(const char *hexstr) {

    assert(hexstr != NULL);
  
    int size = 0;
    const char *p = hexstr;

    /*
     * Parse an hexstring and calculate it's size.
     * Analyze three bytes at a time, in this format: HEX HEX (SPACE|NULL)
     * (Ex. "FF ", "2A ", "EE")
     */
    while ( is_hex(p[0]) && is_hex(p[1]) && (p[2] == ' ' || p[2] == '\n' || p[2] == '\0') ) {
        //printf("p[0] = %c, p[1] = %c, p[2] = %c\n", p[0], p[1], p[2] == '\0' ? 'N' : p[2]);

        size++;
	p += BYTE_MULT;
    }

    return size;
}

/**
 * Create a raw buffer from an hexstring. The buffer must be manually free()d.
 *
 * @param hexstr hexstring to convert
 * @param size hexstring size
 * @return pointer to the converted raw buffer
 */
uint8_t *hexstr_to_raw(const char *hexstr, int *size) {

    assert(hexstr != NULL);
    assert(size != NULL);

    uint8_t *raw = NULL;
    const char *p  = hexstr;

    *size = hexstr_size(hexstr);
    raw = (uint8_t *) malloc(*size); // malloc the raw buffer

    char hex[BYTE_MULT];
    int i = 0;

    /*
     * Parse an hexstring.
     * Analyze three bytes at a time, in this format: HEX HEX (SPACE|NULL)
     * (Ex. "FF ", "2A ", "EE")
     */
    while ( is_hex(p[0]) && is_hex(p[1]) && (p[2] == ' ' || p[2] == '\n' || p[2] == '\0') ) {
        //printf("p[0] = %c, p[1] = %c, p[2] = %c\n", p[0], p[1], p[2] == '\0' ? 'N' : p[2]);

        // extract a single byte
        hex[0] = p[0];
	hex[1] = p[1];
	hex[2] = '\0';

	raw[i] = str_to_hex(hex);
		
	i++;
	p += BYTE_MULT;
    }

    //hex_dump(raw, size);

    return raw;
}

/**
 * Create an hexstring from a raw buffer. The hexstring must be manually free()d.
 *
 * @param raw raw buffer to convert
 * @param size raw buffer size
 * @return converted hexstring
 */
char *raw_to_hexstr(const uint8_t *raw, int size) {

    assert(raw != NULL);
    if(size == 0) return NULL;

    char *hexstr = NULL;
    
    hexstr = (char *) malloc(size*BYTE_MULT); // malloc the hexstring
    
    char hex[BYTE_MULT];
    int i = 0;
    char *p = hexstr;

    /*
     * Parse an hexstring.
     * Analyze three bytes at a time, in this format: HEX HEX (SPACE|NULL)
     * (Ex. "FF ", "2A ", "EE")
     */
    for (i=0; i<size; i++, p+=BYTE_MULT) {

        // extract a single byte
        hex_to_str(raw[i], hex);

        p[0] = hex[0];
        p[1] = hex[1];
        p[2] = ' ';
    }


    hexstr[(size*BYTE_MULT)-1] = '\0';
    return hexstr;
}

#endif /* HEXSTRING_H_ */

