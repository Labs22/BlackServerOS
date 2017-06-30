/*
 *   
 *   Ophcrack is a Lanmanager/NTLM hash cracker based on the faster time-memory
 *   trade-off using rainbow tables. 
 *   
 *   Created with the help of: Maxime Mueller, Luca Wullschleger, Claude
 *   Hochreutiner, Andreas Huber and Etienne Dysli.
 *   
 *   Copyright (c) 2008 Philippe Oechslin, Cedric Tissieres, Bertrand Mesot
 *   
 *   Ophcrack is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *   
 *   Ophcrack is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Ophcrack; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *   
 *   This program is released under the GPL with the additional exemption 
 *   that compiling, linking, and/or using OpenSSL is allowed.
 *   
 *   
 *   
 *   
*/
#ifndef BSWAP_H
#define BSWAP_H

#include <stdint.h>
#include <config.h>

#if HAVE_LIBKERN_OSBYTEORDER_H
#   include <libkern/OSByteOrder.h>
#   define __bswap_16(x) OSSwapInt16(x)
#   define __bswap_32(x) OSSwapInt32(x)
#elif HAVE_BYTESWAP_H
#   include <byteswap.h>
#else
#   define __bswap_16(x) ((uint16_t)((((uint16_t)(x) & 0xff00) >> 8) | \
                                     (((uint16_t)(x) & 0x00ff) << 8)))
#   define __bswap_32(x) ((uint32_t)((((uint32_t)(x) & 0xff000000) >> 24) | \
				     (((uint32_t)(x) & 0x00ff0000) >>  8) | \
				     (((uint32_t)(x) & 0x0000ff00) <<  8) | \
                                     (((uint32_t)(x) & 0x000000ff) << 24)))
#endif
#endif
