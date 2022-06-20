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

#include <stdint.h>
#include <pulse/pulseaudio.h>
#include <pulse/volume.h>

#include "../util/debug.h"
#include "connection.h"
#include "sink.h"

static void
set_sink_input_volume_cb(pa_context *c, int eol, void *userdata)
{
	pulseaudio_connection_t *pac;

	pac = userdata;

	if (!eol) {
		warnf("pa_context_set_sink_input_volume failed: %s",
				pa_strerror(pa_context_errno(c)));

		pa_threaded_mainloop_signal(pac->mainloop, 0);
	}
}

static void
get_sink_input_info_for_set_volume_cb(pa_context *c,
                                      const pa_sink_input_info *i,
                                      int eol,
                                      void *userdata)
{
	sink_t *info;
	pulseaudio_connection_t *pac;
	pa_operation *po;
	pa_cvolume cv;

	pac = userdata;

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
		info = pac->userdata;
		cv = i->volume;

		pa_cvolume_set(
			&cv, i->channel_map.channels,
			((info->volume * (PA_VOLUME_NORM / 100)))
		);

		po = pa_context_set_sink_input_volume(
			c, info->id, &cv,
			set_sink_input_volume_cb, pac
		);

		if (NULL == po) {
			dief("pa_context_set_sink_input_volume failed: %s",
					pa_strerror(pa_context_errno(pac->context)));
		}

		pa_operation_unref(po);
	}
}

extern void
sink_set_volume(pulseaudio_connection_t *pac, sink_t *info, uint32_t volume)
{
	pa_operation *po;

	info->volume = volume;
	pac->userdata = info;

	po = pa_context_get_sink_input_info(
		pac->context, info->id,
		get_sink_input_info_for_set_volume_cb, pac
	);

	if (NULL == po) {
		dief("pa_context_get_sink_input_info failed: %s",
				pa_strerror(pa_context_errno(pac->context)));
	}

	pa_operation_unref(po);
	pa_threaded_mainloop_wait(pac->mainloop);
}

extern void
sink_set_volume_relative(pulseaudio_connection_t *pac,
                         sink_t *info,
                         int32_t delta)
{
#define clamp(v,min,max) (v>max?max:v<min?min:v)

	int32_t volume;

	volume = delta + (int32_t)(info->volume);
	volume = clamp(volume, 0, 100);

#undef clamp

	sink_set_volume(pac, info, (uint32_t)(volume));
}

static void
set_sink_input_mute_cb(pa_context *c, int eol, void *userdata)
{
	pulseaudio_connection_t *pac;

	pac = userdata;

	if (!eol) {
		warnf("pa_context_set_sink_input_mute failed: %s",
				pa_strerror(pa_context_errno(c)));

		pa_threaded_mainloop_signal(pac->mainloop, 0);
	}
}

static void
get_sink_input_info_for_set_mute_cb(pa_context *c,
                                    const pa_sink_input_info *i,
                                    int eol,
                                    void *userdata)
{
	sink_t *info;
	pulseaudio_connection_t *pac;
	pa_operation *po;

	pac = userdata;

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
		info = pac->userdata;

		po = pa_context_set_sink_input_mute(
			c, info->id, info->mute,
			set_sink_input_mute_cb, pac
		);

		if (NULL == po) {
			dief("pa_context_set_sink_input_mute failed: %s",
					pa_strerror(pa_context_errno(pac->context)));
		}

		pa_operation_unref(po);
	}
}

extern void
sink_set_mute(pulseaudio_connection_t *pac, sink_t *info, uint32_t mute)
{
	pa_operation *po;

	info->mute = mute;
	pac->userdata = info;

	po = pa_context_get_sink_input_info(
		pac->context, info->id,
		get_sink_input_info_for_set_mute_cb, pac
	);

	if (NULL == po) {
		dief("pa_context_get_sink_input_info failed: %s",
				pa_strerror(pa_context_errno(pac->context)));
	}

	pa_operation_unref(po);
	pa_threaded_mainloop_wait(pac->mainloop);
}
