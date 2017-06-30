/*
  decode_ospf.c

  Open Shortest Path First.
  
  Copyright (c) 2000 Dug Song <dugsong@monkey.org>
 
  $Id: decode_ospf.c,v 1.5 2000/12/15 20:16:58 dugsong Exp $
*/

#include "config.h"

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "decode.h"

int
decode_ospf(u_char *buf, int len, u_char *obuf, int olen)
{
	if (len < 25)
		return (0);
	
	if (pntohs(buf + 14) != 1)
		return (0);
	
	buf[24] = '\0';
	
	return (snprintf(obuf, olen, "%s\n", buf + 16));
}

