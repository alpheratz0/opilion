/*
	Copyright (C) 2022 <alpheratz99@protonmail.com>

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

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>

#include "../util/debug.h"
#include "../util/xmalloc.h"
#include "font.h"

#define CHKFTERR(name,err) do {                                 \
	FT_Error error;                                             \
	error = (err);                                              \
	if (error != 0)                                             \
		die(name " failed with error code: %d", (int)(error)); \
} while (0)

static char *
font_search(const char *family)
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
				path = strdup((char *)(v.u.s));
			}

			FcPatternDestroy(match);
		}

		FcPatternDestroy(pattern);
		FcFini();
	}

	return path;
}

extern struct font *
font_load(const char *family, uint32_t size)
{
	char *path;
	FT_Library lib;
	FT_Face face;
	struct font *font;

	if (NULL == (path = font_search(family)))
		die("font family not found: %s", family);

	CHKFTERR("FT_Init_FreeType", FT_Init_FreeType(&lib));
	CHKFTERR("FT_New_Face", FT_New_Face(lib, path, 0, &face));
	CHKFTERR("FT_Set_Char_Size", FT_Set_Char_Size(face, 0, size * 64, 72, 72));
	CHKFTERR("FT_Load_Char", FT_Load_Char(face, '0', FT_LOAD_RENDER));

	font = xmalloc(sizeof(struct font));

	font->library = lib;
	font->face = face;
	font->size = size;
	font->width = face->glyph->advance.x >> 6;
	font->height = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;

	free(path);

	return font;
}

extern FT_GlyphSlot
font_get_glyph(struct font *font, char c)
{
	CHKFTERR("FT_Load_Char", FT_Load_Char(font->face, c, FT_LOAD_RENDER));

	return font->face->glyph;
}

extern void
font_unload(struct font *font)
{
	FT_Done_FreeType(font->library);
	free(font);
}
