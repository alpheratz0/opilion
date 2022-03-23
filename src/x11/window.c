#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xproto.h>
#include "../base/bitmap.h"
#include "../util/numdef.h"
#include "../util/debug.h"
#include "window.h"

extern window_t *
window_create(const char *title) {
	window_t *window;

	window = malloc(sizeof(window_t));
	if (!window)
		die("error while calling malloc, no memory available");

	window->connection = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(window->connection))
		die("can't open display");

	window->screen = xcb_setup_roots_iterator(xcb_get_setup(window->connection)).data;
	if (!window->screen) {
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
		XCB_ATOM_STRING, 8, strlen(title), title
	);

	xcb_change_property(
		window->connection, XCB_PROP_MODE_REPLACE, window->id, XCB_ATOM_WM_CLASS,
		XCB_ATOM_STRING, 8, strlen(title), title
	);

	xcb_change_window_attributes(
		window->connection, window->id, XCB_CW_OVERRIDE_REDIRECT,
		(const u32[1]) { 0x1 }
	);

	xcb_map_window(window->connection, window->id);
	xcb_set_input_focus(window->connection, XCB_INPUT_FOCUS_POINTER_ROOT, window->id, XCB_CURRENT_TIME);
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
	window->running = 1;

	while (window->running) {
		if ((ev = xcb_wait_for_event(window->connection))) {
			switch (ev->response_type & ~0x80) {
				case XCB_EXPOSE:
					xcb_image_put(window->connection, window->id, window->gc, window->image, 0, 0, 0);
					break;
				case XCB_KEY_PRESS:
					window->key_pressed(((xcb_key_press_event_t *)(ev))->detail);
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
