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

#include <stdint.h>
#include <pulse/pulseaudio.h>
#include <pulse/volume.h>

#include "log.h"
#include "utils.h"
#include "pa.h"

struct PulseAudioConnection {
	pa_threaded_mainloop *mainloop;
	pa_mainloop_api *api;
	pa_context *context;
	void *userdata;
};

struct PulseAudioSink {
	char *appname;
	uint32_t id;
	pa_cvolume volume;
	uint8_t channels;
	bool mute;
};

struct PulseAudioSinkList {
	size_t len, cap;
	PulseAudioSink_t **sinks;
};

static PulseAudioSinkList_t *
__pulseaudio_sink_list_new(void);

static PulseAudioSink_t *
__pulseaudio_sink_new(const char *appname, uint32_t id,
		const pa_cvolume *volume, uint8_t channels, uint32_t mute);

static void
__pulseaudio_sink_list_push(PulseAudioSinkList_t *sl, PulseAudioSink_t *s);

static void
__context_state_change_cb(pa_context *ctx, void *userdata)
{
	pa_threaded_mainloop *mainloop;

	(void) ctx;

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
		__pulseaudio_sink_list_push(sink_list,
			__pulseaudio_sink_new(
				pa_proplist_gets(sink_input->proplist, "application.name"),
				sink_input->index,
				&sink_input->volume,
				sink_input->channel_map.channels,
				sink_input->mute
			)
		);
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

static PulseAudioSink_t *
__pulseaudio_sink_new(const char *appname, uint32_t id, const pa_cvolume *volume, uint8_t channels, uint32_t mute)
{
	PulseAudioSink_t *s;

	s = xmalloc(sizeof(PulseAudioSink_t));

	// Init
	s->appname = xstrdup(NULL == appname ? "unknown" : appname);
	s->id = id;
	s->volume = *volume;
	s->channels = channels;
	s->mute = mute > 0 ? true : false;

	return s;
}

static void
__pulseaudio_sink_free(PulseAudioSink_t *s)
{
	free(s->appname);
	free(s);
}

static PulseAudioSinkList_t *
__pulseaudio_sink_list_new(void)
{
	PulseAudioSinkList_t *sl;

	sl = malloc(sizeof(PulseAudioSinkList_t));

	sl->len = 0;
	sl->cap = 16;
	sl->sinks = xmalloc(
		sl->cap*sizeof(PulseAudioSink_t*));

	return sl;
}

static void
__pulseaudio_sink_list_push(PulseAudioSinkList_t *sl, PulseAudioSink_t *s)
{
	if (sl->len == sl->cap) {
		sl->cap += 16;
		sl->sinks = realloc(sl->sinks,
				sl->cap*sizeof(PulseAudioSink_t*));
	}

	sl->sinks[sl->len] = s;
	sl->len += 1;
}

extern PulseAudioConnection_t *
pulseaudio_connect(void)
{
	PulseAudioConnection_t *pac;

	pac = xmalloc(sizeof(PulseAudioConnection_t));

	// Init
	pac->mainloop = pa_threaded_mainloop_new();
	pac->api = pa_threaded_mainloop_get_api(pac->mainloop);
	pac->context = pa_context_new(pac->api, NULL);
	pac->userdata = NULL;

	pa_context_set_state_callback(pac->context,
			__context_state_change_cb, pac->mainloop);

	pa_threaded_mainloop_lock(pac->mainloop);
	pa_threaded_mainloop_start(pac->mainloop);
	pa_context_connect(pac->context, NULL, PA_CONTEXT_NOFAIL, NULL);

	while (pa_context_get_state(pac->context) != PA_CONTEXT_READY)
		pa_threaded_mainloop_wait(pac->mainloop);

	return pac;
}

extern PulseAudioSinkList_t *
pulseaudio_get_all_input_sinks(PulseAudioConnection_t *pac)
{
	pa_operation *po;
	PulseAudioSinkList_t *sl;

	pac->userdata = sl = __pulseaudio_sink_list_new();

	po = pa_context_get_sink_input_info_list(pac->context,
			__get_sink_input_cb, pac);

	if (NULL == po) {
		die("pa_context_get_sink_input_info_list failed: %s",
				pa_strerror(pa_context_errno(pac->context)));
	}

	pa_operation_unref(po);
	pa_threaded_mainloop_wait(pac->mainloop);

	return sl;
}

extern void
pulseaudio_sink_set_volume(PulseAudioConnection_t *pac, PulseAudioSink_t *s, int v)
{
	pa_operation *po;

	pa_cvolume_set(&s->volume, s->channels,
			CLAMP(v,0,100)*(PA_VOLUME_NORM/100));

	po = pa_context_set_sink_input_volume(pac->context, s->id, &s->volume,
			__update_sink_input_cb, pac);

	if (NULL == po) {
		die("pa_context_set_sink_input_volume failed: %s",
				pa_strerror(pa_context_errno(pac->context)));
	}

	pa_operation_unref(po);
	pa_threaded_mainloop_wait(pac->mainloop);
}

extern void
pulseaudio_sink_increase_volume(PulseAudioConnection_t *pac, PulseAudioSink_t *s, int v)
{
	pulseaudio_sink_set_volume(pac, s, pulseaudio_sink_get_volume(s)+v);
}

extern void
pulseaudio_sink_set_mute(PulseAudioConnection_t *pac, PulseAudioSink_t *s, bool mute)
{
	pa_operation *po;

	s->mute = mute;
	po = pa_context_set_sink_input_mute(pac->context, s->id, mute,
			__update_sink_input_cb, pac);

	if (NULL == po) {
		die("pa_context_set_sink_input_mute failed: %s",
				pa_strerror(pa_context_errno(pac->context)));
	}

	pa_operation_unref(po);
	pa_threaded_mainloop_wait(pac->mainloop);
}

extern void
pulseaudio_sink_toggle_mute(PulseAudioConnection_t *pac, PulseAudioSink_t *s)
{
	pulseaudio_sink_set_mute(pac, s, !s->mute);
}

extern const char *
pulseaudio_sink_get_app_name(const PulseAudioSink_t *s)
{
	return s->appname;
}

extern int
pulseaudio_sink_get_volume(const PulseAudioSink_t *s)
{
	return s->volume.values[0] / (PA_VOLUME_NORM / 100);
}

extern bool
pulseaudio_sink_is_muted(const PulseAudioSink_t *s)
{
	return s->mute;
}

extern int
pulseaudio_sink_list_get_length(PulseAudioSinkList_t *sl)
{
	return sl->len;
}

extern PulseAudioSink_t *
pulseaudio_sink_list_get(PulseAudioSinkList_t *sl, int i)
{
	return sl->sinks[i];
}

extern void
pulseaudio_sink_list_free(PulseAudioSinkList_t *sl)
{
	size_t i;
	for (i = 0; i < sl->len; ++i)
		__pulseaudio_sink_free(sl->sinks[i]);
	free(sl->sinks);
	free(sl);
}

extern void
pulseaudio_disconnect(PulseAudioConnection_t *pac)
{
	pa_context_unref(pac->context);
	pa_threaded_mainloop_unlock(pac->mainloop);
	pa_threaded_mainloop_free(pac->mainloop);
	free(pac);
}
