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

#ifndef __XPAVM_PULSEAUDIO_CONNECTION_H__
#define __XPAVM_PULSEAUDIO_CONNECTION_H__

#include <pulse/pulseaudio.h>

typedef struct pulseaudio_connection pulseaudio_connection_t;

struct pulseaudio_connection {
	pa_threaded_mainloop *mainloop;
	pa_mainloop_api *api;
	pa_context *context;
	void *userdata;
};

extern pulseaudio_connection_t *
pulseaudio_connect(void);

extern void
pulseaudio_disconnect(pulseaudio_connection_t *pac);

#endif
