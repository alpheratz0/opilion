/*
	Copyright (C) 2022-2023 <alpheratz99@protonmail.com>

	This program is free software; you can redistribute it and/or modify it
	under the terms of the GNU General Public License version 2 as published by
	the Free Software Foundation.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
	FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
	more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc., 59
	Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/

#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "log.h"

extern void *
xmalloc(size_t size)
{
	void *ptr;
	if (NULL == (ptr = malloc(size)))
		die("OOM");
	return ptr;
}

extern void *
xcalloc(size_t nmemb, size_t size)
{
	void *ptr;
	if (NULL == (ptr = calloc(nmemb, size)))
		die("OOM");
	return ptr;
}

extern char *
xstrdup(const char *str)
{
	size_t len;
	char *res;

	if (NULL == str)
		return NULL;

	len = strlen(str);
	res = xmalloc(len + 1);
	memcpy(res, str, len);
	res[len] = '\0';

	return res;
}
