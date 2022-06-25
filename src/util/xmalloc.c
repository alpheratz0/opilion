#include <stdlib.h>

#include "debug.h"
#include "xmalloc.h"

extern void *
xmalloc(size_t size)
{
	void *p;

	if (NULL == (p = malloc(size))) {
		die("error while calling malloc, no memory available");
	}

	return p;
}
