#ifndef __XPAVM_X11_WINDOW_H__
#define __XPAVM_X11_WINDOW_H__

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>

#include "../base/bitmap.h"
#include "../util/numdef.h"

typedef void (*window_key_press_callback_t)(u32 key);
typedef struct window window_t;

struct window {
	xcb_window_t id;
	xcb_connection_t *connection;
	xcb_screen_t *screen;
	xcb_gcontext_t gc;
	xcb_image_t *image;
	bitmap_t *bmp;
	u32 running;
	window_key_press_callback_t key_pressed;
};

extern window_t *
window_create(const char *wm_name, const char *wm_class);

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
