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

#ifndef __XPAVM_BASE_FONT_H__
#define __XPAVM_BASE_FONT_H__

#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H

struct font {
	FT_Library library;
	FT_Face face;
	uint32_t size;
	uint32_t height;
	uint32_t width;
};

extern struct font *
font_load(const char *family, uint32_t size);

extern FT_GlyphSlot
font_get_glyph(struct font *font, char c);

extern void
font_unload(struct font *font);

#endif
