#include "ifuzz.h"


int
count_options (char *optstring)
{
  int count = 0;
  char *ptr = optstring;
  while ((ptr = strchr (ptr, ':')))
    {
      count++;
      ptr++;
    }
  return count;
}

int
count_flags (char *optstring)
{
  return p_strlen (optstring) - count_options (optstring) * 2;
}

int
factorial (int num)
{
  int result = 1;
  while (num)
    result *= num--;
  return result;
}


char *
get_short_option (char opt)
{
  char *ptr;
  ptr = rmalloc (3);
  sprintf (ptr, "-%c", opt);
  return ptr;
}

void
parse_getopt (char *optstring, char **options, char **flags)
{
  int option_count;
  int flag_count;
  char *ptr = optstring;

  option_count = count_options (optstring);	/* how many options that take arguments */
  flag_count = count_flags (optstring);	/* how many simple flags/switches */

  printf ("%d options, %d flags\n", option_count, flag_count);


  *options = calloc (1, option_count + 1);	/* options represented by 1 byte each */
  *flags = calloc (1, flag_count + 1);


/* ultimately just breaks something like a:b:c:def into options:"abc" and flags:"def" */
  while (*ptr++)
    {
      if (*ptr == ':')
	{
	  APPEND_BYTE_TO_STRING (*options, *(ptr - 1));

	}
      else if (*(ptr - 1) != ':' && *(ptr - 1) != '?' && *(ptr - 1) != 'h')
	{
	  APPEND_BYTE_TO_STRING (*flags, *(ptr - 1));
	}
    }

  return;
}
