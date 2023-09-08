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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>

#include "log.h"
#include "text-renderer.h"
#include "pixbuf.h"
#include "utils.h"
#include "color.h"

#define CHKFTERR(name,err) do {                                \
	FT_Error error;                                            \
	error = (err);                                             \
	if (error != 0)                                            \
		die(name " failed with error code: %d", (int)(error)); \
} while (0)

struct TextRenderer {
	FT_Library library;
	FT_Face face;
	int size;
	int height;
	int width;
};

static FT_GlyphSlot
__get_glyph(TextRenderer_t *tr, char c)
{
	CHKFTERR("FT_Load_Char", FT_Load_Char(tr->face, c, FT_LOAD_RENDER));
	return tr->face->glyph;
}

static char *
__query_font(const char *family)
{
	FcPattern *pattern;
	FcPattern *match;
	FcResult result;
	FcValue v;
	char *path = NULL;

	if ((pattern = FcNameParse((const FcChar8 *)(family)))) {
		FcConfigSubstitute(0, pattern, FcMatchPattern);
		FcDefaultSubstitute(pattern);

		if ((match = FcFontMatch(0, pattern, &result))) {
			FcPatternGet(match, FC_FAMILY, 0, &v);

			if (strcmp(family, (char *)(v.u.s)) == 0) {
				FcPatternGet(match, FC_FILE, 0, &v);
				path = xstrdup((char *)(v.u.s));
			}

			FcPatternDestroy(match);
		}

		FcPatternDestroy(pattern);
		FcFini();
	}

	return path;
}

extern TextRenderer_t *
text_renderer_new(const char *font_family, int size)
{
	char *path;
	FT_Library lib;
	FT_Face face;
	TextRenderer_t *tr;

	if (NULL == (path = __query_font(font_family)))
		die("font family not found: %s", font_family);

	CHKFTERR("FT_Init_FreeType", FT_Init_FreeType(&lib));
	CHKFTERR("FT_New_Face", FT_New_Face(lib, path, 0, &face));
	CHKFTERR("FT_Set_Char_Size", FT_Set_Char_Size(face, 0, size * 64, 72, 72));
	CHKFTERR("FT_Load_Char", FT_Load_Char(face, '0', FT_LOAD_RENDER));

	tr = xmalloc(sizeof(TextRenderer_t));

	tr->library = lib;
	tr->face = face;
	tr->size = size;
	tr->width = face->glyph->advance.x >> 6;
	tr->height = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;

	free(path);

	return tr;
}

extern void
text_renderer_draw_char(TextRenderer_t *tr, Pixbuf_t *pb, char c,
		int x, int y, uint32_t color)
{
	FT_GlyphSlot glyph;
	uint32_t width, height, xmap, ymap, gray, cx, cy;
	uint32_t prev_color;

	glyph = __get_glyph(tr, c);
	height = glyph->bitmap.rows;
	width = glyph->bitmap.width;

	for (cx = 0; cx < height; ++cx) {
		for (cy = 0; cy < width; ++cy) {
			xmap = x + cy + glyph->bitmap_left;
			ymap = y + cx - glyph->bitmap_top + tr->size;
			gray = glyph->bitmap.buffer[cx*width+cy];

			if (pixbuf_get(pb, xmap, ymap, &prev_color)) {
				pixbuf_set(pb, xmap, ymap,
						color_mix(prev_color, color, gray));
			}
		}
	}
}

extern void
text_renderer_draw_string(TextRenderer_t *tr, Pixbuf_t *pb, const char *str,
		int x, int y, uint32_t color)
{
	size_t i, len;

	len = strlen(str);

	for (i = 0; i < len && str[i] != '\n'; ++i) {
		text_renderer_draw_char(tr, pb, str[i],
				x+i*tr->width, y, color);
	}
}

extern int
text_renderer_text_width(TextRenderer_t *tr, const char *str)
{
	return strlen(str) * tr->width;
}

extern int
text_renderer_text_height(TextRenderer_t *tr)
{
	return tr->height;
}

extern void
text_renderer_free(TextRenderer_t *tr)
{
	FT_Done_FreeType(tr->library);
	free(tr);
}
