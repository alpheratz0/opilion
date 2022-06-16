#ifndef __XPAVM_UI_SINK_SELECTOR_H__
#define __XPAVM_UI_SINK_SELECTOR_H__

#include <stdint.h>

#include "../base/bitmap.h"
#include "../base/font.h"
#include "../base/linkedlist.h"
#include "../pulseaudio/sink.h"

typedef struct sink_style sink_style_t;
typedef struct sink_selector sink_selector_t;

struct sink_style {
	font_t *font;
	uint32_t width;
	uint32_t height;
	uint32_t foreground;
	uint32_t bar_filled;
	uint32_t bar_unfilled;
};

struct sink_selector {
	linkedlist_t *sinks;
	sink_style_t *snormal, *sselected;
	uint32_t index;
	uint32_t length;
};

extern sink_style_t
sink_style_from(font_t *font,
                uint32_t width,
                uint32_t height,
                uint32_t foreground,
                uint32_t bar_filled,
                uint32_t bar_unfilled);

extern sink_selector_t *
sink_selector_create(linkedlist_t *sinks,
                     sink_style_t *snormal,
                     sink_style_t *sselected);

extern sink_t *
sink_selector_get_selected(sink_selector_t *selector);

extern void
sink_selector_select_up(sink_selector_t *selector);

extern void
sink_selector_select_down(sink_selector_t *selector);

extern void
sink_selector_render_onto(sink_selector_t *selector, bitmap_t *bmp);

extern void
sink_selector_free(sink_selector_t *selector);

#endif
