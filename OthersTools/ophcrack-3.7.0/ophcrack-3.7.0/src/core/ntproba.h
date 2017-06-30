/*
 *   
 *   Ophcrack is a Lanmanager/NTLM hash cracker based on the faster time-memory
 *   trade-off using rainbow tables. 
 *   
 *   Created with the help of: Maxime Mueller, Luca Wullschleger, Claude
 *   Hochreutiner, Andreas Huber and Etienne Dysli.
 *   
 *   Copyright (c) 2013 Philippe Oechslin, Cedric Tissieres, 
 *                      Bertrand Mesot, Pierre Lestringant
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
#ifndef NTPROBA_H
#define NTPROBA_H

#include "table.h"

static const uint64_t ntprobafree_sizes[8] = { 3, 131076, 90178767, 120238356, 90186573, 120248764, 90172467, 120229956};
static const uint64_t ntproba10g_sizes[10] = { 4, 2097156, 1190375745, 1587167660, 1190385840, 1587181120, 1190415414, 1587220552, 1190490027, 1587320036};
static const uint64_t ntproba60g_sizes[10] = { 4, 16777220, 5742439644, 9570732740, 5742564921, 9570941535, 5742605898, 9571009830, 5742597219, 9570995365};

int ntproba_setup(void *tbl_);
void *ntproba_init(void *tbl_);
void ntproba_cleanup(void *tbl_);

void ntproba_find(void *hsh_, void *tbl_, void *el_);
int ntproba_check(void *hsh_, void *tbl_, void *el_);
int ntproba_isvalid(void *hsh_, void *tbl_);

int ntproba_lookup_idx(void *hsh_, void *tbl_, void *el_);
int ntproba_lookup_end(void *hsh_, void *tbl_, void *el_);
int ntproba_lookup_srt(void *hsh_, void *tbl_, void *el_);

#endif
