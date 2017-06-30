/*
 *   
 *   Ophcrack is a Lanmanager/NTLM hash cracker based on the faster time-memory
 *   trade-off using rainbow tables. 
 *   
 *   Created with the help of: Maxime Mueller, Luca Wullschleger, Claude
 *   Hochreutiner, Andreas Huber and Etienne Dysli.
 *   
 *   Copyright (c) 2008 Philippe Oechslin, Cedric Tissieres, Bertrand Mesot
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
/** @file list.c
 *  Implementation of a doubly linked list.
 */

#include <stdlib.h>
#include <assert.h>

#include "list.h"
/*-------------------------------------------------------------------------*/
list_t *list_alloc(void) {
  list_t *l = (list_t*)malloc(sizeof(list_t));

  l->size = 0;
  l->head = 0;
  l->tail = 0;

  return l;
}
/*-------------------------------------------------------------------------*/
void list_free(list_t *l) {
  list_clear(l);
  free(l);
}
/*-------------------------------------------------------------------------*/
void list_clear(list_t *l) {
  while (l->size)
    list_rem_head(l);
}
/*-------------------------------------------------------------------------*/
void list_add_head(list_t *l, void *data) {
  list_nd_t *nd  = list_nd_alloc(data);
  list_nd_t *old = l->head;

  nd->next = old;
  l->head  = nd;
  l->size += 1;

  if (old != 0) 
    old->prev = nd;
  else
    l->tail = nd;
}
/*-------------------------------------------------------------------------*/
void list_add_tail(list_t *l, void *data) {
  list_nd_t *nd  = list_nd_alloc(data);
  list_nd_t *old = l->tail;

  nd->prev = old;
  l->tail  = nd;
  l->size += 1;

  if (old != 0) 
    old->next = nd;
  else
    l->head = nd;
}
/*-------------------------------------------------------------------------*/
void *list_rem_head(list_t *l) {
  list_nd_t *nd = l->head;

  if (nd == 0) return 0;

  list_nd_t *old = nd->next;

  l->head  = old;
  l->size -= 1;

  if (old != 0) 
    old->prev = 0;
  else
    l->tail = 0;
  
  return list_nd_free(nd);
}
/*-------------------------------------------------------------------------*/
void *list_rem_tail(list_t *l) {
  list_nd_t *nd = l->tail;

  if (nd == 0) return 0;

  list_nd_t *old = nd->prev;

  l->tail  = old;
  l->size -= 1;

  if (old != 0) 
    old->next = 0;
  else
    l->head = 0;

  return list_nd_free(nd);
}
/*-------------------------------------------------------------------------*/
list_nd_t *list_nd_alloc(void *data) {
  list_nd_t *nd = (list_nd_t*)malloc(sizeof(list_nd_t));

  nd->next = 0;
  nd->prev = 0;
  nd->data = data;

  return nd;
}
/*-------------------------------------------------------------------------*/
void *list_nd_free(list_nd_t *nd) {
  void *data = nd->data;

  free(nd);

  return data;
}
