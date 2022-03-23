#ifndef __XPAVM_UTIL_DEBUG_H__
#define __XPAVM_UTIL_DEBUG_H__

extern void
warn(const char *err);

extern void
warnf(const char *err, ...);

extern void
die(const char *err);

extern void
dief(const char *err, ...);

#endif
