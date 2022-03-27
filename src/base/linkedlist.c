#include <stdio.h>
#include <stdlib.h>
#include "../util/numdef.h"
#include "linkedlist.h"

extern void
linkedlist_append(linkedlist_t **ref, void *data) {
	while (*ref) {
		ref = &((*ref)->next);
	}

	*ref = malloc(sizeof(node_t));
	(*ref)->data = data;
	(*ref)->next = NULL;
}

extern void *
linkedlist_get(linkedlist_t *list, u32 pos) {
	while (pos--) {
		list = list->next;
	}
	return list->data;
}

extern u32
linkedlist_length(linkedlist_t *list) {
	for (u32 length = 0;;++length) {
		if (!list) {
			return length;
		}
		list = list->next;
	}
}

extern void
linkedlist_free(linkedlist_t *list) {
	if (list) {
		if (list->next) {
			linkedlist_free(list->next);
		}
		free(list);
	}
}
