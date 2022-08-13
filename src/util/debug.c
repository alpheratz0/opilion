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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include "debug.h"

extern void
warn(const char *wrn)
{
	fprintf(stderr, "xpavm: %s\n", wrn);
}

extern void
warnf(const char *fmt, ...)
{
	va_list args;

	fputs("xpavm: ", stderr);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
}

extern void
die(const char *err)
{
	char command[256];

	if (isatty(STDOUT_FILENO)) {
		fprintf(stderr, "xpavm: %s\n", err);
	}
	else {
		snprintf(command, sizeof(command), "notify-send \"xpavm\" \"%s\"", err);
		system(command);
	}
	exit(1);
}

extern void
dief(const char *fmt, ...)
{
	va_list args;

	fputs("xpavm: ", stderr);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
	exit(1);
}
