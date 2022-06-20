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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../base/bitmap.h"
#include "../base/font.h"
#include "../base/linkedlist.h"
#include "../pulseaudio/sink.h"
#include "../util/debug.h"
#include "label.h"
#include "sink_selector.h"

extern sink_style_t
sink_style_from(font_t *font,
                uint32_t width,
                uint32_t height,
                uint32_t foreground,
                uint32_t bar_filled,
                uint32_t bar_unfilled)
{
	sink_style_t style;

	style.font = font;
	style.width = width;
	style.height = height;
	style.foreground = foreground;
	style.bar_filled = bar_filled;
	style.bar_unfilled = bar_unfilled;

	return style;
}

extern sink_selector_t *
sink_selector_create(linkedlist_t *sinks,
                     sink_style_t *snormal,
                     sink_style_t *sselected)
{
	sink_selector_t *selector;

	if (NULL == (selector = malloc(sizeof(sink_selector_t)))) {
		die("error while calling malloc, no memory available");
	}

	selector->sinks = sinks;
	selector->snormal = snormal;
	selector->sselected = sselected;
	selector->length = linkedlist_length(sinks);
	selector->index = 0;

	return selector;
}

extern sink_t *
sink_selector_get_selected(sink_selector_t *selector)
{
	sink_t *selected;

	selected = linkedlist_get_as(selector->sinks, selector->index, sink_t);

	return selected;
}

extern void
sink_selector_select_up(sink_selector_t *selector)
{
	selector->index = selector->index == 0 ?
		selector->length - 1 :
		selector->index - 1;
}

extern void
sink_selector_select_down(sink_selector_t *selector)
{
	selector->index = selector->index == selector->length - 1 ?
		0 : selector->index + 1;
}

static void
sink_selector_sink_volume_format(sink_t *sink, char *volstr)
{
	if (sink->mute) {
		strcpy(volstr, "muted");
		return;
	}

	sprintf(volstr, "%u%%", sink->volume);
}

static void
sink_selector_sink_render_onto(sink_t *sink,
                               sink_style_t *style,
                               uint32_t y,
                               bitmap_t *bmp)
{
	uint32_t x;
	char volstr[128];

	/* centered */
	x = (bmp->width - style->width) / 2;

	sink_selector_sink_volume_format(sink, volstr);

	/* application name (top left corner) */
	label_render_onto(
		sink->appname, style->font, style->foreground,
		x, y, bmp
	);

	/* application volume (top right corner) */
	label_render_onto(
		volstr, style->font, style->foreground,
		x + style->width - style->font->width * strlen(volstr), y, bmp
	);

	/* volume bar */
	bitmap_rect(
		bmp, x, y + style->font->height, style->width,
		style->height - style->font->height, style->bar_unfilled
	);

	bitmap_rect(
		bmp, x, y + style->font->height, (style->width * sink->volume) / 100,
		style->height - style->font->height, style->bar_filled
	);
}

extern void
sink_selector_render_onto(sink_selector_t *selector, bitmap_t *bmp)
{
	linkedlist_t *temp;
	sink_t *sink;
	sink_style_t *style;
	uint32_t margin, index, y;

	index = 0;
	margin = 30;
	temp = selector->sinks;

	/* total available space */
	y = bmp->height;

	/* space used by non selected sinks */
	y -= (selector->length - 1) * (selector->snormal->height);

	/* space used by selected sinks */
	y -= (selector->sselected->height);

	/* space used by margin */
	y -= (selector->length - 1) * (margin);

	/* center it */
	y /= 2;

	while (NULL != temp) {
		sink = temp->data;
		temp = temp->next;
		style = selector->snormal;

		if (index == selector->index) {
			style = selector->sselected;
		}

		sink_selector_sink_render_onto(sink, style, y, bmp);

		y += style->height + margin;
		++index;
	}
}

extern void
sink_selector_free(sink_selector_t *selector)
{
	free(selector);
}
