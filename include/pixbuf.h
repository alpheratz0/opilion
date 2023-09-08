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

#include <xcb/xcb.h>

typedef struct Pixbuf Pixbuf_t;

extern Pixbuf_t *
pixbuf_new(xcb_connection_t *conn, xcb_window_t win, int w, int h);

extern int
pixbuf_get_width(Pixbuf_t *pb);

extern int
pixbuf_get_height(Pixbuf_t *pb);

extern int
pixbuf_get(Pixbuf_t *pb, int x, int y, uint32_t *color);

extern void
pixbuf_set(Pixbuf_t *pb, int x, int y, uint32_t color);

extern void
pixbuf_set_viewport(Pixbuf_t *pb, int vw, int vh);

extern void
pixbuf_rect(Pixbuf_t *pb, int x, int y, int w, int h, uint32_t color);

extern void
pixbuf_clear(Pixbuf_t *pb, uint32_t color);

extern void
pixbuf_render(Pixbuf_t *pb);

extern void
pixbuf_free(Pixbuf_t *pb);
