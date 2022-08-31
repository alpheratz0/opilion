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

#include <stdlib.h>
#include <stdint.h>

#include "../util/xmalloc.h"
#include "bitmap.h"

extern struct bitmap *
bitmap_create(uint32_t width, uint32_t height, uint32_t color)
{
	struct bitmap *bmp;

	bmp = xmalloc(sizeof(struct bitmap));

	bmp->width = width;
	bmp->height = height;
	bmp->px = xmalloc(sizeof(uint32_t)*width*height);

	bitmap_clear(bmp, color);

	return bmp;
}

extern void
bitmap_set(struct bitmap *bmp, uint32_t x, uint32_t y, uint32_t color)
{
	if (x < bmp->width && y < bmp->height)
		bmp->px[y*bmp->width+x] = color;
}

extern uint32_t
bitmap_get(struct bitmap *bmp, uint32_t x, uint32_t y)
{
	if (x < bmp->width && y < bmp->height)
		return bmp->px[y*bmp->width+x];
	return 0;
}

extern void
bitmap_rect(struct bitmap *bmp, uint32_t x, uint32_t y,
            uint32_t width, uint32_t height, uint32_t color)
{
	uint32_t i, j;

	for (i = 0; i < width; ++i)
		for (j = 0; j < height; ++j)
			bmp->px[(j+y)*bmp->width+i+x] = color;
}

extern void
bitmap_clear(struct bitmap *bmp, uint32_t color)
{
	uint32_t x, y;

	for (x = 0; x < bmp->width; ++x)
		for (y = 0; y < bmp->height; ++y)
			bmp->px[y*bmp->width+x] = color;
}

extern void
bitmap_free(struct bitmap *bmp)
{
	free(bmp->px);
	free(bmp);
}
