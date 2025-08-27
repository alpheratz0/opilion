/*
	Copyright (C) 2022-2025 <alpheratz99@protonmail.com>

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "utils.h"
#include "log.h"

extern const char *
str_fallback(const char *str, const char *fallback)
{
	if (NULL == str)
		return fallback;
	return str;
}

extern char *
str_fmt(const char *fmt, ...)
{
	va_list args;
	static char fmt_buf[512];

	va_start(args, fmt);
	vsnprintf(fmt_buf, sizeof(fmt_buf), fmt, args);
	va_end(args);

	return xstrdup(&fmt_buf[0]);
}

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

extern void *
xrealloc(void *orig_ptr, size_t size)
{
	void *ptr;
	if (NULL == (ptr = realloc(orig_ptr, size)))
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
