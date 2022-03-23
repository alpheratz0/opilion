#include <stdlib.h>
#include <string.h>
#include <pulse/pulseaudio.h>
#include <pulse/volume.h>
#include "../util/numdef.h"
#include "../util/debug.h"
#include "../base/linkedlist.h"
#include "connection.h"
#include "sink.h"

static sink_t *
sink_create(const char *application_name, u32 id, u32 volume, u32 mute) {
	sink_t *info;

	if ((info = malloc(sizeof(sink_t)))) {
		info->application_name = strdup(application_name);
		info->id = id;
		info->volume = volume;
		info->mute = mute;

		return info;
	}

	die("error while calling malloc, no memory available");

	return (void *)(0);
}

static void
get_sink_input_info_callback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
	sink_t *info;
	pulseaudio_connection_t *connection;

	connection = (pulseaudio_connection_t *)(userdata);

	if (eol < 0) {
		dief("failed to get sink input information: %s", pa_strerror(pa_context_errno(c)));
		return;
	}

	if (eol > 0) {
		pa_threaded_mainloop_signal(connection->mainloop, 0);
		return;
	}

	if (i) {
		info = sink_create(
			pa_proplist_gets(i->proplist, "application.name"),
			i->index,
			i->volume.values[0] / (PA_VOLUME_NORM / 100),
			i->mute != 0 ? 1 : 0
		);

		linkedlist_append((linkedlist_t **)(&(connection->userdata)), info);
	}
}

extern linkedlist_t *
sink_get_all_input_sinks(pulseaudio_connection_t *connection) {
	connection->userdata = NULL;
	pa_operation_unref(pa_context_get_sink_input_info_list(connection->context, get_sink_input_info_callback, connection));
	pa_threaded_mainloop_wait(connection->mainloop);
	return (linkedlist_t *)(connection->userdata);
}

extern void
sink_list_free(linkedlist_t *sinks) {
	u32 length;
	sink_t *sink;

	length = linkedlist_length(sinks);

	for (u32 i = 0; i < length; ++i) {
		sink = linkedlist_get_as(sinks, i, sink_t);
		free(sink->application_name);
		free(sink);
	}

	linkedlist_free(sinks);
}
