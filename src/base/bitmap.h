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

#ifndef __XPAVM_BASE_BITMAP_H__
#define __XPAVM_BASE_BITMAP_H__

#include <stdint.h>

struct bitmap {
	uint32_t *px;
	uint32_t width;
	uint32_t height;
};

extern struct bitmap *
bitmap_create(uint32_t width, uint32_t height, uint32_t color);

extern void
bitmap_set(struct bitmap *bmp, uint32_t x, uint32_t y, uint32_t color);

extern uint32_t
bitmap_get(struct bitmap *bmp, uint32_t x, uint32_t y);

extern void
bitmap_rect(struct bitmap *bmp,
            uint32_t x,
            uint32_t y,
            uint32_t width,
            uint32_t height,
            uint32_t color);

extern void
bitmap_clear(struct bitmap *bmp, uint32_t color);

extern void
bitmap_free(struct bitmap *bmp);

#endif
