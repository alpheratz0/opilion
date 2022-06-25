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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pulse/pulseaudio.h>
#include <pulse/volume.h>

#include "../util/debug.h"
#include "../util/xmalloc.h"
#include "../base/linkedlist.h"
#include "connection.h"
#include "sink.h"

static struct sink *
sink_create(const char *appname, uint32_t id, uint32_t volume, uint32_t mute)
{
	struct sink *sink;

	sink = xmalloc(sizeof(struct sink));

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
	struct sink *sink;
	struct pulseaudio_connection *pac;

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

		linkedlist_append((struct linkedlist **)(&pac->userdata), sink);
	}
}

extern struct linkedlist *
sink_get_all_input_sinks(struct pulseaudio_connection *pac)
{
	pa_operation *po;
	struct linkedlist *sinks;

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
sink_list_free(struct linkedlist *sinks)
{
	struct linkedlist *temp;
	struct sink *sink;

	temp = sinks;

	while (NULL != temp) {
		sink = temp->data;
		temp = temp->next;
		free(sink->appname);
		free(sink);
	}

	linkedlist_free(sinks);
}
