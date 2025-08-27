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
#include <pulse/pulseaudio.h>
#include <pulse/volume.h>
#include <stddef.h>

#include "icon.h"
#include "log.h"
#include "utils.h"
#include "pa.h"


// TODO (alpheratz0): please clean up this repetitive mess

#define PULSEAUDIO_SINK_LIST_DEFAULT_CAPACITY 16

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

typedef enum {
	PULSEAUDIO_ENTITY_KIND_SINK,
	PULSEAUDIO_ENTITY_KIND_SINK_INPUT,
	PULSEAUDIO_ENTITY_KIND_SOURCE
} PulseAudioSinkKind;

struct PulseAudioConnection {
	pa_threaded_mainloop *mainloop;
	pa_mainloop_api *api;
	pa_context *ctx;
	void *userdata;
};

struct PulseAudioSink {
	PulseAudioSinkKind kind;
	unsigned int id;
	char *display_name;
	const Icon_t *icon;
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
__get_sink_input_cb(pa_context *ctx, const pa_sink_input_info *sink_input,
		int eol, void *userdata)
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
				pulseaudio_sink_from_sink_input(sink_input));
	}
}

static void
__get_sink_cb(pa_context *ctx, const pa_sink_info *sink,
		int eol, void *userdata)
{
	PulseAudioConnection_t *pac;
	PulseAudioSinkList_t *sink_list;

	pac = userdata;
	sink_list = pac->userdata;

	if (eol < 0) {
		die("couldn't get sink info: %s",
				pa_strerror(pa_context_errno(ctx)));
	}

	if (eol > 0) {
		pa_threaded_mainloop_signal(pac->mainloop, 0);
		return;
	}

	if (NULL != sink) {
		pulseaudio_sink_list_push_back(sink_list,
				pulseaudio_sink_from_sink(sink));
	}
}

static void
__get_source_cb(pa_context *ctx, const pa_source_info *source,
		int eol, void *userdata)
{
	PulseAudioConnection_t *pac;
	PulseAudioSinkList_t *sink_list;

	pac = userdata;
	sink_list = pac->userdata;

	if (eol < 0) {
		die("couldn't get source info: %s",
				pa_strerror(pa_context_errno(ctx)));
	}

	if (eol > 0) {
		pa_threaded_mainloop_signal(pac->mainloop, 0);
		return;
	}

	if (NULL != source) {
		pulseaudio_sink_list_push_back(sink_list,
				pulseaudio_sink_from_source(source));
	}
}

