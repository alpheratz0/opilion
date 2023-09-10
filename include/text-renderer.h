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

#pragma once

#include "pixbuf.h"

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

typedef struct TextRenderer TextRenderer_t;

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

extern TextRenderer_t *
text_renderer_new(const char *font_family, int size);

extern int
text_renderer_draw_char(TextRenderer_t *tr, Pixbuf_t *pb, char c, int x, int y, uint32_t color);

extern int
text_renderer_draw_string(TextRenderer_t *tr, Pixbuf_t *pb, const char *str, int x, int y, uint32_t color);

extern int
text_renderer_text_width(const TextRenderer_t *tr, const char *str);

extern int
text_renderer_text_height(const TextRenderer_t *tr);

extern void
text_renderer_free(TextRenderer_t *tr);
