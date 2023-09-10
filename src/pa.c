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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pulse/pulseaudio.h>
#include <pulse/volume.h>
#include <stddef.h>

#include "log.h"
#include "utils.h"
#include "pa.h"

#define PULSEAUDIO_SINK_LIST_DEFAULT_CAPACITY 16

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

struct PulseAudioConnection {
	pa_threaded_mainloop *mainloop;
	pa_mainloop_api *api;
	pa_context *ctx;
	void *userdata;
};

struct PulseAudioSink {
	unsigned int id;
	char *application_name;
	pa_cvolume volume;
	uint8_t channels;
	bool is_muted;
};

struct PulseAudioSinkList {
	size_t len, cap;
	PulseAudioSink_t **sinks;
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

static void
__context_state_change_cb(UNUSED pa_context *ctx, void *userdata)
{
	pa_threaded_mainloop *mainloop;

	mainloop = userdata;

	pa_threaded_mainloop_signal(mainloop, 0);
}

static void
__get_sink_input_cb(pa_context *ctx, const pa_sink_input_info *sink_input, int eol, void *userdata)
{
	PulseAudioConnection_t *pac;
	PulseAudioSinkList_t *sink_list;

	pac = userdata;
	sink_list = pac->userdata;

	if (eol < 0) {
		die("couldn't get sink input info: %s",
				pa_strerror(pa_context_errno(ctx)));
	}

	if (eol > 0) {
		pa_threaded_mainloop_signal(pac->mainloop, 0);
		return;
	}

	if (NULL != sink_input) {
		pulseaudio_sink_list_push_back(sink_list,
				pulseaudio_sink_new(sink_input));
	}
}

static void
__update_sink_input_cb(pa_context *ctx, int eol, void *userdata)
{
	PulseAudioConnection_t *pac;

	pac = userdata;

	if (!eol) {
		// Application was closed after xpavm started?
		info("couldn't update sink input state: %s",
				pa_strerror(pa_context_errno(ctx)));
	}

	pa_threaded_mainloop_signal(pac->mainloop, 0);
}

extern PulseAudioConnection_t *
pulseaudio_connect(void)
{
	PulseAudioConnection_t *pac;

	pac = xmalloc(sizeof(PulseAudioConnection_t));

	// Init
	pac->mainloop = pa_threaded_mainloop_new();
	pac->api = pa_threaded_mainloop_get_api(pac->mainloop);
	pac->ctx = pa_context_new(pac->api, NULL);
	pac->userdata = NULL;

	pa_context_set_state_callback(pac->ctx,
			__context_state_change_cb, pac->mainloop);

	pa_threaded_mainloop_lock(pac->mainloop);
	pa_threaded_mainloop_start(pac->mainloop);
	pa_context_connect(pac->ctx, NULL, PA_CONTEXT_NOFAIL, NULL);

	while (pa_context_get_state(pac->ctx) != PA_CONTEXT_READY)
		pa_threaded_mainloop_wait(pac->mainloop);

	return pac;
}

extern PulseAudioSinkList_t *
pulseaudio_get_all_input_sinks(PulseAudioConnection_t *pac)
{
	pa_operation *po;
	PulseAudioSinkList_t *sl;

	pac->userdata = sl = pulseaudio_sink_list_new();

	po = pa_context_get_sink_input_info_list(pac->ctx,
			__get_sink_input_cb, pac);

	if (NULL == po) {
		die("pa_context_get_sink_input_info_list failed: %s",
				pa_strerror(pa_context_errno(pac->ctx)));
	}

	pa_operation_unref(po);
	pa_threaded_mainloop_wait(pac->mainloop);

	return sl;
}

extern void
pulseaudio_disconnect(PulseAudioConnection_t *pac)
{
	pa_context_unref(pac->ctx);
	pa_threaded_mainloop_unlock(pac->mainloop);
	pa_threaded_mainloop_free(pac->mainloop);
	free(pac);
}

extern PulseAudioSink_t *
pulseaudio_sink_new(const pa_sink_input_info *sink_input)
{
	PulseAudioSink_t *s;
	const char *orig_app_name;

	s = xmalloc(sizeof(PulseAudioSink_t));

	orig_app_name = pa_proplist_gets(sink_input->proplist,
			"application.name");

	if (NULL == orig_app_name)
		orig_app_name = "Unknown";

	s->id = sink_input->index;
	s->application_name = xstrdup(orig_app_name);
	s->volume = sink_input->volume;
	s->channels = sink_input->channel_map.channels;
	s->is_muted = sink_input->mute > 0;

	return s;
}

extern const char *
pulseaudio_sink_get_app_name(const PulseAudioSink_t *s)
{
	return s->application_name;
}

extern int
pulseaudio_sink_get_volume(const PulseAudioSink_t *s)
{
	return s->volume.values[0] / (PA_VOLUME_NORM / 100);
}

extern bool
pulseaudio_sink_is_muted(const PulseAudioSink_t *s)
{
	return s->is_muted;
}

extern void
pulseaudio_sink_format_volume(const PulseAudioSink_t *s, size_t sz, char *str)
{
	int vol;

	if (pulseaudio_sink_is_muted(s)) {
		strncpy(str, "muted", sz-1);
	} else {
		vol = pulseaudio_sink_get_volume(s);
		snprintf(str, sz, "%u%%", vol);
	}
}

extern void
pulseaudio_sink_set_volume(PulseAudioConnection_t *pac, PulseAudioSink_t *s, int v)
{
	pa_operation *po;

	pa_cvolume_set(&s->volume, s->channels,
			CLAMP(v,0,v)*(PA_VOLUME_NORM/100));

	po = pa_context_set_sink_input_volume(pac->ctx, s->id, &s->volume,
			__update_sink_input_cb, pac);

	if (NULL == po) {
		die("pa_context_set_sink_input_volume failed: %s",
				pa_strerror(pa_context_errno(pac->ctx)));
	}

	pa_operation_unref(po);
	pa_threaded_mainloop_wait(pac->mainloop);
}

extern void
pulseaudio_sink_increase_volume(PulseAudioConnection_t *pac, PulseAudioSink_t *s, int v)
{
	pulseaudio_sink_set_volume(pac, s,
			pulseaudio_sink_get_volume(s)+v);
}

extern void
pulseaudio_sink_set_mute(PulseAudioConnection_t *pac, PulseAudioSink_t *s, bool mute)
{
	pa_operation *po;

	s->is_muted = mute;
	po = pa_context_set_sink_input_mute(pac->ctx, s->id, mute,
			__update_sink_input_cb, pac);

	if (NULL == po) {
		die("pa_context_set_sink_input_mute failed: %s",
				pa_strerror(pa_context_errno(pac->ctx)));
	}

	pa_operation_unref(po);
	pa_threaded_mainloop_wait(pac->mainloop);
}

extern void
pulseaudio_sink_toggle_mute(PulseAudioConnection_t *pac, PulseAudioSink_t *s)
{
	pulseaudio_sink_set_mute(pac, s, !s->is_muted);
}

extern void
pulseaudio_sink_free(PulseAudioSink_t *s)
{
	free(s->application_name);
	free(s);
}

extern PulseAudioSinkList_t *
pulseaudio_sink_list_new(void)
{
	PulseAudioSinkList_t *sl;
	sl = xcalloc(1, sizeof(PulseAudioSinkList_t));
	sl->cap = PULSEAUDIO_SINK_LIST_DEFAULT_CAPACITY;
	sl->sinks = xmalloc(sl->cap * sizeof(void *));
	return sl;
}

extern void
pulseaudio_sink_list_resize(PulseAudioSinkList_t *sl, size_t new_size)
{
	if (new_size > sl->cap) {
		sl->sinks = xrealloc(sl->sinks,
			(sl->cap=new_size) * sizeof(void *));
	}
}

extern void
pulseaudio_sink_list_push_back(PulseAudioSinkList_t *sl, PulseAudioSink_t *s)
{
	if (sl->len == sl->cap)
		pulseaudio_sink_list_resize(sl, sl->len + 16);
	sl->sinks[sl->len++] = s;
}

extern int
pulseaudio_sink_list_get_length(const PulseAudioSinkList_t *sl)
{
	return sl->len;
}

extern PulseAudioSink_t *
pulseaudio_sink_list_get(const PulseAudioSinkList_t *sl, int index)
{
	if (index < 0 || index >= (int)(sl->len))
		return NULL;
	return sl->sinks[index];
}

extern void
pulseaudio_sink_list_free(PulseAudioSinkList_t *sl)
{
	while (sl->len > 0)
		pulseaudio_sink_free(
			sl->sinks[--sl->len]);
	free(sl->sinks);
	free(sl);
}