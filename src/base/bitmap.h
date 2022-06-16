#ifndef __XPAVM_BASE_BITMAP_H__
#define __XPAVM_BASE_BITMAP_H__

#include <stdint.h>

typedef struct bitmap bitmap_t;

struct bitmap {
	uint32_t *px;
	uint32_t width;
	uint32_t height;
};

extern bitmap_t *
bitmap_create(uint32_t width, uint32_t height, uint32_t color);

extern void
bitmap_set(bitmap_t *bmp, uint32_t x, uint32_t y, uint32_t color);

extern uint32_t
bitmap_get(bitmap_t *bmp, uint32_t x, uint32_t y);

extern void
bitmap_rect(bitmap_t *bmp,
            uint32_t x,
            uint32_t y,
            uint32_t width,
            uint32_t height,
            uint32_t color);

extern void
bitmap_clear(bitmap_t *bmp, uint32_t color);

extern void
bitmap_free(bitmap_t *bmp);

#endif
