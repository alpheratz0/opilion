/*
	Copyright (C) 2022-2023 <alpheratz99@protonmail.com>

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

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

typedef struct SinkColorTheme SinkColorTheme_t;
typedef struct SinkSelector SinkSelector_t;

struct SinkColorTheme {
	uint32_t c_app_name;
	uint32_t c_volume;
	uint32_t c_volume_bar[2];
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

extern SinkColorTheme_t
sink_color_theme_from(uint32_t c_app_name, uint32_t c_volume, const uint32_t c_volume_bar[2]);

extern SinkSelector_t *
sink_selector_new(PulseAudioSinkList_t *sl, TextRenderer_t *tr, SinkColorTheme_t *ct_nor, SinkColorTheme_t *ct_sel);

extern PulseAudioSink_t *
sink_selector_get_selected(const SinkSelector_t *ss);

extern void
sink_selector_select_up(SinkSelector_t *ss);

extern void
sink_selector_select_down(SinkSelector_t *ss);

extern void
sink_selector_render_to(const SinkSelector_t *ss, Pixbuf_t *pb);

extern void
sink_selector_free(SinkSelector_t *ss);
