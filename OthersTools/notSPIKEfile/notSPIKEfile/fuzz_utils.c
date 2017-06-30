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


#include <notSPIKEfile.h>

char **stringlist = NULL;
int stringcount=0;
struct bloblist **bloblist = NULL;
int blobcount=0;

char stupid_progress(int ix)
{
	char buff[] = "|/-\\";
	return(buff[ix%4]);
}

off_t get_filesize(char *filename)
{
	struct stat fstat;
	if ( stat(filename,&fstat) == -1 )
	{
		fprintf(stderr,"File %s doesn't exist\n",filename);
		exit(ERR_BAD);
	}
	return(fstat.st_size);
}

off_t assert_range(char *pBasefile,off_t endbyte)
{
	if ( endbyte >= get_filesize(pBasefile) )
	{
		/* XXX remove the fprintf ? XXX */
		fprintf(stderr,"Invalid byte range for file.\n");
		return(get_filesize(pBasefile));
	}
	else return(ERR_OK);
}

void add_blob(void *blob,size_t len)
{
	bloblist = realloc(bloblist,sizeof(struct bloblist *) * (blobcount+1));
	bloblist[blobcount] = malloc(sizeof(struct bloblist));
	bloblist[blobcount]->blob = malloc(len);
	bloblist[blobcount]->len = len;
	memcpy(bloblist[blobcount++]->blob,blob,len); 
	return;
}

/* why does this code feel more like spike than something called "notspike" should? */

void add_string(char *string)
{
	stringlist = realloc(stringlist,sizeof(char *) * (stringcount+1));
	stringlist[stringcount++] = strdup(string);
	return;
}

/* msb = 1 means do it in msb */
void add_int16(u_int16_t integer,int msb)
{
	if ( msb )
	{
		integer = htons(integer);
	}

	add_blob(&integer,2);
}

/* msb = 1 means do it in msb */
void add_int32(u_int32_t integer,int msb)
{
	if ( msb )
	{
		integer = htonl(integer);
	}

	add_blob(&integer,4);
}

/* XXX add more blobs like 8 and 12 byte ones XXX */
/* XXX add m byte blobs split by n bytes example: (4,2) == 0x80008000 XXX */
void init_fuzz()
{
	char buff[512000];
	int ix;
	memset(buff,0x41,512000);

/* uncomment these to be thorough. they are commented
** simply to simplify my debug testsuite
*/

/*
	buff[140000-1] = 0x0;
	add_string(buff);
	buff[66000-1] = 0x0;
	add_string(buff);
	buff[33000-1] = 0x0;  
	add_string(buff);
	buff[17000-1] = 0x0;
	add_string(buff);
	buff[9000-1] = 0x0;
	add_string(buff);
	buff[4100-1] = 0x0;
	add_string(buff);
	memset(buff,'0',512000);
	buff[140000-1] = 0x0;
	add_string(buff);
	buff[66000-1] = 0x0;
	add_string(buff);
*/
	add_string("%n%n%n%n%n%n");
	buff[33000-1] = 0x0;  
	add_string(buff);
	buff[17000-1] = 0x0;
	add_string(buff);
	buff[9000-1] = 0x0;
	add_string(buff);
	buff[4100-1] = 0x0;
	add_string(buff);

	/* these arent terribly likely to be effective as strings but its worth a shot */
	add_string("65535");
	add_string("65534");
	add_string("2147483647");
	add_string("2147483648");
	add_string("-1");
	add_string("4294967295");
	add_string("4294967294");
	add_string("0");
	add_string("1");
	add_string("357913942");
	add_string("-2147483648");
	add_string("536870912");

	/* target multiplication type integer overflows */
	for ( ix=2;ix<16;ix+=2 )
	{
		add_int32(0xffffffff/ix+1,0);
		add_int32(0xffffffff/ix+1,1);
		add_int16((0xffff/ix)+1,0);
		add_int16((0xffff/ix)+1,1);
		add_int32((0xffffffff/ix),0);
		add_int32((0xffffffff/ix),1);
		add_int16((0xffff/ix),0);
		add_int16((0xffff/ix),1);
	}
	/* target small increment/decrement/add/subtract type integer overflows */
	for ( ix=-8;ix<8;ix++ )
	{
		add_int16((short)ix,0);
		add_int16((short)ix,1);
		add_int32(ix,0);
		add_int32(ix,1);
	}

	/* target non typical length fields */
	add_blob("\xff",1);
	add_blob("\xff\xff\xff\xff\xff\xff\xff\xff",8);

	if ( !quiet )
	{
		printf("initialized %d strings, %d blobs\n",stringcount,blobcount);
	}
}

