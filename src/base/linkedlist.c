/*
	Copyright (C) 2022 <alpheratz99@protonmail.com>

	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU General Public License version 2 as published by the
	Free Software Foundation.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place, Suite 330, Boston, MA 02111-1307 USA

*/

#include <stdlib.h>
#include <stdint.h>

#include "../util/debug.h"
#include "linkedlist.h"

extern void
linkedlist_append(linkedlist_t **ref, void *data)
{
	while (NULL != ref[0]) {
		ref = &ref[0]->next;
	}

	if (NULL == (ref[0] = malloc(sizeof(node_t)))) {
		die("error while calling malloc, no memory available");
	}

	ref[0]->data = data;
	ref[0]->next = NULL;
}

extern void *
linkedlist_get(linkedlist_t *list, uint32_t pos)
{
	while (pos-- > 0 && NULL != list) {
		list = list->next;
	}

	return list->data;
}

extern uint32_t
linkedlist_length(linkedlist_t *list)
{
	uint32_t length;

	for (length = 0; NULL != list; ++length) {
		list = list->next;
	}

	return length;
}

extern void
linkedlist_free(linkedlist_t *list)
{
	linkedlist_t *head, *temp;

	head = list;

	while (head) {
		temp = head;
		head = head->next;
		free(temp);
	}
}
