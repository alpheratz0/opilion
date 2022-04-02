#include <stdlib.h>
#include <string.h>
#include <freetype/freetype.h>
#include <fontconfig/fontconfig.h>

#include "../util/debug.h"
#include "../util/numdef.h"
#include "font.h"

static char *
font_search(const char *family) {
	FcPattern *pattern;
	FcPattern *match;
	FcResult result;
	char *path = NULL;

	if ((pattern = FcNameParse((const FcChar8 *)(family)))) {
		FcConfigSubstitute(0, pattern, FcMatchPattern);
		FcDefaultSubstitute(pattern);

		if ((match = FcFontMatch(0, pattern, &result))) {
			FcValue v;

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

extern font_t *
font_load(const char *family, u32 size) {
	char *path;
	font_t *font;

	if (!(path = font_search(family))) {
		dief("font family not found: %s", family);
	}

	if (!(font = malloc(sizeof(font_t)))) {
		die("error while calling malloc, no memory available");
	}

	if (FT_Init_FreeType(&font->library)) {
		die("error while calling FT_Init_FreeType()");
	}

	if (FT_New_Face(font->library, path, 0, &(font->face))) {
		die("error while calling FT_New_Face()");
	}

	if (FT_Set_Char_Size(font->face, 0, size * 64, 72, 72)) {
		die("error while calling FT_Set_Char_Size()");
	}

	if (FT_Load_Glyph(font->face, FT_Get_Char_Index(font->face, '0'), 0)) {
		die("error while calling FT_Load_Glyph()");
	}

	if (FT_Render_Glyph(font->face->glyph, FT_RENDER_MODE_NORMAL)) {
		die("error while calling FT_Render_Glyph()");
	}

	font->size = size;
	font->width = font->face->glyph->advance.x >> 6;
	font->height = (font->face->size->metrics.ascender - font->face->size->metrics.descender) >> 6;

	free(path);

	return font;
}

extern FT_GlyphSlot
font_get_glyph(font_t *font, char c) {
	if (FT_Load_Glyph(font->face, FT_Get_Char_Index(font->face, c), 0)) {
		die("error while calling FT_Load_Glyph()");
	}

	if (FT_Render_Glyph(font->face->glyph, FT_RENDER_MODE_NORMAL)) {
		die("error while calling FT_Render_Glyph()");
	}

	return font->face->glyph;
}

extern void
font_unload(font_t *font) {
	FT_Done_FreeType(font->library);
	free(font);
}
