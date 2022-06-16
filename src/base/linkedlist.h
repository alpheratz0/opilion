#ifndef __XPAVM_BASE_LINKEDLIST_H__
#define __XPAVM_BASE_LINKEDLIST_H__

#include <stdint.h>

typedef struct node linkedlist_t;
typedef struct node node_t;

struct node {
	node_t *next;
	void *data;
};

extern void
linkedlist_append(linkedlist_t **ref, void *data);

extern void *
linkedlist_get(linkedlist_t *list, uint32_t pos);

#define linkedlist_get_as(list, pos, ptype) \
	((ptype *)(linkedlist_get(list, pos)))

extern uint32_t
linkedlist_length(linkedlist_t *list);

extern void
linkedlist_free(linkedlist_t *list);

#endif
