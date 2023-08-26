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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

#include "../base/bitmap.h"
#include "../util/debug.h"
#include "../util/xmalloc.h"
#include "window.h"

static xcb_atom_t
xatom(xcb_connection_t *conn, const char *name)
{
	xcb_atom_t atom;
	xcb_generic_error_t *error;
	xcb_intern_atom_cookie_t cookie;
	xcb_intern_atom_reply_t *reply;

	cookie = xcb_intern_atom(conn, 0, strlen(name), name);
	reply = xcb_intern_atom_reply(conn, cookie, &error);

	if (NULL != error)
		die("xcb_intern_atom failed with error code: %d",
				(int)(error->error_code));

	atom = reply->atom;
	free(reply);

	return atom;
}

static void
window_get_size(xcb_connection_t *conn, xcb_window_t wid,
                uint16_t *width, uint16_t *height)
{
	xcb_get_geometry_cookie_t cookie;
	xcb_get_geometry_reply_t *reply;
	xcb_generic_error_t *error;

	cookie = xcb_get_geometry(conn, wid);
	reply = xcb_get_geometry_reply(conn, cookie, &error);

	if (NULL != error)
		die("xcb_get_geometry failed with error code: %d",
				(int)(error->error_code));

	*width = reply->width;
	*height = reply->height;

	free(reply);
}

static void
window_set_fullscreen(xcb_connection_t *conn, xcb_window_t wid)
{
	xcb_change_property(
		conn, XCB_PROP_MODE_REPLACE, wid,
		xatom(conn, "_NET_WM_STATE"), XCB_ATOM_ATOM, 32, 1,
		(const xcb_atom_t []) { xatom(conn, "_NET_WM_STATE_FULLSCREEN") }
	);
}

static void
window_enable_wm_delete_window(xcb_connection_t *conn, xcb_window_t wid)
{
	xcb_change_property(
		conn, XCB_PROP_MODE_REPLACE, wid,
		xatom(conn, "WM_PROTOCOLS"), XCB_ATOM_ATOM, 32, 1,
		(const xcb_atom_t []) { xatom(conn, "WM_DELETE_WINDOW") }
	);
}

static void
window_set_wm_name(xcb_connection_t *conn, xcb_window_t wid, const char *title)
{
	size_t prop_size;

	prop_size = strlen(title);

	xcb_change_property(
		conn, XCB_PROP_MODE_REPLACE, wid,
		XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, prop_size, title
	);
}

static void
window_set_wm_class(xcb_connection_t *conn, xcb_window_t wid,
                    const char *iname, const char *cname)
{
	/* set instance and class name */
	/* https://x.org/releases/X11R7.6/doc/xorg-docs/specs/ICCCM/icccm.html */
	size_t prop_size;
	char prop[256];

	prop_size = strlen(iname) + strlen(cname) + 2;
	sprintf(prop, "%s%c%s", iname, '\0', cname);

	xcb_change_property(
		conn, XCB_PROP_MODE_REPLACE, wid,
		XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8, prop_size, prop
	);
}

