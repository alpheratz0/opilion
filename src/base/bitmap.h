#ifndef __XPAVM_BASE_BITMAP_H__
#define __XPAVM_BASE_BITMAP_H__

#include "../util/numdef.h"

typedef struct bitmap bitmap_t;

struct bitmap {
	u32 *px;
	u32 width;
	u32 height;
};

extern bitmap_t *
bitmap_create(u32 width, u32 height, u32 color);

extern void
bitmap_set(bitmap_t *bmp, u32 x, u32 y, u32 color);

extern void
bitmap_set_safe(bitmap_t *bmp, u32 x, u32 y, u32 color);

extern u32
bitmap_get(bitmap_t *bmp, u32 x, u32 y);

extern u32
bitmap_get_safe(bitmap_t *bmp, u32 x, u32 y);

extern void
bitmap_rect(bitmap_t *bmp, u32 x, u32 y, u32 width, u32 height, u32 color);

extern void
bitmap_clear(bitmap_t *bmp, u32 color);

extern void
bitmap_copy(bitmap_t *from, bitmap_t *to, u32 x, u32 y);

extern void
bitmap_save(bitmap_t *bmp, const char *path);

extern void
bitmap_free(bitmap_t *bmp);

#endif
