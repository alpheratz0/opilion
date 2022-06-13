#include <ft2build.h>
#include FT_FREETYPE_H

#include "../base/bitmap.h"
#include "../base/font.h"
#include "../util/debug.h"
#include "../util/color.h"
#include "../util/numdef.h"
#include "label.h"

static void
label_render_char_onto(bitmap_t *bmp, font_t *font, u32 color, char c, u32 x, u32 y)
{
	FT_GlyphSlot glyph;
	u32 width, height, xmap, ymap, gray;

	glyph = font_get_glyph(font, c);
	height = glyph->bitmap.rows;
	width = glyph->bitmap.width;

	for (u32 i = 0; i < height; ++i) {
		for (u32 j = 0; j < width; ++j) {
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
label_render_onto(bitmap_t *bmp, font_t *font, u32 color, char *text, u32 x, u32 y)
{
	size_t len;

	len = strlen(text);

	for (size_t i = 0; i < len && text[i] != '\n'; ++i) {
		label_render_char_onto(bmp, font, color, text[i], x + i * font->width, y);
	}
}
