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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xproto.h>

#include "../base/bitmap.h"
#include "../util/debug.h"
#include "window.h"

static xcb_atom_t
x11_get_atom(xcb_connection_t *conn, const char *name)
{
	xcb_atom_t atom;
	xcb_intern_atom_reply_t *reply;

	reply = xcb_intern_atom_reply(
		conn,
		xcb_intern_atom_unchecked(
			conn, 1, strlen(name), name
		),
		NULL
	);

	atom = reply->atom;
	free(reply);

	return atom;
}

static void
window_set_fullscreen(xcb_connection_t *conn, xcb_window_t wid)
{
	xcb_atom_t net_wm_state, net_wm_state_fullscreen;

	net_wm_state = x11_get_atom(conn, "_NET_WM_STATE");
	net_wm_state_fullscreen = x11_get_atom(conn, "_NET_WM_STATE_FULLSCREEN");

	xcb_change_property(
		conn, XCB_PROP_MODE_REPLACE, wid,
		net_wm_state, XCB_ATOM_ATOM, 32, 1, &net_wm_state_fullscreen
	);
}

static void
window_enable_wm_delete_window(xcb_connection_t *conn, xcb_window_t wid)
{
	xcb_atom_t wm_protocols, wm_delete_window;

	wm_protocols = x11_get_atom(conn, "WM_PROTOCOLS");
	wm_delete_window = x11_get_atom(conn, "WM_DELETE_WINDOW");

	xcb_change_property(
		conn, XCB_PROP_MODE_REPLACE, wid,
		wm_protocols, XCB_ATOM_ATOM, 32, 1, &wm_delete_window
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
window_set_wm_class(xcb_connection_t *conn,
                    xcb_window_t wid,
                    const char *iname,
                    const char *cname)
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

extern window_t *
window_create(const char *title, const char *class)
{
	xcb_connection_t *conn;
	xcb_screen_t *screen;
	xcb_window_t wid;
	xcb_gcontext_t gc;
	xcb_image_t *image;
	bitmap_t *bmp;
	window_t *window;
	uint32_t evmask;

	if (xcb_connection_has_error((conn = xcb_connect(NULL, NULL)))) {
		die("can't open display");
	}

	if (NULL == (screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data)) {
		xcb_disconnect(conn);
		die("can't get default screen");
	}

	wid = xcb_generate_id(conn);
	gc = xcb_generate_id(conn);
	bmp = bitmap_create(screen->width_in_pixels, screen->height_in_pixels, 0);

	image = xcb_image_create_native(
		conn, bmp->width, bmp->height, XCB_IMAGE_FORMAT_Z_PIXMAP,
		screen->root_depth, bmp->px, 4 * bmp->width * bmp->height,
		(uint8_t *)(bmp->px)
	);

	evmask = XCB_EVENT_MASK_EXPOSURE |
	         XCB_EVENT_MASK_KEY_PRESS |
	         XCB_EVENT_MASK_KEY_RELEASE;

	xcb_create_window(
		conn, XCB_COPY_FROM_PARENT, wid, screen->root, 0, 0, bmp->width,
		bmp->height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
		XCB_CW_EVENT_MASK, &evmask
	);

	xcb_create_gc(conn, gc, wid, 0, 0);

	window_set_wm_name(conn, wid, title);
	window_set_wm_class(conn, wid, class, class);
	window_set_fullscreen(conn, wid);
	window_enable_wm_delete_window(conn, wid);

	xcb_map_window(conn, wid);
	xcb_flush(conn);

	if (NULL == (window = malloc(sizeof(window_t)))) {
		die("error while calling malloc, no memory available");
	}

	window->running = 0;
	window->connection = conn;
	window->screen = screen;
	window->id = wid;
	window->image = image;
	window->bmp = bmp;
	window->gc = gc;

	return window;
}

extern void
window_loop_start(window_t *window)
{
	xcb_generic_event_t *ev;
	xcb_key_press_event_t *kpev;
	xcb_client_message_event_t *cmev;
	xcb_atom_t atom;

	window->running = 1;

	while (window->running) {
		if ((ev = xcb_wait_for_event(window->connection))) {
			switch (ev->response_type & ~0x80) {
				case XCB_CLIENT_MESSAGE:
					cmev = (xcb_client_message_event_t *)(ev);
					atom = cmev->data.data32[0];

					/* check if the wm sent a delete window message */
					/* https://www.x.org/docs/ICCCM/icccm.pdf */
					if (atom == x11_get_atom(window->connection, "WM_DELETE_WINDOW")) {
						window_loop_end(window);
					}

					break;
				case XCB_EXPOSE:
					xcb_image_put(
						window->connection, window->id, window->gc,
						window->image, 0, 0, 0
					);

					break;
				case XCB_KEY_PRESS:
					kpev = (xcb_key_press_event_t *)(ev);
					window->key_pressed(kpev->detail);
					break;
				default:
					break;
			}

			free(ev);
		}
	}
}

extern void
window_loop_end(window_t *window)
{
	window->running = 0;
}

extern void
window_force_redraw(window_t *window)
{
	xcb_clear_area(window->connection, 1, window->id, 0, 0, 1, 1);
	xcb_flush(window->connection);
}

extern void
window_set_key_press_callback(window_t *window, window_key_press_callback_t cb)
{
	window->key_pressed = cb;
}

extern void
window_free(window_t *window)
{
	xcb_free_gc(window->connection, window->gc);
	xcb_disconnect(window->connection);
	bitmap_free(window->bmp);
	free(window->image);
	free(window);
}
