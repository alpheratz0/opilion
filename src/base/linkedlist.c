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

#include "../util/xmalloc.h"
#include "linkedlist.h"

extern void
linkedlist_append(struct linkedlist **ref, void *data)
{
	while (NULL != ref[0]) {
		ref = &ref[0]->next;
	}

	ref[0] = xmalloc(sizeof(struct linkedlist));
	ref[0]->data = data;
	ref[0]->next = NULL;
}

extern void *
linkedlist_get(struct linkedlist *list, uint32_t pos)
{
	while (pos-- > 0 && NULL != list) {
		list = list->next;
	}

	return list->data;
}

extern uint32_t
linkedlist_length(struct linkedlist *list)
{
	uint32_t length;

	for (length = 0; NULL != list; ++length) {
		list = list->next;
	}

	return length;
}

extern void
linkedlist_free(struct linkedlist *list)
{
	struct linkedlist *head, *temp;

	head = list;

	while (head) {
		temp = head;
		head = head->next;
		free(temp);
	}
}
