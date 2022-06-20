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

#ifndef __XPAVM_X11_WINDOW_H__
#define __XPAVM_X11_WINDOW_H__

#include <stdint.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>

#include "../base/bitmap.h"

typedef void (*window_key_press_callback_t)(uint32_t key);
typedef struct window window_t;

struct window {
	xcb_window_t id;
	xcb_connection_t *connection;
	xcb_screen_t *screen;
	xcb_gcontext_t gc;
	xcb_image_t *image;
	bitmap_t *bmp;
	uint32_t running;
	window_key_press_callback_t key_pressed;
};

extern window_t *
window_create(const char *title, const char *class);

extern void
window_loop_start(window_t *window);

extern void
window_loop_end(window_t *window);

extern void
window_force_redraw(window_t *window);

extern void
window_set_key_press_callback(window_t *window, window_key_press_callback_t cb);

extern void
window_free(window_t *window);

#endif
