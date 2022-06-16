#ifndef __XPAVM_BASE_FONT_H__
#define __XPAVM_BASE_FONT_H__

#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct font font_t;

struct font {
	FT_Library library;
	FT_Face face;
	uint32_t size;
	uint32_t height;
	uint32_t width;
};

extern font_t *
font_load(const char *family, uint32_t size);

extern FT_GlyphSlot
font_get_glyph(font_t *font, char c);

extern void
font_unload(font_t *font);

#endif
