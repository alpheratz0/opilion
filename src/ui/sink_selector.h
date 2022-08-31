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

#ifndef __XPAVM_UI_SINK_SELECTOR_H__
#define __XPAVM_UI_SINK_SELECTOR_H__

#include <stdint.h>

#include "../base/bitmap.h"
#include "../base/font.h"
#include "../base/linkedlist.h"
#include "../pulseaudio/sink.h"

struct sink_style {
	struct font *font;
	uint32_t width;
	uint32_t height;
	uint32_t foreground;
	uint32_t bar_filled;
	uint32_t bar_unfilled;
};

struct sink_selector {
	struct linkedlist *sinks;
	struct sink_style *snormal, *sselected;
	uint32_t index;
	uint32_t length;
};

extern struct sink_style
sink_style_from(struct font *font, uint32_t width, uint32_t height,
                uint32_t foreground, uint32_t bar_filled, uint32_t bar_unfilled);

extern struct sink_selector *
sink_selector_create(struct linkedlist *sinks, struct sink_style *snormal,
                     struct sink_style *sselected);

extern struct sink *
sink_selector_get_selected(struct sink_selector *selector);

extern void
sink_selector_select_up(struct sink_selector *selector);

extern void
sink_selector_select_down(struct sink_selector *selector);

extern void
sink_selector_render_onto(struct sink_selector *selector, struct bitmap *bmp);

extern void
sink_selector_free(struct sink_selector *selector);

#endif
