/*
    notSPIKEfile
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

#ifndef _FUZZ_UTILS_H
#define _FUZZ_UTILS_H

struct bloblist
{
	void *blob;
	size_t len;
};

char **F_build_argv (char * buff,char *last);
char *F_dstring_replace (char *buff, char *before, const char *after);
int8_t F_mmap_replace (size_t byte, char *infile, char *outfile, fuzztype type,int index);
void init_fuzz();
int F_ascii2signum(char *sig);
int get_signum(char *sig);
char stupid_progress(int ix);
off_t get_filesize(char *filename);
off_t assert_range(char *pBasefile,off_t endbyte);

#endif
