/*
	Copyright (C) 2022-2023 <alpheratz99@protonmail.com>

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

#include <pixman.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcft/fcft.h>
#include <string.h>
#include <ctype.h>

#include "log.h"
#include "text-renderer.h"
#include "pixbuf.h"
#include "utils.h"

#define LEN(arr) (sizeof(arr)/sizeof(arr[0]))

struct TextRenderer {
	struct fcft_font *font;
	struct {
		uint32_t color;
		pixman_color_t pcolor;
		pixman_image_t *image;
	} colors[16];
};

static pixman_color_t
__pixman_color_from_uint32(uint32_t c)
{
	pixman_color_t pc;
	pc.red = ((c>>16) & 0xff) * 257;
	pc.green = ((c>>8) & 0xff) * 257;
	pc.blue = ((c>>0) & 0xff) * 257;
	return pc;
}

static pixman_image_t *
__text_renderer_get_image_for_color(TextRenderer_t *tr, uint32_t c)
{
	size_t i;

	for (i = 0; i < LEN(tr->colors); ++i) {
		if (NULL == tr->colors[i].image) break;
		if (c != tr->colors[i].color) continue;
		return tr->colors[i].image;
	}

	if (i == LEN(tr->colors))
		return NULL;

	tr->colors[i].color = c;
	tr->colors[i].pcolor = __pixman_color_from_uint32(c);
	tr->colors[i].image = pixman_image_create_solid_fill(&tr->colors[i].pcolor);

	return tr->colors[i].image;
}

extern TextRenderer_t *
text_renderer_new(const char *font_family, int size)
{
	TextRenderer_t *tr;
	char font_query[128];

	snprintf(font_query, sizeof(font_query), "%s:size=%d", font_family, size);

	fcft_init(FCFT_LOG_COLORIZE_ALWAYS, false, FCFT_LOG_CLASS_NONE);

	if (!fcft_set_scaling_filter(FCFT_SCALING_FILTER_LANCZOS3))
		die("fcft_set_scaling_filter failed");

	tr = xcalloc(1, sizeof(TextRenderer_t));

	tr->font = fcft_from_name(1, (const char *[]){font_query}, NULL);

	if (NULL == tr->font)
		die("fcft_from_name couldn't load font: %s:size=%d", font_family, size);

	fcft_set_emoji_presentation(tr->font, FCFT_EMOJI_PRESENTATION_DEFAULT);

	return tr;
}

extern int
text_renderer_draw_char(TextRenderer_t *tr, Pixbuf_t *pb, char c,
		int x, int y, uint32_t color)
{
	pixman_image_t *color_img;
	const struct fcft_glyph *g;

	g = fcft_rasterize_char_utf32(tr->font, c, FCFT_SUBPIXEL_DEFAULT);

	if (NULL == g ||
			pixman_image_get_format(g->pix) == PIXMAN_a8r8g8b8)
		return 0;

	color_img = __text_renderer_get_image_for_color(tr, color);

	pixman_image_composite32(
			PIXMAN_OP_OVER, color_img, g->pix, pixbuf_get_pixman_image(pb), 0, 0, 0, 0,
			x + g->x, y + tr->font->ascent - g->y, g->width, g->height);

	return g->advance.x;
}

extern int
text_renderer_draw_string(TextRenderer_t *tr, Pixbuf_t *pb, const char *str,
		int x, int y, uint32_t color)
{
	size_t i, len;
	int w;

	w = 0;
	len = strlen(str);

	for (i = 0; i < len && str[i] != '\n'; ++i) {
		w += text_renderer_draw_char(tr, pb, str[i],
				x+w, y, color);
	}

	return w;
}

extern int
text_renderer_text_width(const TextRenderer_t *tr, const char *str)
{
	int width;
	const struct fcft_glyph *g;

	width = 0;

	while (*str) {
		g = fcft_rasterize_char_utf32(tr->font, *str++, FCFT_SUBPIXEL_DEFAULT);
		if (NULL == g || pixman_image_get_format(g->pix) == PIXMAN_a8r8g8b8) continue;
		width += g->advance.x;
	}

	return width;
}

extern int
text_renderer_text_height(const TextRenderer_t *tr)
{
	return tr->font->height;
}

extern void
text_renderer_free(TextRenderer_t *tr)
{
	size_t i;
	for (i = 0; i < LEN(tr->colors); ++i) {
		if (NULL != tr->colors[i].image)
			pixman_image_unref(tr->colors[i].image);
	}
	fcft_fini();
	free(tr);
}
