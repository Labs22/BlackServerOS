/* $Id */

#include "ifuzz.h"

/*
** basic fuzz strings and their random # index
**  
** strings starting with:	
**	0		/
**    	1		0-9
**    	2		#
**    	3		.
**    	4		-
**    	5		--
**    	6		%
**    	7		:
**	8		+
**  strings containing exactly one
**  	9		/ 
**	10		:
**  strings ending with
**  	11		/ 
**	12		% 
**	13		: 
**	14		:0 
**  start AND end with 
**	15		/
**  valid infor
**      16 		valid filename
**	17		valid path (directory)
**      18		valid xdisplay
**      19		valid username
**	20		valid number
**	
**  
**
** possible string sizes 100,250,500,1000,2000,3000,5000,7000,10000
*/


unsigned int string_lengths[] =
  { 100, 250, 500, 1000, 2000, 3000, 5000, 7000, 10000, MAX_FUZZ_LENGTH, 0 };

static int init = 0;

#define FS_TRIGGER "%n%n%n%n%n%n%n"	/* always try to trigger format string crap */

int
get_random_int (int min, int max)
{
  if (!init)
    {
      srand (time ((time_t *) NULL) + getpid ());
      init = 1;
    }
  if (!max)
    return 0;

  return (rand () % max) + min;
}

char *
make_string (int size, char *startswith, char *endswith, char *hasone)
{
  char *buff;
  unsigned char filler;
  int min_size = 0;

  /* don't know behavior of strlen(NULL) on all implementations */
  min_size =
    p_strlen (startswith) + p_strlen (endswith) + p_strlen (hasone) + 1;
  if (size < min_size)
    {
      printf ("error with the size of the string dude\n");
      exit (-1);
    }

  buff = rmalloc (size);

  if (!init)
    {
      srand (time ((time_t *) NULL) + getpid ());
      init = 1;
    }


  filler = (rand () % 254) + 1;

  if (hasone)
    {
      while ((p_strlen (hasone) == 1) && (filler == hasone[0])
	     && !(isprint (filler)))
	filler = (rand () % 254) + 1;
    }

  memset (buff, filler, size);
  buff[size - 1] = 0x0;
  if (startswith)
    {
      memcpy (buff, startswith, p_strlen (startswith));
      memcpy (buff + p_strlen (startswith), FS_TRIGGER, p_strlen (FS_TRIGGER));

    }
  else
    {
      memcpy (buff + 1, FS_TRIGGER, p_strlen (FS_TRIGGER));
    }


  if (endswith)
    {
      strcpy (buff + p_strlen (buff) - p_strlen (endswith) - 1, endswith);
    }

  if (hasone)
    {
      if ((p_strlen (buff) / 2) < p_strlen (hasone))
	{
	   printf("huh\n"); 
	   exit (-1);
	}
      memcpy (buff + (p_strlen (buff) / 2), hasone, p_strlen (hasone));
    }


  return buff;
}

char *
get_random_string ()
{
  char integer[2];
  int stringtype;
  int size;
  char *ptr;

  size = MAX_FUZZ_LENGTH;	// XXX make it random or select semi random from a list 

  if (!init)
    {
      srand (time ((time_t *) NULL) + getpid ());
      init = 1;
    }

  stringtype = rand () % 20 + 1;	// XXX define 20 somewhere as maximum case for future additions



  switch (stringtype)
    {
    case 0:
      return make_string (size, "/", NULL, NULL);
    case 1:
      sprintf (integer, "%d", rand () % 9);
      return make_string (size, integer, NULL, NULL);
    case 2:
      return make_string (size, "#", NULL, NULL);
    case 3:
      return make_string (size, ".", NULL, NULL);
    case 4:
      return make_string (size, "-", NULL, NULL);
    case 5:
      return make_string (size, "--", NULL, NULL);
    case 6:
      return make_string (size, "%", NULL, NULL);
    case 7:
      return make_string (size, ":", NULL, NULL);
    case 8:
      return make_string (size, "+", NULL, NULL);
    case 9:
      return make_string (size, NULL, NULL, "/");
    case 10:
      return make_string (size, NULL, NULL, ":");
    case 11:
      return make_string (size, NULL, "/", NULL);
    case 12:
      return make_string (size, NULL, "%", NULL);
    case 13:
      return make_string (size, NULL, ":", NULL);
    case 14:
      return make_string (size, NULL, ":0", NULL);
    case 15:
      return make_string (size, "/", "/", NULL);
    case 16:
      verify_file ();		/* checks to see VALID_FILE is there, if it isn't it makes it */
      return VALID_FILE;	/* from ifuzz.h */
    case 17:
      return "/tmp";		/* bah */
    case 18:
    case 19:
      return "UNIMPLEMENTED.SO.FAR";
    case 20:
      ptr = rmalloc (16);
      snprintf (ptr, 16 - 1, "%d", (rand () % 0xffffffff));
      return ptr;
      break;
    }
  return NULL;
}



size_t
p_strlen (const char *s)
{
  return (s) ? (strlen (s)) : 0;
}


/* portable strfry */
/* takes a C string as an argument and "randomly" switches the letters around */
char *
p_strfry (char *string)
{
  int len, i;

  if (!init)
    {
      srand (time ((time_t *) NULL) + getpid ());
      init = 1;
    }

  len = p_strlen (string);
  for (i = 0; i < len; ++i)
    {
      size_t j = rand () % len;
      char c = string[i];
      string[i] = string[j];
      string[j] = c;
    }

  return string;
}
