#include "ifuzz.h"

static char *ptr_list[MAX_HEAP_STRINGS];

void
rfree ()
{
  int ix = 0;
  while (ptr_list[ix] && (ix < MAX_HEAP_STRINGS))
    {
      free (ptr_list[ix]);
      ptr_list[ix++] = NULL;
    }
  return;
}


/* malloc and register some heap space */
void *
rmalloc (size_t size)
{
  int ix = 0;
  void *ptr;
  if (!size)
    return NULL;

  if (!(ptr = malloc (size)))
    {
      fprintf (stderr, "FATAL ERROR: OUT OF MEMORY IN RMALLOC\n");
      exit (-1);
    }
  while (ptr_list[ix] && (ix < MAX_HEAP_STRINGS))
    ix++;
  if (ix < MAX_HEAP_STRINGS)
    ptr_list[ix] = ptr;
  else
    {
      fprintf (stderr, "FATAL ERROR: TOO MANY HEAP BASED FUZZ STRINGS.\n");
      fprintf (stderr,
	       "TRY ADJUSTING 'MAX_HEAP_STRINGS' OR JUST DON'T BE SUCH A MANIAC ALL THE TIME\n");
      exit (-1);
    }
  return ptr;
}
