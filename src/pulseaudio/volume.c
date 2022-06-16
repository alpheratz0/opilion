#include <pulse/pulseaudio.h>
#include <pulse/volume.h>

#include "../util/numdef.h"
#include "../util/debug.h"
#include "connection.h"
#include "sink.h"

static void
set_sink_input_volume_cb(pa_context *c, int eol, void *userdata)
{
	pulseaudio_connection_t *pac;

	pac = (pulseaudio_connection_t *)(userdata);

	if (!eol) {
		warnf("pa_context_set_sink_input_volume failed: %s",
				pa_strerror(pa_context_errno(c)));
		pa_threaded_mainloop_signal(pac->mainloop, 0);
	}
}

static void
get_sink_input_info_for_set_volume_cb(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata)
{
	sink_t *info;
	pulseaudio_connection_t *pac;
	pa_operation *o;

	pac = (pulseaudio_connection_t *)(userdata);

	if (eol < 0) {
		warnf("pa_context_get_sink_input_info failed: %s",
				pa_strerror(pa_context_errno(c)));
		pa_threaded_mainloop_signal(pac->mainloop, 0);
		return;
	}

	if (eol > 0) {
		pa_threaded_mainloop_signal(pac->mainloop, 0);
		return;
	}

	if (NULL != i) {
		info = (sink_t *)(pac->userdata);
		pa_cvolume cv = i->volume;
		pa_cvolume_set(&cv, i->channel_map.channels, ((info->volume * (PA_VOLUME_NORM / 100))));

		o = pa_context_set_sink_input_volume(c, info->id, &cv, set_sink_input_volume_cb, pac);

		if (NULL == o) {
			dief("pa_context_set_sink_input_volume failed: %s",
					pa_strerror(pa_context_errno(pac->context)));
		}

		pa_operation_unref(o);
	}
}

extern void
sink_set_volume(pulseaudio_connection_t *pac, sink_t *info, u32 volume)
{
	pa_operation *o;

	info->volume = volume;
	pac->userdata = info;

	o = pa_context_get_sink_input_info(pac->context, info->id, get_sink_input_info_for_set_volume_cb, pac);

	if (NULL == o) {
		dief("pa_context_get_sink_input_info failed: %s",
				pa_strerror(pa_context_errno(pac->context)));
	}

	pa_operation_unref(o);
	pa_threaded_mainloop_wait(pac->mainloop);
}

extern void
sink_set_volume_relative(pulseaudio_connection_t *pac, sink_t *info, i32 relative)
{
	if (relative < 0 && (relative + (i32)(info->volume)) < 0) sink_set_volume(pac, info, 0);
	else if (relative > 0 && (relative + info->volume) > 100) sink_set_volume(pac, info, 100);
	else sink_set_volume(pac, info, info->volume + relative);
}

static void
set_sink_input_mute_cb(pa_context *c, int eol, void *userdata)
{
	pulseaudio_connection_t *pac;

	pac = (pulseaudio_connection_t *)(userdata);

	if (!eol) {
		warnf("pa_context_set_sink_input_mute failed: %s",
				pa_strerror(pa_context_errno(c)));
		pa_threaded_mainloop_signal(pac->mainloop, 0);
	}
}

static void
get_sink_input_info_for_set_mute_cb(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata)
{
	sink_t *info;
	pulseaudio_connection_t *pac;
	pa_operation *o;

	pac = (pulseaudio_connection_t *)(userdata);

	if (eol < 0) {
		warnf("pa_context_get_sink_input_info failed: %s",
				pa_strerror(pa_context_errno(c)));
		pa_threaded_mainloop_signal(pac->mainloop, 0);
		return;
	}

	if (eol > 0) {
		pa_threaded_mainloop_signal(pac->mainloop, 0);
		return;
	}

	if (NULL != i) {
		info = (sink_t *)(pac->userdata);
		o = pa_context_set_sink_input_mute(c, info->id, info->mute, set_sink_input_mute_cb, pac);

		if (NULL == o) {
			dief("pa_context_set_sink_input_mute failed: %s",
					pa_strerror(pa_context_errno(pac->context)));
		}

		pa_operation_unref(o);
	}
}

extern void
sink_set_mute(pulseaudio_connection_t *pac, sink_t *info, u32 mute)
{
	pa_operation *o;

	info->mute = mute;
	pac->userdata = info;
	o = pa_context_get_sink_input_info(pac->context, info->id, get_sink_input_info_for_set_mute_cb, pac);

	if (NULL == o) {
		dief("pa_context_get_sink_input_info failed: %s",
				pa_strerror(pa_context_errno(pac->context)));
	}

	pa_operation_unref(o);
	pa_threaded_mainloop_wait(pac->mainloop);
}
