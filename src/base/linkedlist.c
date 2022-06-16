#include <stdio.h>
#include <stdlib.h>

#include "../util/debug.h"
#include "../util/numdef.h"
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
linkedlist_get(linkedlist_t *list, u32 pos)
{
	while (pos-- > 0 && NULL != list) {
		list = list->next;
	}

	return list->data;
}

extern u32
linkedlist_length(linkedlist_t *list)
{
	u32 length;

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
