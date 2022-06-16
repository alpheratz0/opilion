#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pulse/pulseaudio.h>
#include <pulse/volume.h>

#include "../util/debug.h"
#include "../base/linkedlist.h"
#include "connection.h"
#include "sink.h"

static sink_t *
sink_create(const char *appname, uint32_t id, uint32_t volume, uint32_t mute)
{
	sink_t *sink;

	if (NULL == (sink = malloc(sizeof(sink_t)))) {
		die("error while calling malloc, no memory available");
	}

	sink->appname = strdup(appname);
	sink->id = id;
	sink->volume = volume;
	sink->mute = mute;

	return sink;
}

static void
get_sink_input_info_cb(pa_context *c,
                       const pa_sink_input_info *i,
                       int eol,
                       void *userdata)
{
	sink_t *sink;
	pulseaudio_connection_t *pac;

	pac = userdata;

	if (eol < 0) {
		dief("failed to get sink input information: %s",
				pa_strerror(pa_context_errno(c)));
	}

	if (eol > 0) {
		pa_threaded_mainloop_signal(pac->mainloop, 0);

		return;
	}

	if (NULL != i) {
		sink = sink_create(
			pa_proplist_gets(i->proplist, "application.name"),
			i->index,
			i->volume.values[0] / (PA_VOLUME_NORM / 100),
			i->mute != 0 ? 1 : 0
		);

		linkedlist_append((linkedlist_t **)(&pac->userdata), sink);
	}
}

extern linkedlist_t *
sink_get_all_input_sinks(pulseaudio_connection_t *pac)
{
	pa_operation *po;
	linkedlist_t *sinks;

	pac->userdata = NULL;

	po = pa_context_get_sink_input_info_list(
		pac->context, get_sink_input_info_cb, pac
	);

	if (NULL == po) {
		dief("pa_context_get_sink_input_info_list failed: %s",
				pa_strerror(pa_context_errno(pac->context)));
	}

	pa_operation_unref(po);
	pa_threaded_mainloop_wait(pac->mainloop);

	sinks = pac->userdata;

	return sinks;
}

extern void
sink_list_free(linkedlist_t *sinks)
{
	linkedlist_t *temp;
	sink_t *sink;

	temp = sinks;

	while (NULL != temp) {
		sink = temp->data;
		temp = temp->next;
		free(sink->appname);
		free(sink);
	}

	linkedlist_free(sinks);
}
