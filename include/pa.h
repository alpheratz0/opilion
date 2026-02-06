/*
	Copyright (C) 2022-2026 <alpheratz99@protonmail.com>

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

#include <stddef.h>
#include <stdbool.h>
#include <pulse/pulseaudio.h>

#include "icon.h"

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

typedef struct PulseAudioConnection   PulseAudioConnection_t;
typedef struct PulseAudioSink               PulseAudioSink_t;
typedef struct PulseAudioSinkList       PulseAudioSinkList_t;
typedef struct PulseAudioServerInfo   PulseAudioServerInfo_t;

typedef enum {
	PULSEAUDIO_SINK_FILTER_NONE = 0,
	PULSEAUDIO_SINK_FILTER_SPEAKER = 1UL << 0,
	PULSEAUDIO_SINK_FILTER_MICROPHONE = 1UL << 1,
	PULSEAUDIO_SINK_FILTER_APPLICATION = 1UL << 2,
	PULSEAUDIO_SINK_FILTER_ALL = (PULSEAUDIO_SINK_FILTER_APPLICATION<<1)-1
} PulseAudioSinkFilter;

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

extern PulseAudioConnection_t *
pulseaudio_connect(void);

extern PulseAudioSinkList_t *
pulseaudio_get_all_sinks(PulseAudioConnection_t *pac, PulseAudioSinkFilter filter);

extern void
pulseaudio_disconnect(PulseAudioConnection_t *pac);

extern bool
pulseaudio_sink_matches_filter(const PulseAudioSink_t *s, PulseAudioSinkFilter filter);

extern void
pulseaudio_sink_filter_toggle(PulseAudioSinkFilter *out, PulseAudioSinkFilter to_toggle, bool toggle);

extern PulseAudioSink_t *
pulseaudio_sink_from_source(const pa_source_info *source);

extern PulseAudioSink_t *
pulseaudio_sink_from_sink(const pa_sink_info *sink);

extern PulseAudioSink_t *
pulseaudio_sink_from_sink_input(const pa_sink_input_info *sink_input);

extern const char *
pulseaudio_sink_get_display_name(const PulseAudioSink_t *s);

extern const Icon_t *
pulseaudio_sink_get_icon(const PulseAudioSink_t *s);

extern int
pulseaudio_sink_get_volume(const PulseAudioSink_t *s);

extern bool
pulseaudio_sink_is_muted(const PulseAudioSink_t *s);

extern bool
pulseaudio_sink_is_isolated(const PulseAudioSink_t *s, PulseAudioSinkList_t *sinks);

extern bool
pulseaudio_sink_is_default(const PulseAudioSink_t *s);

extern bool
pulseaudio_sink_is_sink(const PulseAudioSink_t *s);

extern bool
pulseaudio_sink_is_sink_input(const PulseAudioSink_t *s);

extern bool
pulseaudio_sink_is_source(const PulseAudioSink_t *s);

extern void
pulseaudio_sink_format_volume(const PulseAudioSink_t *s, size_t len, char *str);

extern void
pulseaudio_sink_set_volume(PulseAudioConnection_t *pac, PulseAudioSink_t *s,
		int v);

extern void
pulseaudio_sink_increase_volume(PulseAudioConnection_t *pac,
		PulseAudioSink_t *s, int v);

extern void
pulseaudio_sink_set_mute(PulseAudioConnection_t *pac, PulseAudioSink_t *s,
		bool mute);

extern void
pulseaudio_sink_set_isolate(PulseAudioConnection_t *pac, PulseAudioSink_t *s,
		PulseAudioSinkList_t *sinks, bool isolated);

extern void
pulseaudio_sink_set_default(PulseAudioConnection_t *pac, PulseAudioSink_t *s,
		PulseAudioSinkList_t *sl);

extern void
pulseaudio_sink_toggle_mute(PulseAudioConnection_t *pac, PulseAudioSink_t *s);

extern void
pulseaudio_sink_toggle_isolate(PulseAudioConnection_t *pac, PulseAudioSink_t *s,
		PulseAudioSinkList_t *sinks);

extern void
pulseaudio_sink_kill(PulseAudioConnection_t *pac, PulseAudioSink_t *s);

extern void
pulseaudio_sink_free(PulseAudioSink_t *s);

extern PulseAudioSinkList_t *
pulseaudio_sink_list_new(void);

extern void
pulseaudio_sink_list_resize(PulseAudioSinkList_t *sl, size_t new_size);

extern void
pulseaudio_sink_list_push_back(PulseAudioSinkList_t *sl, PulseAudioSink_t *s);

extern int
pulseaudio_sink_list_get_length(const PulseAudioSinkList_t *sl);

extern PulseAudioSink_t *
pulseaudio_sink_list_get(const PulseAudioSinkList_t *sl, int index);

extern void
pulseaudio_sink_list_free(PulseAudioSinkList_t *sl);