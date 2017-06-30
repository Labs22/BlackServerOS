/* 
** slightly modified version of sharefuzz
** allows you to use current environment
** values for specified variables
*/

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define VERSION 1.0

#define SIZE 10000		/*size of our returned environment */
#define FUZCHAR 0x50		/*our fuzzer character */

extern char **environ;

/* 
** NULL terminated list of environment variables we want to take
** from the users real environment AKA "DON'T FUZZ THESE"
*/

char *check_environment[] = {
  "DISPLAY",
  "PATH",
  NULL
};

static char stuff[SIZE];

char *_getenv(char *string)
{
int ix=0;
while (environ[ix])
{
 if ( ! ( strncmp(string,environ[ix],strlen(string))) && (environ[ix][strlen(string)] == '=')  )
 {
  return environ[ix]+strlen(string)+1;
 }
 ix++;
}
return 0;
}



char *
getenv (char *environment)
{
  int ix;
  fprintf (stderr, "GETENV: %s ", environment);

  for (ix = 0; check_environment[ix]; ix++)
    {
      if (!(strcmp (check_environment[ix], environment)))
	{
	  fprintf (stderr, "[user environment]\n");
	  return _getenv (environment);
	}
    }
  printf ("[fuzz]\n");
  return stuff;
}

_init ()
{
  memset (stuff, FUZCHAR, SIZE - 1);
  memcpy (stuff, "/%x%n%n%n%n%n%n", 15);
  stuff[SIZE - 1] = 0x0;
  stuff[SIZE - 2] = 0x2f;
}
