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

#pragma once

#include <stdint.h>

#include "pa.h"
#include "text-renderer.h"
#include "pixbuf.h"

typedef struct SinkRenderOptions SinkRenderOptions_t;
typedef struct SinkSelectorRenderOptions SinkSelectorRenderOptions_t;
typedef struct SinkSelector SinkSelector_t;

struct SinkRenderOptions {
	TextRenderer_t *tr;
	int width, height;
	uint32_t fg_color;
	uint32_t vbar_colors[2];
};

struct SinkSelectorRenderOptions {
	SinkRenderOptions_t sro_normal;
	SinkRenderOptions_t sro_selected;
};

extern void
sink_render_options_init(SinkRenderOptions_t *sro, TextRenderer_t *tr,
		int w, int h, uint32_t fg_color, uint32_t vbar_color1,
		uint32_t vbar_color2);

extern void
sink_selector_render_options_init(SinkSelectorRenderOptions_t *ssro,
		SinkRenderOptions_t *sro_normal, SinkRenderOptions_t *sro_selected);

extern SinkSelector_t *
sink_selector_new(PulseAudioSinkList_t *sl, SinkSelectorRenderOptions_t *ssro);

extern PulseAudioSink_t *
sink_selector_get_selected(SinkSelector_t *s);

extern void
sink_selector_select_up(SinkSelector_t *s);

extern void
sink_selector_select_down(SinkSelector_t *s);

extern void
sink_selector_render(SinkSelector_t *s, Pixbuf_t *pb);

extern void
sink_selector_free(SinkSelector_t *s);
