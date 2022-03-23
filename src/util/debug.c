#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern void
warn(const char *err) {
	fprintf(stderr, "xpavm: %s\n", err);
}

extern void
warnf(const char *err, ...) {
	va_list list;
	fputs("xpavm: ", stderr);
	va_start(list, err);
	vfprintf(stderr, err, list);
	va_end(list);
	fputs("\n", stderr);
}

extern void
die(const char *err) {
	fprintf(stderr, "xpavm: %s\n", err);
	exit(1);
}

extern void
dief(const char *err, ...) {
	va_list list;
	fputs("xpavm: ", stderr);
	va_start(list, err);
	vfprintf(stderr, err, list);
	va_end(list);
	fputs("\n", stderr);
	exit(1);
}
