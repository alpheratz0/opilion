#ifndef __XPAVM_UI_SINK_SELECTOR_H__
#define __XPAVM_UI_SINK_SELECTOR_H__

#include "../base/bitmap.h"
#include "../base/font.h"
#include "../base/linkedlist.h"
#include "../pulseaudio/sink.h"
#include "../util/numdef.h"

typedef struct sink_style sink_style_t;
typedef struct sink_selector sink_selector_t;

struct sink_style {
	u32 text_color;
	u32 bar_filled_color;
	u32 bar_not_filled_color;
};

struct sink_selector {
	linkedlist_t *sinks;
	font_t *font;
	u32 width, height;
	sink_style_t *snormal, *sselected;
	u32 index;
};

extern sink_style_t
sink_style_from(u32 text_color, u32 bar_filled_color, u32 bar_not_filled_color);

extern sink_selector_t *
sink_selector_create(linkedlist_t *sinks, font_t *font, u32 width, u32 height, sink_style_t *snormal, sink_style_t *sselected);

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