/* makes buff into an execve compatible array, makes last
** the last argument if not NULL
*/
char **
F_build_argv (char * buff,char * last)
{
	char *ptr;
	char **argp;
	char *string;
	u_int32_t ix = 1;
	u_int32_t elements;
	string = strdup (buff);
	ptr = string;

	while ( (ptr = strstr (ptr, " ")) )
	{
		ix++;
		while ( (*(ptr++) == ' ') )	 /* so multiple spaces is ok */
			if ( !(*ptr) )
				ix--; /* so trailing spaces with no more args is ok */
	}
	elements = ++ix;



	if ( !(argp = malloc (sizeof (char *) * (elements + 1 + (last!=NULL) ))) )
	{

		fprintf (stderr, "F_build_argv: malloc failed. fatal\n");
		exit (-1);
	}

	memset (argp, 0x0, sizeof (char *) * (elements + 1+(last!=NULL)));

	for ( ix = 0, ptr = string; ix < elements - 1; ix++ )
	{
		char *end;
		while ( *ptr && (*ptr == ' ') )
			ptr++;
		end = ptr;
		while ( *end && ((*end) != ' ') )
			end++;
		*end = 0x0;
		argp[ix] = strdup (ptr);
		ptr = end + 1;
	}
	if ( last )
	{
		if ( !quiet )
		{
			printf("Adding %s\n",last);
		}
		argp[ix++] = last;
	}
	argp[ix] = NULL;
	return(argp);
}

/* very quick sloppy way to insert a fuzzed string into a file using mmap */
/*
XXXXXX
XXXX
XXXXXX
XXXX
XXXXXX

Needs proper bailout routine!!!!!!
*/
int8_t
F_mmap_replace (size_t byte, char *infile, char *outfile, fuzztype type,int index)
{
	int fdin, fdout;
	u_int8_t *src, *dst;
	struct stat statbuf;
	unsigned char *data;
	unsigned int outsize;

	int fuzzlen;
	if ( type == F_STRING )
	{
		data = strdup(stringlist[index]);
		fuzzlen = strlen(data);
		outsize = fuzzlen; 
	}
	else if ( type == F_BLOB )
	{
		fuzzlen = bloblist[index]->len;
		data = malloc(fuzzlen);
		memcpy(data,bloblist[index]->blob,fuzzlen);
		outsize = 0;
	}
	else
	{
		printf("tiago is stupid\n");
		exit(ERR_BAD);
	}

	if ( !(infile) || !(outfile) )
	{
		fprintf (stderr, "mmap_replace bad file. fatal.\n");
		exit (ERR_BAD);
	}

	if ( (fdin = open (infile, O_RDONLY)) < 0 )
	{
		perror ("open");exit(ERR_BAD);
	}
	if ( (fdout = open (outfile, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU)) < 0 )
	{
		perror ("open"); exit(ERR_BAD);
	}

	if ( fstat (fdin, &statbuf) < 0 )
	{
		perror ("fstat"); unlink(outfile); exit(ERR_BAD);
	}


	if ( byte > statbuf.st_size )
	{
		munmap (dst, statbuf.st_size + outsize);
		close (fdin);
		close (fdout);
		free (data);
		unlink(outfile);
		return(-1);
	}

/* last byte in new fuzzed file */
	if ( lseek (fdout, statbuf.st_size + outsize - 1, SEEK_SET) == -1 )
	{
		unlink(outfile);
		exit(-1);
		perror ("lseek");
	}
	if ( write (fdout, "", 1) != 1 )
	{
		perror ("write"); /* XXX these all need close() and unmap and stuff added */
		unlink(outfile);
		exit(-1);
	}
	/* mmap the input file */
	if ( (src =
		  (u_int8_t *) mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin,
							 0)) == (void *) -1 )
	{
		unlink(outfile);  perror ("mmap");exit(-1);
	}

	if ( (dst =
		  (u_int8_t *) mmap (0, statbuf.st_size + outsize,
							 PROT_READ | PROT_WRITE, MAP_SHARED, fdout,
							 0)) == (void *) (-1) )
	{
		unlink(outfile); perror ("mmap");exit(-1);
	}

	memcpy (dst, src, byte);
	memcpy (dst + byte, data, fuzzlen);

