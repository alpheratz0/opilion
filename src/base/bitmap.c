#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../util/numdef.h"
#include "../util/debug.h"
#include "bitmap.h"

extern bitmap_t *
bitmap_create(u32 width, u32 height, u32 color)
{
	bitmap_t *bmp;

	if (NULL == (bmp = malloc(sizeof(bitmap_t)))) {
		die("error while calling malloc, no memory available");
	}

	bmp->width = width;
	bmp->height = height;

	if (NULL == (bmp->px = malloc(4*width*height))) {
		die("error while calling malloc, no memory available");
	}

	bitmap_clear(bmp, color);

	return bmp;
}

extern void
bitmap_set(bitmap_t *bmp, u32 x, u32 y, u32 color)
{
	if (x < bmp->width && y < bmp->height) {
		bmp->px[y*bmp->width+x] = color;
	}
}

extern u32
bitmap_get(bitmap_t *bmp, u32 x, u32 y)
{
	if (x < bmp->width && y < bmp->height) {
		return bmp->px[y*bmp->width+x];
	}

	return 0;
}

extern void
bitmap_rect(bitmap_t *bmp, u32 x, u32 y, u32 width, u32 height, u32 color)
{
	for (u32 i = 0; i < width; ++i) {
		for (u32 j = 0; j < height; ++j) {
			bmp->px[(j+y)*bmp->width+i+x] = color;
		}
	}
}

extern void
bitmap_clear(bitmap_t *bmp, u32 color)
{
	for (u32 x = 0; x < bmp->width; ++x) {
		for (u32 y = 0; y < bmp->height; ++y) {
			bmp->px[y*bmp->width+x] = color;
		}
	}
}

extern void
bitmap_free(bitmap_t *bmp)
{
	free(bmp->px);
	free(bmp);
}
