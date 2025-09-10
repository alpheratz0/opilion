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

#include <pixman.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <stdint.h>

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

typedef struct Pixbuf Pixbuf_t;

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

extern Pixbuf_t *
pixbuf_new(xcb_connection_t *conn, xcb_drawable_t container, int w, int h);

extern int
pixbuf_get_width(const Pixbuf_t *pb);

extern int
pixbuf_get_height(const Pixbuf_t *pb);

extern pixman_image_t *
pixbuf_get_pixman_image(const Pixbuf_t *pb);

extern void
pixbuf_set_container_size(Pixbuf_t *pb, int cw, int ch);

extern void
pixbuf_rect(Pixbuf_t *pb, int x, int y, int w, int h, uint32_t color);

extern void
pixbuf_fade(Pixbuf_t *pb, int y0, int y1, uint32_t fade_color);

extern void
pixbuf_clear(Pixbuf_t *pb, uint32_t color);

extern void
pixbuf_render(const Pixbuf_t *pb);

extern void
pixbuf_free(Pixbuf_t *pb);
