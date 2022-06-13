#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include "debug.h"

extern void
warn(const char *err)
{
	fprintf(stderr, "xpavm: %s\n", err);
}

extern void
warnf(const char *err, ...)
{
	va_list list;
	fputs("xpavm: ", stderr);
	va_start(list, err);
	vfprintf(stderr, err, list);
	va_end(list);
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
		snprintf(command, sizeof(command), "notify-send \"xpavm: %s\"", err);
		system(command);
	}
	exit(1);
}

extern void
dief(const char *err, ...)
{
	va_list list;
	fputs("xpavm: ", stderr);
	va_start(list, err);
	vfprintf(stderr, err, list);
	va_end(list);
	fputc('\n', stderr);
	exit(1);
}
