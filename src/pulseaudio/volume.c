#include <pulse/pulseaudio.h>
#include <pulse/volume.h>
#include "../util/numdef.h"
#include "../util/debug.h"
#include "connection.h"
#include "sink.h"

static void
set_sink_input_volume_callback(pa_context *c, int eol, void *userdata) {
	pulseaudio_connection_t *connection;

	connection = (pulseaudio_connection_t *)(userdata);

	if (!eol) {
		warnf("pa_context_set_sink_input_volume failed: %s", pa_strerror(pa_context_errno(c)));
		pa_threaded_mainloop_signal(connection->mainloop, 0);
	}
}

static void
get_sink_input_info_for_set_volume_callback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
	sink_t *info;
	pulseaudio_connection_t *connection;

	connection = (pulseaudio_connection_t *)(userdata);

	if (eol < 0) {
		warnf("pa_context_get_sink_input_info failed: %s", pa_strerror(pa_context_errno(c)));
		pa_threaded_mainloop_signal(connection->mainloop, 0);
		return;
	}

	if (eol > 0) {
		pa_threaded_mainloop_signal(connection->mainloop, 0);
		return;
	}

	if (i) {
		info = (sink_t *)(connection->userdata);
		pa_cvolume cv = i->volume;
		pa_cvolume_set(&cv, i->channel_map.channels, ((info->volume * (PA_VOLUME_NORM / 100))));
		pa_operation_unref(pa_context_set_sink_input_volume(c, info->id, &cv, set_sink_input_volume_callback, connection));
	}

}

extern void
sink_set_volume(pulseaudio_connection_t *connection, sink_t *info, u32 volume) {
	info->volume = volume;
	connection->userdata = info;
	pa_operation_unref(pa_context_get_sink_input_info(connection->context, info->id, get_sink_input_info_for_set_volume_callback, connection));
	pa_threaded_mainloop_wait(connection->mainloop);
}

extern void
sink_set_volume_relative(pulseaudio_connection_t *connection, sink_t *info, i32 relative) {
	if (relative < 0 && (relative + (i32)(info->volume)) < 0) sink_set_volume(connection, info, 0);
	else if(relative > 0 && (relative + info->volume) > 100) sink_set_volume(connection, info, 100);
	else sink_set_volume(connection, info, info->volume + relative);
}

static void
set_sink_input_mute_callback(pa_context *c, int eol, void *userdata) {
	pulseaudio_connection_t *connection;

	connection = (pulseaudio_connection_t *)(userdata);

	if (!eol) {
		warnf("pa_context_set_sink_input_mute failed: %s", pa_strerror(pa_context_errno(c)));
		pa_threaded_mainloop_signal(connection->mainloop, 0);
	}
}

static void
get_sink_input_info_for_set_mute_callback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
	sink_t *info;
	pulseaudio_connection_t *connection;

	connection = (pulseaudio_connection_t *)(userdata);

	if (eol < 0) {
		warnf("pa_context_get_sink_input_info failed: %s", pa_strerror(pa_context_errno(c)));
		pa_threaded_mainloop_signal(connection->mainloop, 0);
		return;
	}

	if (eol > 0) {
		pa_threaded_mainloop_signal(connection->mainloop, 0);
		return;
	}

	if (i) {
		info = (sink_t *)(connection->userdata);
		pa_operation_unref(pa_context_set_sink_input_mute(c, info->id, info->mute, set_sink_input_mute_callback, connection));
	}
}

extern void
sink_set_mute(pulseaudio_connection_t *connection, sink_t *info, u32 mute) {
	info->mute = mute;
	connection->userdata = info;
	pa_operation_unref(pa_context_get_sink_input_info(connection->context, info->id, get_sink_input_info_for_set_mute_callback, connection));
	pa_threaded_mainloop_wait(connection->mainloop);
}
