#include <stdlib.h>

#include "../base/bitmap.h"
#include "../base/font.h"
#include "../base/linkedlist.h"
#include "../pulseaudio/sink.h"
#include "../util/debug.h"
#include "../util/numdef.h"
#include "label.h"
#include "sink_selector.h"

extern sink_style_t
sink_style_from(u32 text_color, u32 bar_filled_color, u32 bar_not_filled_color)
{
	sink_style_t style;
	style.text_color = text_color;
	style.bar_filled_color = bar_filled_color;
	style.bar_not_filled_color = bar_not_filled_color;
	return style;
}

extern sink_selector_t *
sink_selector_create(linkedlist_t *sinks, font_t *font, u32 width, u32 height, sink_style_t *snormal, sink_style_t *sselected)
{
	sink_selector_t *selector;

	if (NULL == (selector = malloc(sizeof(sink_selector_t)))) {
		die("error while calling malloc, no memory available");
	}

	selector->sinks = sinks;
	selector->font = font;
	selector->width = width;
	selector->height = height;
	selector->snormal = snormal;
	selector->sselected = sselected;
	selector->index = 0;

	return selector;
}

extern sink_t *
sink_selector_get_selected(sink_selector_t *selector)
{
	return linkedlist_get_as(selector->sinks, selector->index, sink_t);
}

extern void
sink_selector_select_up(sink_selector_t *selector)
{
	if (selector->index == 0) selector->index = linkedlist_length(selector->sinks) - 1;
	else selector->index--;
}

extern void
sink_selector_select_down(sink_selector_t *selector)
{
	if (selector->index == (linkedlist_length(selector->sinks) - 1)) selector->index = 0;
	else selector->index++;
}

extern void
sink_selector_render_onto(sink_selector_t *selector, bitmap_t *bmp)
{
	u32 nsinks, height, start_x, start_y, current_x, current_y;
	sink_t *current_sink;
	sink_style_t *current_style;
	char vstr[5];

	if ((nsinks = linkedlist_length(selector->sinks)) != 0) {
		height = nsinks * selector->height + 30 * (nsinks - 1);
		start_x = (bmp->width - selector->width) / 2;
		start_y = (bmp->height - height) / 2;

		current_x = start_x;
		current_y = start_y;

		for (u32 i = 0; i < nsinks; ++i) {
			current_sink = linkedlist_get_as(selector->sinks, i, sink_t);
			current_style = i == selector->index ? selector->sselected :
				selector->snormal;
			label_render_onto(bmp, selector->font, current_style->text_color,
					current_sink->appname, current_x, current_y);

			if (current_sink->mute) {
				label_render_onto(
					bmp, selector->font, current_style->text_color, "muted",
					current_x + selector->width - selector->font->width *
					(sizeof("muted") - 1), current_y
				);
			}
			else {
				snprintf(vstr, sizeof(vstr), "%d%%", current_sink->volume);
				label_render_onto(
					bmp, selector->font, current_style->text_color, vstr,
					current_x + selector->width - selector->font->width *
					strlen(vstr), current_y
				);
			}

			bitmap_rect(
				bmp, current_x, current_y + selector->font->height,
				selector->width, selector->height - selector->font->height,
				current_style->bar_not_filled_color
			);

			bitmap_rect(
				bmp, current_x, current_y + selector->font->height,
				(selector->width * current_sink->volume) / 100,
				selector->height - selector->font->height,
				current_style->bar_filled_color
			);

			current_x = start_x;
			current_y += selector->height + 30;
		}
	}
}

extern void
sink_selector_free(sink_selector_t *selector)
{
	free(selector);
}
