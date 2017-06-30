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
/* This file is a copy from implCPU/v6 */

#ifndef PATTERNDESCRIPTOR_H
#define PATTERNDESCRIPTOR_H

/*Attention la taille max d'un mot de passe est fixé à 16 caractères*/

#define PATTERN_ARRAY_MAX_LENGTH 16

#define DESC_IGNORED				0x00
#define DESC_NUMERAL				0x02
#define DESC_SPECIAL				0x01
#define DESC_UPPER_NOMARK			0x0B
#define DESC_UPPER_MARKFIRST		0x0C
#define DESC_UPPER_MARKSECOND		0x0D
#define DESC_UPPER_MARK				0x0E
#define DESC_LOWER_NOMARK			0x03
#define DESC_LOWER_MARKFIRST		0x04
#define DESC_LOWER_MARKSECOND		0x05
#define DESC_LOWER_MARK				0x06


#define NO_MARK_IGNORED				0x00
#define NO_MARK_NUMERAL				0x02
#define NO_MARK_SPECIAL				0x01
#define NO_MARK_LETTER				0x03

#define MARKOV_FIRST				0x00
#define MARKOV_SECOND				0x01
#define MARKOV_MAIN					0x02


#endif