static void
__sink_action_finished_callback(pa_context *ctx, int eol, void *userdata)
{
	PulseAudioConnection_t *pac;

	pac = userdata;

	if (!eol) {
		// Application was closed after opilion started?
		info("couldn't update sink state: %s",
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
pulseaudio_get_all_sinks(PulseAudioConnection_t *pac)
{
	pa_operation *po;
	PulseAudioSinkList_t *sl;

	pac->userdata = sl = pulseaudio_sink_list_new();

	po = pa_context_get_source_info_list(pac->ctx,
			__get_source_cb, pac);

	if (NULL == po) {
		die("pa_context_get_source_info_list failed: %s",
				pa_strerror(pa_context_errno(pac->ctx)));
	}

	pa_operation_unref(po);
	pa_threaded_mainloop_wait(pac->mainloop);

	po = pa_context_get_sink_info_list(pac->ctx,
			__get_sink_cb, pac);

	if (NULL == po) {
		die("pa_context_get_sink_info_list failed: %s",
				pa_strerror(pa_context_errno(pac->ctx)));
	}

	pa_operation_unref(po);
	pa_threaded_mainloop_wait(pac->mainloop);

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
pulseaudio_sink_from_sink_input(const pa_sink_input_info *sink_input)
{
	PulseAudioSink_t *s;
	const char *app_name, *media_name, *icon_name;

	app_name = str_fallback(pa_proplist_gets(sink_input->proplist, "application.name"), "Unknown");
	icon_name = str_fallback(pa_proplist_gets(sink_input->proplist, "application.icon_name"), "audio-x-generic");
	media_name = pa_proplist_gets(sink_input->proplist, "media.name");

	s = xmalloc(sizeof(PulseAudioSink_t));

	s->kind = PULSEAUDIO_ENTITY_KIND_SINK_INPUT;
	s->id = sink_input->index;
	s->display_name = NULL != media_name ? str_fmt("%s - %s", app_name, media_name) : xstrdup(app_name);
	s->icon = icon_from_name(icon_name);
	s->volume = sink_input->volume;
	s->channels = sink_input->channel_map.channels;
	s->is_muted = sink_input->mute > 0;

	return s;
}

extern PulseAudioSink_t *
pulseaudio_sink_from_sink(const pa_sink_info *sink)
{
	const char *dev_class, *sink_name;
	PulseAudioSink_t *s;

	dev_class = pa_proplist_gets(sink->proplist, "device.class");

	if (NULL == dev_class ||
			0 == strcmp(dev_class, "monitor")) // ignore monitor devices
		return NULL;

	sink_name = str_fallback(sink->description, sink->name);

	s = xmalloc(sizeof(PulseAudioSink_t));

	s->kind = PULSEAUDIO_ENTITY_KIND_SINK;
	s->id = sink->index;
	s->display_name = NULL != sink_name ? str_fmt("%s [Speakers]", sink_name) : xstrdup("Speakers");
	s->icon = icon_from_name("audio-speakers");
	s->volume = sink->volume;
	s->channels = sink->channel_map.channels;
	s->is_muted = sink->mute > 0;

	return s;
}

extern PulseAudioSink_t *
pulseaudio_sink_from_source(const pa_source_info *source)
{
	const char *dev_class, *src_name;
	PulseAudioSink_t *s;

	dev_class = pa_proplist_gets(source->proplist, "device.class");

	if (NULL == dev_class ||
			0 == strcmp(dev_class, "monitor")) // ignore monitor devices
		return NULL;

	src_name = str_fallback(source->description, source->name);

	s = xmalloc(sizeof(PulseAudioSink_t));

	s->kind = PULSEAUDIO_ENTITY_KIND_SOURCE;
	s->id = source->index;
	s->display_name = NULL != src_name ? str_fmt("%s [Mic]", src_name) : xstrdup("Microphone");
	s->icon = icon_from_name("audio-input-microphone");
	s->volume = source->volume;
	s->channels = source->channel_map.channels;
	s->is_muted = source->mute > 0;

	return s;
}

extern const char *
pulseaudio_sink_get_display_name(const PulseAudioSink_t *s)
{
	return s->display_name;
}

extern const Icon_t *
pulseaudio_sink_get_icon(const PulseAudioSink_t *s)
{
	return s->icon;
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

extern bool
pulseaudio_sink_is_sink(const PulseAudioSink_t *s)
{
	return s->kind == PULSEAUDIO_ENTITY_KIND_SINK;
}

extern bool
pulseaudio_sink_is_sink_input(const PulseAudioSink_t *s)
{
	return s->kind == PULSEAUDIO_ENTITY_KIND_SINK_INPUT;
}

extern bool
pulseaudio_sink_is_source(const PulseAudioSink_t *s)
{
	return s->kind == PULSEAUDIO_ENTITY_KIND_SOURCE;
}

extern bool
pulseaudio_sink_is_isolated(const PulseAudioSink_t *s, PulseAudioSinkList_t *sinks)
{
	PulseAudioSink_t *iterated_sink;
	int n_sinks = pulseaudio_sink_list_get_length(sinks);

	for (int i = 0; i < n_sinks; ++i) {
		iterated_sink = pulseaudio_sink_list_get(sinks, i);

		if (iterated_sink->kind == PULSEAUDIO_ENTITY_KIND_SINK_INPUT &&
				iterated_sink != s && !pulseaudio_sink_is_muted(iterated_sink))
			return false;
	}

	return true;
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
pulseaudio_sink_set_volume(PulseAudioConnection_t *pac, PulseAudioSink_t *s,
		int v)
{
	pa_operation *po;

	pa_cvolume_set(&s->volume, s->channels,
			CLAMP(v,0,v)*(PA_VOLUME_NORM/100));

	switch (s->kind) {
	case PULSEAUDIO_ENTITY_KIND_SOURCE:
		po = pa_context_set_source_volume_by_index(pac->ctx, s->id, &s->volume,
				__sink_action_finished_callback, pac);
		break;
	case PULSEAUDIO_ENTITY_KIND_SINK:
		po = pa_context_set_sink_volume_by_index(pac->ctx, s->id, &s->volume,
				__sink_action_finished_callback, pac);
		break;
	case PULSEAUDIO_ENTITY_KIND_SINK_INPUT:
		po = pa_context_set_sink_input_volume(pac->ctx, s->id, &s->volume,
				__sink_action_finished_callback, pac);
		break;
	}

	if (NULL == po) {
		die("pulseaudio_sink_set_volume failed: %s",
				pa_strerror(pa_context_errno(pac->ctx)));
	}

	pa_operation_unref(po);
	pa_threaded_mainloop_wait(pac->mainloop);
}

extern void
pulseaudio_sink_increase_volume(PulseAudioConnection_t *pac,
		PulseAudioSink_t *s, int v)
{
	pulseaudio_sink_set_volume(pac, s,
			pulseaudio_sink_get_volume(s)+v);
}

extern void
pulseaudio_sink_set_mute(PulseAudioConnection_t *pac, PulseAudioSink_t *s,
		bool mute)
{
	pa_operation *po;

	s->is_muted = mute;

	switch (s->kind) {
	case PULSEAUDIO_ENTITY_KIND_SOURCE:
		po = pa_context_set_source_mute_by_index(pac->ctx, s->id, mute,
				__sink_action_finished_callback, pac);
		break;
	case PULSEAUDIO_ENTITY_KIND_SINK:
		po = pa_context_set_sink_mute_by_index(pac->ctx, s->id, mute,
				__sink_action_finished_callback, pac);
		break;
	case PULSEAUDIO_ENTITY_KIND_SINK_INPUT:
		po = pa_context_set_sink_input_mute(pac->ctx, s->id, mute,
				__sink_action_finished_callback, pac);
		break;
	}

	if (NULL == po) {
		die("pulseaudio_sink_set_mute failed: %s",
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
pulseaudio_sink_set_isolate(PulseAudioConnection_t *pac, PulseAudioSink_t *s,
		PulseAudioSinkList_t *sinks, bool isolated)
{
	PulseAudioSink_t *iterated_sink;
	int n_sinks = pulseaudio_sink_list_get_length(sinks);

	if (s->kind == PULSEAUDIO_ENTITY_KIND_SINK_INPUT) {
		for (int i = 0; i < n_sinks; ++i) {
			iterated_sink = pulseaudio_sink_list_get(sinks, i);
			if (iterated_sink->kind == PULSEAUDIO_ENTITY_KIND_SINK_INPUT)
				pulseaudio_sink_set_mute(pac, iterated_sink, iterated_sink == s ? false : isolated);
		}
	}
}

extern void
pulseaudio_sink_toggle_isolate(PulseAudioConnection_t *pac, PulseAudioSink_t *s,
		PulseAudioSinkList_t *sinks)
{
	pulseaudio_sink_set_isolate(pac, s, sinks, !pulseaudio_sink_is_isolated(s, sinks));
}

extern void
pulseaudio_sink_kill(PulseAudioConnection_t *pac, PulseAudioSink_t *s)
{
	pa_operation *po;

	if (s->kind == PULSEAUDIO_ENTITY_KIND_SOURCE || s->kind == PULSEAUDIO_ENTITY_KIND_SINK)
		return;

	po = pa_context_kill_sink_input(pac->ctx, s->id,
			__sink_action_finished_callback, pac);

	if (NULL == po) {
		die("pulseaudio_sink_kill failed: %s",
				pa_strerror(pa_context_errno(pac->ctx)));
	}

	pa_operation_unref(po);
	pa_threaded_mainloop_wait(pac->mainloop);
}

extern void
pulseaudio_sink_free(PulseAudioSink_t *s)
{
	free(s->display_name);
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
	if (NULL == s) return;
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