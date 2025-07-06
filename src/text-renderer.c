/*
	Copyright (C) 2022-2025 <alpheratz99@protonmail.com>

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
#include <grapheme.h>

#include "log.h"
#include "pixman-image-cache.h"
#include "text-renderer.h"
#include "pixbuf.h"
#include "utils.h"

struct TextRenderer {
	struct fcft_font_options *font_options;
	struct fcft_font *font;
};

extern TextRenderer_t *
text_renderer_new(const char *font_family, int size)
{
	TextRenderer_t *tr;
	char font_query_main[128], font_query_fallback[128];

	snprintf(font_query_main, sizeof(font_query_main), "%s:size=%d",
			font_family, size);

	snprintf(font_query_fallback, sizeof(font_query_fallback), "%s:size=%d",
			"Noto Color Emoji", size);

	fcft_init(FCFT_LOG_COLORIZE_ALWAYS, false, FCFT_LOG_CLASS_NONE);

	tr = xcalloc(1, sizeof(TextRenderer_t));

	tr->font_options = fcft_font_options_create();;
	tr->font_options->scaling_filter = FCFT_SCALING_FILTER_LANCZOS3;
	tr->font_options->emoji_presentation = FCFT_EMOJI_PRESENTATION_DEFAULT;
	tr->font = fcft_from_name2(2, (const char *[]){font_query_main, font_query_fallback}, NULL, tr->font_options);

	if (NULL == tr->font) {
		die("fcft_from_name couldn't load font: %s:size=%d",
				font_family, size);
	}

	return tr;
}

extern int
text_renderer_draw_char(TextRenderer_t *tr, Pixbuf_t *pb, uint32_t cp,
		int x, int y, uint32_t color)
{
	pixman_image_t *color_img;
	const struct fcft_glyph *g;

	g = fcft_rasterize_char_utf32(tr->font, cp, FCFT_SUBPIXEL_DEFAULT);

	if (NULL == g)
		return 0;

	switch (pixman_image_get_format(g->pix)) {
	case PIXMAN_a8r8g8b8:
		pixman_image_composite32(PIXMAN_OP_OVER, g->pix, NULL,
				pixbuf_get_pixman_image(pb), 0, 0, 0, 0, x + g->x,
				y + tr->font->ascent - g->y, g->width, g->height);
		break;
	default:
		color_img = pixman_image_create_solid_fill_cached(color);
		pixman_image_composite32(PIXMAN_OP_OVER, color_img, g->pix,
				pixbuf_get_pixman_image(pb), 0, 0, 0, 0, x + g->x,
				y + tr->font->ascent - g->y, g->width, g->height);
		break;
	}

	return g->advance.x;
}

extern int
text_renderer_draw_string(TextRenderer_t *tr, Pixbuf_t *pb, const char *str,
		int x, int y, uint32_t color)
{
	const char *walk = str;
	uint_least32_t cp = 0;
	size_t total_len = strlen(str);
	int width = 0;

	while (walk < str + total_len && *walk != '\n') {
		const char *old_walk = walk;
		walk += grapheme_decode_utf8(walk, total_len - (walk - str), &cp);
		if (walk == old_walk) break;
		width += text_renderer_draw_char(tr, pb, cp, x + width, y, color);
	}

	return width;
}

extern int
text_renderer_char_width(const TextRenderer_t *tr, char c)
{
	const struct fcft_glyph *glyph;

	glyph = fcft_rasterize_char_utf32(tr->font, c, FCFT_SUBPIXEL_DEFAULT);

	if (!glyph || pixman_image_get_format(glyph->pix) == PIXMAN_a8r8g8b8)
		return 0;

	return glyph->advance.x;
}

extern int
text_renderer_text_width(const TextRenderer_t *tr, const char *str)
{
	int width;
	for (width = 0; *str; ++str)
		width += text_renderer_char_width(tr, *str);
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
	fcft_font_options_destroy(tr->font_options);
	fcft_fini();
	free(tr);
}