/* review this code */
	/* should behave differently for blobs and do a true overlay insert */
	/* need to add a check if position+fuzzlen is > end of file, do something */
	if ( type == F_STRING )
	{
		memcpy (dst + byte + fuzzlen, src + byte+1, statbuf.st_size - byte-1);
	}
	else if ( type == F_BLOB ) /* add an assert that src+byte+fuzzlen + the copylen isnt beyond us */
	{
		if ( fuzzlen+byte > statbuf.st_size )
		{
			unlink(outfile);exit(-1);
		}
		memcpy (dst + byte + fuzzlen, src + byte+fuzzlen, statbuf.st_size - byte-fuzzlen);  
	}


	//memcpy (dst + byte + fuzzlen, src + byte, statbuf.st_size - byte); // this way "inserts", above way "replaces"
	munmap (src, statbuf.st_size);
	munmap (dst, statbuf.st_size + outsize);
	close (fdin);
	close (fdout);
	free (data);
	return(ERR_OK);
}

/* pass signal name/number as string, will return int */
/* ERR_BAD on error */
int get_signum(char *sig)
{
	char *err=NULL;
	int ret;



	ret = strtol(sig,&err,0);

	if ( err  != (sig+strlen(sig)) )
	{
		return(F_ascii2signum(sig));
	}
	return(ret);
}


/* buff must be a malloc,realloc,calloc or strdup string */
char *
F_dstring_replace (char *buff, char *before, const char *after)
{
	u_int8_t *ptr = buff;
	u_int16_t replacements = 0;

	while ( (ptr = strstr (ptr, before)) )
	{
		replacements++;
		ptr++;
	}
	if ( strlen (after) == strlen (before) )
	{
		ptr = buff;
		while ( (ptr = strstr (ptr, before)) )
		{
			memcpy (ptr, after, strlen (after));
			ptr++;
		}
	}
	else
	{
		size_t size;

		if ( strlen (after) > strlen (before) )
		{
			int start,stop;

			size =strlen(buff)-strlen(before)*replacements+strlen(after)*replacements+1;
			start = strlen(buff)+1;
			stop = size-start;
			buff = realloc (buff, size);
			memset(buff+start,0x0,stop);
			ptr = buff;

			while ( (ptr = strstr (ptr, before)) )
			{
				char *dst,*src;
				size_t len;

				dst = ptr+strlen(after);
				src = ptr+strlen(before);
				len = strlen(src)+1;
				memmove ((void *) (dst),(void *) (src),(size_t) len);
				memcpy (ptr, after, strlen (after));
				ptr += strlen (after);
			}
		}
		else
		{
			ptr = buff;
			size = strlen (buff) + 1;
			size -= replacements * (strlen (before) - strlen (after));

			while ( (ptr = strstr (ptr, before)) )
			{
				memcpy (ptr, after, strlen (after));
				memmove ((void *) (ptr + strlen (after)),(void *) (ptr + strlen (before) + 1),(size_t) ((buff + size) - (int) ptr));
			}
			*(buff + size - 1) = 0x0;
		}
	}
	return(buff);
}
