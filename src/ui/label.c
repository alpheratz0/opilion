#include <ft2build.h>
#include FT_FREETYPE_H

#include "../base/bitmap.h"
#include "../base/font.h"
#include "../util/debug.h"
#include "../util/color.h"
#include "../util/numdef.h"
#include "label.h"

static void
label_render_char_onto(bitmap_t *bmp, font_t *font, u32 color, char c, u32 x, u32 y) {
	FT_GlyphSlot glyph = font_get_glyph(font, c);
	u32 glyph_h = glyph->bitmap.rows;
	u32 glyph_w = glyph->bitmap.width;

	for (u32 i = 0; i < glyph_h; ++i) {
		for (u32 j = 0; j < glyph_w; ++j) {
			u32 xmap = x + j + glyph->bitmap_left;
			u32 ymap = y + i - glyph->bitmap_top + font->size;
			u8 gray = glyph->bitmap.buffer[i*glyph_w+j];

			bitmap_set(bmp, xmap, ymap, color_lerp(bitmap_get(bmp, xmap, ymap), color, gray));
		}
	}
}

extern void
label_render_onto(bitmap_t *bmp, font_t *font, u32 color, char *text, u32 x, u32 y) {
	for (size_t i = 0; i < strlen(text) && text[i] != '\n'; ++i) {
		label_render_char_onto(bmp, font, color, text[i], x + i * font->width, y);
	}
}
