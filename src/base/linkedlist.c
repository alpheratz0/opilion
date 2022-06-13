#include <stdio.h>
#include <stdlib.h>

#include "../util/numdef.h"
#include "linkedlist.h"

extern void
linkedlist_append(linkedlist_t **ref, void *data)
{
	while (NULL != *ref) {
		ref = &((*ref)->next);
	}

	*ref = malloc(sizeof(node_t));
	(*ref)->data = data;
	(*ref)->next = NULL;
}

extern void *
linkedlist_get(linkedlist_t *list, u32 pos)
{
	while (pos--) {
		list = list->next;
	}

	return list->data;
}

extern u32
linkedlist_length(linkedlist_t *list)
{
	for (u32 length = 0;;++length) {
		if (NULL == list) {
			return length;
		}
		list = list->next;
	}
}

extern void
linkedlist_free(linkedlist_t *list)
{
	linkedlist_t *head;
	linkedlist_t *temp;

	head = list;

	while (head) {
		temp = head;
		head = head->next;
		free(temp);
	}
}
