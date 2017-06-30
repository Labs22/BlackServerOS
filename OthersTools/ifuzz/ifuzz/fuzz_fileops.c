#include "ifuzz.h"

/*
** just verifies if the "file" exists.
** doesnt enforce it to be a regular file,
** could be a directory or fifo or symlink or whatever
*/
void
verify_file ()
{
  struct stat statbuf;

  if (stat (VALID_FILE, &statbuf))
    {
      creat (VALID_FILE, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);	/* 755 */
    }
  return;
}

/* fairly blindly remove VALID_FILE, unless its a symlink */
void
remove_file ()
{
  struct stat statbuf;
  fprintf (stderr, "called\n");
  if (!(stat (VALID_FILE, &statbuf)))
    {
      if (!(S_ISLNK (statbuf.st_mode)))
	{
	  unlink (VALID_FILE);
	  fprintf (stderr, "remove\n");
	}
    }


  return;
}
