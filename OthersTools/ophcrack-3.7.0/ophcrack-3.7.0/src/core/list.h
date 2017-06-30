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
/** @file list.h
 *  Declarations for a doubly linked list.
 */

#ifndef LIST_H
#define LIST_H

#ifdef  __cplusplus
extern "C" {
#endif

/** A list node. */

typedef struct list_nd_t_ {
  void *data;                 /**< The element to be stored. */
  struct list_nd_t_ *next;    /**< A pointer to the following node. */
  struct list_nd_t_ *prev;    /**< A pointer to the previous node. */
} list_nd_t;

/** A doubly linked list. */

typedef struct list_t_ {
  int size;                   /**< The size of the list */
  list_nd_t *head;            /**< A pointer to the head of the list. */
  list_nd_t *tail;            /**< A pointer to the tail of the list. */
} list_t;

/** Allocate memory for a list. */

list_t *list_alloc(void);

/** Free the memory used by the list.
 *  
 *  The memory used by the element is not freed. It is therefore your
 *  responsability to clean up that part of the memory.
 *
 *  @param l A pointer to the list to be cleared.
 */

void list_free(list_t *l);

/** Empty the list. */

void list_clear(list_t *l);

/** Add an element to the head of the list. 
 *  @param l A pointer to the list.
 *  @param data A pointer to the element.
*/

void list_add_head(list_t *l, void *data);

/** Add an element to the tail of the list. 
 *  @param l A pointer to the list.
 *  @param data A pointer to the element.
*/

void list_add_tail(list_t *l, void *data);

/** Remove an element from the head of the list.
 *  @param l A pointer to the list.
 *  @return A pointer to the element stored in the head.
 */

void *list_rem_head(list_t *l);

/** Remove an element from the tail of the list.
 *  @param l A pointer to the list.
 *  @return A pointer to the element stored in the tail.
 */

void *list_rem_tail(list_t *l);

/** Allocate memory for a list node.
 *  @param data A pointer to the element.
 *  @return A list node containing the element.
 */

list_nd_t *list_nd_alloc(void *data);

/** Free the memory used by a node.
 *  @param nd The node to be freed.
 *  @return A pointer to the element stored in the node.
 */

void *list_nd_free(list_nd_t *nd);

#ifdef  __cplusplus
}
#endif
#endif
