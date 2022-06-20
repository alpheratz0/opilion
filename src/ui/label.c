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

#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "../base/bitmap.h"
#include "../base/font.h"
#include "../util/debug.h"
#include "../util/color.h"
#include "label.h"

static void
label_render_char_onto(char c,
                       font_t *font,
                       uint32_t color,
                       uint32_t x,
                       uint32_t y,
                       bitmap_t *bmp)
{
	FT_GlyphSlot glyph;
	uint32_t width, height, xmap, ymap, gray;

	glyph = font_get_glyph(font, c);
	height = glyph->bitmap.rows;
	width = glyph->bitmap.width;

	for (uint32_t i = 0; i < height; ++i) {
		for (uint32_t j = 0; j < width; ++j) {
			xmap = x + j + glyph->bitmap_left;
			ymap = y + i - glyph->bitmap_top + font->size;
			gray = glyph->bitmap.buffer[i*width+j];

			bitmap_set(
				bmp, xmap, ymap,
				color_lerp(
					bitmap_get(bmp, xmap, ymap),
					color, gray
				)
			);
		}
	}
}

extern void
label_render_onto(const char *text,
                  font_t *font,
                  uint32_t color,
                  uint32_t x,
                  uint32_t y,
                  bitmap_t *bmp)
{
	size_t len;

	len = strlen(text);

	for (size_t i = 0; i < len && text[i] != '\n'; ++i) {
		label_render_char_onto(text[i], font, color, x + i * font->width, y, bmp);
	}
}
