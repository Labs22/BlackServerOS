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


/*listener.c*/
#include <stdlib.h>
#include <sys/types.h>
#include "listener.h"
#include "hdebug.h"
#include <string.h>		/*for strcpy */


void
clear_listener (listener * myListener)
{
  myListener->isused = UNUSED;
  myListener->isopen = 0;
  myListener->size = 0;
  myListener->crc = 0;
  myListener->iscrc = 0;
}


void
initListenerArray (listener * listenerArray)
{
  int i;


  for (i = 0; i < MAXLISTENERS; i++)
    {
      clear_listener (&(listenerArray[i]));
    }
}

listener *
find_unused_listener (listener * myList)
{

  int i;
  if (myList == NULL)
    {
      hdebug ("MY list is null!\n");
      return NULL;
    }

  /* now we do our real job - find an unused listener and return it's address */
  for (i = 0; i < MAXLISTENERS; i++)
    {
      if (!(myList[i].isused))
	{
	  return (&myList[i]);
	}
    }

  /*we failed to find an unused listener - so we return NULL */
  return (NULL);
}


listener *
add_listener (listener * myListeners, char *astring)
{
  listener *myListener;


#if 0
  printf ("Inside add listener.\n");
#endif

  myListener = find_unused_listener (myListeners);

  if (myListener == NULL)
    {
      hdebug ("Was not able to find a spare listener!\n");
      return NULL;
    }


  /*otherwise we have a place to go */
  if (astring != NULL)
    {
      /*debug info */
#if 0
      printf ("Strcpying %s\n", astring);
#endif

      strncpy (myListener->mystring, astring, sizeof (myListener->mystring));
    }
  else
    {
#if 0
      printf ("Who passed null into add_listener?\n");
#endif

      strncpy (myListener->mystring, "", sizeof (myListener->mystring));
    }

  /*set the listener to open and used */
  myListener->isused = 1;
  myListener->isopen = 1;

  hdebug ("Added a listener.\n");
  return myListener;
}

listener *
find_listener (listener * listenerArray, char *astring)
{
  int i;

  /*if not a valid array */
  if (listenerArray == NULL)
    return NULL;

  for (i = 0; i < MAXLISTENERS; i++)
    {
      if (listenerArray[i].isused
	  && (!strcmp (astring, listenerArray[i].mystring)))
	return (&(listenerArray[i]));
    }
  /*not found */
  return NULL;

}
