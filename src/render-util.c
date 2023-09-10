/*
	Copyright (C) 2023 <alpheratz99@protonmail.com>

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

#include <stdint.h>

#include "pixbuf.h"
#include "utils.h"
#include "text-renderer.h"

extern void
render_util_render_slider(Pixbuf_t *pb, int x, int y, int w, int h, int pct,
		const uint32_t c[2])
{
	pixbuf_rect(pb, x, y, w, h, c[0]);
	pixbuf_rect(pb, x, y, (w*CLAMP(pct,0,100))/100, h, c[1]);
}

extern void
render_util_render_key_value_pair(Pixbuf_t *pb, int x, int y, int w,
		TextRenderer_t *tr, const char *key, uint32_t key_color,
		const char *value, uint32_t value_color)
{
	text_renderer_draw_string(tr, pb, key, x, y, key_color);
	text_renderer_draw_string(tr, pb, value,
			x + w - text_renderer_text_width(tr, value), y, value_color);
}
