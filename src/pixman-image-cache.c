/*
	Copyright (C) 2023 <alpheratz99@protonmail.com>

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

#include <stddef.h>
#include <stdint.h>
#include <pixman.h>

#define CACHE_SIZE 16

#define RED(c) ((c>>16) & 0xff)
#define GREEN(c) ((c>>8) & 0xff)
#define BLUE(c) ((c>>0) & 0xff)

static int g_next_slot;
static struct {
	uint32_t color;
	pixman_image_t *image;
} g_cached[CACHE_SIZE];

extern pixman_image_t *
pixman_image_create_solid_fill_cached(uint32_t c)
{
	int i, slot;

	for (i = 0; i < CACHE_SIZE && NULL != g_cached[i].image; ++i)
		if (g_cached[i].color == c)
			return g_cached[i].image;

	if (g_cached[g_next_slot].image)
		pixman_image_unref(g_cached[g_next_slot].image);

	slot = g_next_slot;
	g_next_slot = (g_next_slot + 1) % CACHE_SIZE;

	g_cached[slot].color = c;

	g_cached[slot].image = pixman_image_create_solid_fill(
		&(const pixman_color_t) {
			.red = RED(c) * 257,
			.green = GREEN(c) * 257,
			.blue = BLUE(c) * 257,
			.alpha = 0xffff
		}
	);

	return g_cached[slot].image;
}
