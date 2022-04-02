#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xproto.h>

#include "../base/bitmap.h"
#include "../util/numdef.h"
#include "../util/debug.h"
#include "window.h"

static xcb_atom_t
x11_get_atom(window_t *window, const char *name) {
	xcb_atom_t atom;
	xcb_intern_atom_reply_t *reply;

	reply = xcb_intern_atom_reply(
		window->connection,
		xcb_intern_atom_unchecked(
			window->connection, 1,
			strlen(name), name
		),
		NULL
	);

	atom = reply->atom;
	free(reply);

	return atom;
}

extern window_t *
window_create(const char *wm_name, const char *wm_class) {
	window_t *window;

	if (!(window = malloc(sizeof(window_t)))) {
		die("error while calling malloc, no memory available");
	}

	if (xcb_connection_has_error((window->connection = xcb_connect(NULL, NULL)))) {
		die("can't open display");
	}

	if (!(window->screen = xcb_setup_roots_iterator(xcb_get_setup(window->connection)).data)) {
		xcb_disconnect(window->connection);
		die("can't get default screen");
	}

	window->id = xcb_generate_id(window->connection);
	window->bmp = bitmap_create(window->screen->width_in_pixels, window->screen->height_in_pixels, 0x000000);

	xcb_void_cookie_t window_cookie = xcb_create_window_checked(
		window->connection, XCB_COPY_FROM_PARENT, window->id,
		window->screen->root, 0, 0, window->bmp->width, window->bmp->height, 0,
		XCB_WINDOW_CLASS_INPUT_OUTPUT, window->screen->root_visual,
		XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
		(const u32[2]) {
			0x000000,
			XCB_EVENT_MASK_EXPOSURE |
			XCB_EVENT_MASK_KEY_PRESS |
			XCB_EVENT_MASK_KEY_RELEASE
		}
	);

	if (xcb_request_check(window->connection, window_cookie)) {
		xcb_disconnect(window->connection);
		die("can't create window");
	}

	xcb_change_property(
		window->connection, XCB_PROP_MODE_REPLACE, window->id, XCB_ATOM_WM_NAME,
		XCB_ATOM_STRING, 8, strlen(wm_name), wm_name
	);

	/* set instance and class name */
	/* see: https://x.org/releases/X11R7.6/doc/xorg-docs/specs/ICCCM/icccm.html */
	size_t class_size = sizeof(char) * (2 + strlen(wm_class) * 2);
	char *class = malloc(class_size);
	sprintf(class, "%s%c%s", wm_class, '\0', wm_class);

	xcb_change_property(
		window->connection,
		XCB_PROP_MODE_REPLACE,
		window->id,
		XCB_ATOM_WM_CLASS,
		XCB_ATOM_STRING, 8, class_size,
		class
	);

	free(class);

	/* set fullscreen */
	xcb_atom_t net_wm_state = x11_get_atom(window, "_NET_WM_STATE");
	xcb_atom_t net_wm_state_fullscreen = x11_get_atom(window, "_NET_WM_STATE_FULLSCREEN");

	xcb_change_property(
		window->connection,
		XCB_PROP_MODE_REPLACE,
		window->id,
		net_wm_state,
		XCB_ATOM_ATOM, 32, 1,
		&net_wm_state_fullscreen
	);

	/* set wm protocols */
	xcb_atom_t wm_protocols = x11_get_atom(window, "WM_PROTOCOLS");
	xcb_atom_t wm_delete_window = x11_get_atom(window, "WM_DELETE_WINDOW");

	xcb_change_property(
		window->connection,
		XCB_PROP_MODE_REPLACE,
		window->id,
		wm_protocols,
		XCB_ATOM_ATOM, 32, 1,
		&wm_delete_window
	);

	xcb_map_window(window->connection, window->id);
	xcb_flush(window->connection);

	window->running = 0;
	window->gc = xcb_generate_id(window->connection);
	window->image = xcb_image_create_native(
		window->connection, window->bmp->width, window->bmp->height,
		XCB_IMAGE_FORMAT_Z_PIXMAP, window->screen->root_depth,
		window->bmp->px, 4*window->bmp->width*window->bmp->height, (u8 *)(window->bmp->px)
	);

	xcb_create_gc(window->connection, window->gc, window->id, 0, 0);

	return window;
}

extern void
window_loop_start(window_t *window) {
	xcb_generic_event_t *ev;
	xcb_key_press_event_t *kpev;
	xcb_client_message_event_t *cmev;

	window->running = 1;

	while (window->running) {
		if ((ev = xcb_wait_for_event(window->connection))) {
			switch (ev->response_type & ~0x80) {
				case XCB_CLIENT_MESSAGE:
					cmev = (xcb_client_message_event_t *)(ev);

					/* end loop if the window manager sent a delete window message */
					/* https://www.x.org/docs/ICCCM/icccm.pdf */
					if (cmev->data.data32[0] == x11_get_atom(window, "WM_DELETE_WINDOW")) {
						window_loop_end(window);
					}

					break;
				case XCB_EXPOSE:
					xcb_image_put(window->connection, window->id, window->gc, window->image, 0, 0, 0);
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
window_loop_end(window_t *window) {
	window->running = 0;
}

extern void
window_force_redraw(window_t *window) {
	xcb_clear_area(window->connection, 1, window->id, 0, 0, 1, 1);
	xcb_flush(window->connection);
}

extern void
window_set_key_press_callback(window_t *window, window_key_press_callback_t cb) {
	window->key_pressed = cb;
}

extern void
window_free(window_t *window) {
	xcb_free_gc(window->connection, window->gc);
	xcb_disconnect(window->connection);
	bitmap_free(window->bmp);
	free(window->image);
	free(window);
}
