/*
    SpikeFile
    Copyright (C) 2005 Adam Greene <agreene@idefense.com>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
    more details.

    You should have received a copy of the GNU General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place, Suite 330, Boston, MA 02111-1307 USA
*/


/* spike_oncrpc.h
 * Dominique Brezinski, 2002
 * License: GPL v2.0
 *
 * This is the public header file for spike_oncrpc.c 
 * Use these functions to fuzz ONC-RPC applications to your heart's content
 */

#include <sys/types.h>
#include "spike.h"

struct onc_endpoint
{
  uint prognum;
  uint progver;
  uint proto;
  uint port;
};


void s_onc_call_header (uint, uint, uint, uint);

void s_onc_opaqueauth_none ();

void s_onc_opaqueauth_sys (uint, unsigned char *, uint, uint, uint *, uint);

void s_onc_verifier_none ();

int s_binary_block_size_onc_recordfrag (char *);

int s_onc_push_random_bytes (uint);

void s_onc_push_random_xdr_thing ();

int s_onc_do_portmap_getport (char *, struct onc_endpoint *);

int s_onc_do_portmap_dump (char *, struct onc_endpoint *, uint);

int s_onc_read_reply (struct spike *, unsigned char *, uint *);

int s_onc_parse_response (unsigned char *, uint, unsigned char *, uint);
