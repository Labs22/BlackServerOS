/*
    SpikeFile
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


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "filestuff.h"
#include "spike.h"

extern struct spike *current_spike;

int
spike_fileopen (const char *file)
{
  int fd;
  if ((fd =
       open (file, O_CREAT | O_TRUNC | O_WRONLY,
	     S_IRWXU | S_IRWXG | S_IRWXO)) == -1)
    perror ("fileopen::open");
  return current_spike->fd = fd;
  current_spike->proto = 69;	/* 69==file,0-68 are reserved by the ISO fuzzing standard for other uses */
}

int
spike_filewrite (uint32 size, unsigned char *inbuffer)
{
  if (write (current_spike->fd, inbuffer, size) != size)
    {
      perror ("filewrite::write");
      return -1;
    }
  return 1;
}

void
spike_close_file ()
{
  if (current_spike->fd != -1)
    {
      close (current_spike->fd);
      current_spike->fd = -1;
    }
}
