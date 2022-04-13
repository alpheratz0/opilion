#ifndef __XPAVM_BASE_FONT_H__
#define __XPAVM_BASE_FONT_H__

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../util/numdef.h"

typedef struct font font_t;

struct font {
	FT_Library library;
	FT_Face face;
	u32 size;
	u32 height;
	u32 width;
};

extern font_t *
font_load(const char *family, u32 size);

extern FT_GlyphSlot
font_get_glyph(font_t *font, char c);

extern void
font_unload(font_t *font);

#endif
