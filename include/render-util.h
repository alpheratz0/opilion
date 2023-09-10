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

#pragma once

#include <stdint.h>

#include "pixbuf.h"
#include "text-renderer.h"

extern void
render_util_render_slider(Pixbuf_t *pb, int x, int y, int w, int h, int pct,
		const uint32_t c[2]);

extern void
render_util_render_key_value_pair(Pixbuf_t *pb, int x, int y, int w,
		TextRenderer_t *tr, const char *key, uint32_t key_color,
		const char *value, uint32_t value_color);
