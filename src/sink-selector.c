/*
	Copyright (C) 2022 <alpheratz99@protonmail.com>

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
#include <string.h>
#include <stdint.h>

#include "pixbuf.h"
#include "sink-selector.h"
#include "text-renderer.h"
#include "pa.h"
#include "utils.h"

struct SinkSelector {
	PulseAudioSinkList_t *sinks;
	SinkSelectorStyle_t ssro;
	int selected;
};

static void
__fmt_volume(PulseAudioSink_t *sink, char *str)
{
	if (pulseaudio_sink_is_muted(sink)) {
		strcpy(str, "muted");
	} else {
		sprintf(str, "%u%%", pulseaudio_sink_get_volume(sink));
	}
}

extern void
sink_style_init(SinkStyle_t *sro, TextRenderer_t *tr,
		int w, int h, uint32_t fg_color, uint32_t vbar_color1,
		uint32_t vbar_color2)
{
	sro->tr = tr;
	sro->width = w;
	sro->height = h;
	sro->fg_color = fg_color;
	sro->vbar_colors[0] = vbar_color1;
	sro->vbar_colors[1] = vbar_color2;
}

extern void
sink_selector_style_init(SinkSelectorStyle_t *ssro,
		SinkStyle_t *sro_normal, SinkStyle_t *sro_selected)
{
	ssro->sro_normal = *sro_normal;
	ssro->sro_selected = *sro_selected;
}

extern SinkSelector_t *
sink_selector_new(PulseAudioSinkList_t *sl, SinkSelectorStyle_t *ssro)
{
	SinkSelector_t *sink_selector;

	sink_selector = xmalloc(sizeof(SinkSelector_t));
	sink_selector->sinks = sl;
	sink_selector->selected = 0;
	sink_selector->ssro = *ssro;

	return sink_selector;
}

extern PulseAudioSink_t *
sink_selector_get_selected(SinkSelector_t *s)
{
	return pulseaudio_sink_list_get(s->sinks,
			s->selected);
}

extern void
sink_selector_select_up(SinkSelector_t *s)
{
	if (s->selected == 0) {
		s->selected = pulseaudio_sink_list_get_length(s->sinks) - 1;
	} else {
		s->selected -= 1;
	}
}

extern void
sink_selector_select_down(SinkSelector_t *s)
{
	if (s->selected == pulseaudio_sink_list_get_length(s->sinks) - 1) {
		s->selected = 0;
	} else {
		s->selected += 1;
	}
}

static void
__render_sink(PulseAudioSink_t *sink, SinkStyle_t *sro, uint32_t y, Pixbuf_t *pb)
{
	char volume_str[128];
	int app_name_x, app_name_y;
	int volume_str_x, volume_str_y;
	int volume_box_x, volume_box_y;

	// render app name
	app_name_x = (pixbuf_get_width(pb) - sro->width) / 2;
	app_name_y = y;

	text_renderer_draw_string(sro->tr, pb, pulseaudio_sink_get_app_name(sink),
			app_name_x, app_name_y, sro->fg_color);

	// render volume str
	__fmt_volume(sink, volume_str);
	volume_str_x = app_name_x + sro->width - text_renderer_text_width(sro->tr, volume_str);
	volume_str_y = y;

	text_renderer_draw_string(sro->tr, pb, volume_str, volume_str_x,
			volume_str_y, sro->fg_color);

	// render volume box
	volume_box_x = app_name_x;
	volume_box_y = app_name_y + text_renderer_text_height(sro->tr);

	pixbuf_rect(pb, volume_box_x, volume_box_y, sro->width,
			sro->height - text_renderer_text_height(sro->tr), sro->vbar_colors[1]);

	pixbuf_rect(pb, volume_box_x, volume_box_y, (sro->width * pulseaudio_sink_get_volume(sink)) / 100,
			sro->height - text_renderer_text_height(sro->tr), sro->vbar_colors[0]);
}

extern void
sink_selector_render(SinkSelector_t *s, Pixbuf_t *pb)
{
	int i, len;
	PulseAudioSinkList_t *sl;
	SinkStyle_t *sro;
	int margin, y;

	sl = s->sinks;
	len = pulseaudio_sink_list_get_length(sl);
	margin = 30;

	/* total available space */
	y = pixbuf_get_height(pb);

	if (len > 5) {
		/* space used by selected sink */
		y -= (s->ssro.sro_selected.height);

		/* center it */
		y /= 2;

		/* space used by non selected sinks (before selected one)*/
		y -= (s->selected) * (s->ssro.sro_normal.height);

		/* space used by margin */
		y -= (s->selected) * (margin);
	} else {
		/* space used by non selected sinks */
		y -= (len - 1) * (s->ssro.sro_normal.height);

		/* space used by selected sinks */
		y -= (s->ssro.sro_selected.height);

		/* space used by margin */
		y -= (len - 1) * (margin);

		/* center it */
		y /= 2;
	}

	for (i = 0; i < len; ++i) {
		sro = &s->ssro.sro_normal;

		if (s->selected == i)
			sro = &s->ssro.sro_selected;

		__render_sink(pulseaudio_sink_list_get(sl, i),
				sro, y, pb);

		y += sro->height + margin;
	}
}

extern void
sink_selector_free(SinkSelector_t *s)
{
	free(s);
}
