/*
	Copyright (C) 2022-2025 <alpheratz99@protonmail.com>

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
#include "render-util.h"
#include "pa.h"
#include "utils.h"

#define SINK_VOL_SLIDER_HEIGHT 5
#define SINK_MARGIN 15

#define LOOPVAL(v,n) \
	(((v)%(n)+(n))%(n))

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

struct SinkSelector {
	TextRenderer_t *tr;
	PulseAudioSinkList_t *sinks;
	SinkColorTheme_t ct_nor, ct_sel;
	int len, selected;
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

static void
__sink_render_to(const PulseAudioSink_t *s, TextRenderer_t *tr,
		const SinkColorTheme_t *ct, int x, int y, Pixbuf_t *pb)
{
	char volume_str[128];

	pulseaudio_sink_format_volume(s, sizeof(volume_str), volume_str);

	render_util_render_key_value_pair(pb, x, y, pixbuf_get_width(pb),
			tr, pulseaudio_sink_get_app_name(s), ct->c_app_name,
			volume_str, ct->c_volume);

	render_util_render_slider(pb, x, y+text_renderer_text_height(tr),
			pixbuf_get_width(pb), SINK_VOL_SLIDER_HEIGHT, pulseaudio_sink_get_volume(s),
			ct->c_volume_bar);
}

extern SinkColorTheme_t
sink_color_theme_from(uint32_t c_app_name, uint32_t c_volume,
		const uint32_t c_volume_bar[2])
{
	SinkColorTheme_t ct;
	ct.c_app_name = c_app_name;
	ct.c_volume = c_volume;
	ct.c_volume_bar[0] = c_volume_bar[0];
	ct.c_volume_bar[1] = c_volume_bar[1];
	return ct;
}

extern SinkSelector_t *
sink_selector_new(PulseAudioSinkList_t *sl, TextRenderer_t *tr,
		SinkColorTheme_t *ct_nor, SinkColorTheme_t *ct_sel)
{
	SinkSelector_t *ss;

	ss = xmalloc(sizeof(SinkSelector_t));

	ss->sinks = sl;
	ss->tr = tr;
	ss->selected = 0;
	ss->len = pulseaudio_sink_list_get_length(sl);
	ss->ct_nor = *ct_nor;
	ss->ct_sel = *ct_sel;

	return ss;
}

extern PulseAudioSink_t *
sink_selector_get_selected(const SinkSelector_t *ss)
{
	return pulseaudio_sink_list_get(ss->sinks,
			ss->selected);
}

extern void
sink_selector_set_sink_list(SinkSelector_t *ss, PulseAudioSinkList_t *sl)
{
	ss->sinks = sl;
	ss->len = pulseaudio_sink_list_get_length(sl);
	if (ss->len == 0)
		ss->selected = 0;
	else if (ss->selected >= ss->len)
		ss->selected = ss->len - 1;
}

extern void
sink_selector_select_up(SinkSelector_t *ss)
{
	ss->selected = LOOPVAL(ss->selected-1, ss->len);
}

extern void
sink_selector_select_down(SinkSelector_t *ss)
{
	ss->selected = LOOPVAL(ss->selected+1, ss->len);
}

extern void
sink_selector_render_to(const SinkSelector_t *ss, Pixbuf_t *pb)
{
	int i, x, y;
	int pb_w, pb_h, sink_h;
	PulseAudioSink_t *sink;
	const SinkColorTheme_t *ct;

	pb_w = pixbuf_get_width(pb);
	pb_h = pixbuf_get_height(pb);
	sink_h = text_renderer_text_height(ss->tr) + SINK_VOL_SLIDER_HEIGHT;
	x = (pb_w - pixbuf_get_width(pb)) / 2;

	if (ss->len > 10) {
		y = (pb_h - sink_h) / 2 - ss->selected * (sink_h + SINK_MARGIN);
	} else {
		y = (pb_h - ss->len * sink_h - (ss->len - 1) * SINK_MARGIN) / 2;
	}

	for (i = 0; i < ss->len; ++i) {
		sink = pulseaudio_sink_list_get(ss->sinks, i);
		ct = i == ss->selected ? &ss->ct_sel : &ss->ct_nor;
		__sink_render_to(sink, ss->tr, ct, x, y, pb);
		y += sink_h + SINK_MARGIN;
	}
}

extern void
sink_selector_free(SinkSelector_t *ss)
{
	free(ss);
}