extern struct window *
window_create(const char *title, const char *class)
{
	xcb_connection_t *conn;
	xcb_screen_t *screen;
	xcb_window_t wid;
	xcb_key_symbols_t *ksyms;
	xcb_gcontext_t gc;
	xcb_image_t *image;
	struct bitmap *bmp;
	struct window *window;

	if (xcb_connection_has_error((conn = xcb_connect(NULL, NULL))))
		die("can't open display");

	if (NULL == (screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data))
		die("can't get default screen");

	ksyms = xcb_key_symbols_alloc(conn);
	wid = xcb_generate_id(conn);
	gc = xcb_generate_id(conn);
	bmp = bitmap_create(screen->width_in_pixels, screen->height_in_pixels, 0);

	image = xcb_image_create_native(
		conn, bmp->width, bmp->height, XCB_IMAGE_FORMAT_Z_PIXMAP,
		screen->root_depth, NULL, 4 * bmp->width * bmp->height,
		(uint8_t *)(bmp->px)
	);

	xcb_create_window_aux(
		conn, XCB_COPY_FROM_PARENT, wid, screen->root, 0, 0, bmp->width, bmp->height,
		0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, XCB_CW_EVENT_MASK,
		(const xcb_create_window_value_list_t []) {{
			.event_mask = XCB_EVENT_MASK_EXPOSURE |
			              XCB_EVENT_MASK_KEY_PRESS |
			              XCB_EVENT_MASK_KEY_RELEASE
		}}
	);

	xcb_create_gc(conn, gc, wid, 0, NULL);

	window_set_wm_name(conn, wid, title);
	window_set_wm_class(conn, wid, class, class);
	window_set_fullscreen(conn, wid);
	window_enable_wm_delete_window(conn, wid);

	xcb_map_window(conn, wid);
	xcb_flush(conn);

	window = xmalloc(sizeof(struct window));

	window->running = 0;
	window->connection = conn;
	window->screen = screen;
	window->ksyms = ksyms;
	window->id = wid;
	window->image = image;
	window->bmp = bmp;
	window->gc = gc;
	window->key_pressed = NULL;

	return window;
}

static void
h_client_message(struct window *window, xcb_client_message_event_t *ev)
{
	/* check if the wm sent a delete window message */
	/* https://www.x.org/docs/ICCCM/icccm.pdf */
	if (ev->data.data32[0] == xatom(window->connection, "WM_DELETE_WINDOW"))
		window_loop_end(window);

}

static void
h_expose(struct window *window, xcb_expose_event_t *ev)
{
	uint16_t width, height;

	(void) ev;

	window_get_size(window->connection, window->id, &width, &height);

	xcb_image_put(
		window->connection, window->id, window->gc, window->image,
		(width - window->bmp->width) / 2, (height - window->bmp->height) / 2, 0
	);
}

static void
h_key_press(struct window *window, xcb_key_press_event_t *ev)
{
	xcb_keysym_t keysym;

	keysym = xcb_key_symbols_get_keysym(window->ksyms, ev->detail,
			ev->state & XCB_MOD_MASK_SHIFT);

	if (NULL != window->key_pressed)
		window->key_pressed(keysym);
}

static void
h_mapping_notify(struct window *window, xcb_mapping_notify_event_t *ev)
{
	if (ev->count > 0)
		xcb_refresh_keyboard_mapping(window->ksyms, ev);
}

extern void
window_loop_start(struct window *window)
{
	xcb_generic_event_t *ev;

	window->running = 1;

	while (window->running) {
		if ((ev = xcb_wait_for_event(window->connection))) {
			switch (ev->response_type & ~0x80) {
				case XCB_CLIENT_MESSAGE:   h_client_message(window, (void *)(ev)); break;
				case XCB_EXPOSE:           h_expose(window, (void *)(ev)); break;
				case XCB_KEY_PRESS:        h_key_press(window, (void *)(ev)); break;
				case XCB_MAPPING_NOTIFY:   h_mapping_notify(window, (void *)(ev)); break;
			}

			free(ev);
		}
	}
}

extern void
window_loop_end(struct window *window)
{
	window->running = 0;
}

extern void
window_force_redraw(struct window *window)
{
	xcb_clear_area(window->connection, 1, window->id, 0, 0, 0, 0);
	xcb_flush(window->connection);
}

extern void
window_set_key_press_callback(struct window *window, window_key_press_callback_t cb)
{
	window->key_pressed = cb;
}

extern void
window_free(struct window *window)
{
	xcb_key_symbols_free(window->ksyms);
	xcb_free_gc(window->connection, window->gc);
	xcb_disconnect(window->connection);
	xcb_image_destroy(window->image);
	bitmap_free(window->bmp);
	free(window);
}
