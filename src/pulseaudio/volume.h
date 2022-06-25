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

#ifndef __XPAVM_PULSEAUDIO_VOLUME_H__
#define __XPAVM_PULSEAUDIO_VOLUME_H__

#include <stdint.h>

#include "connection.h"
#include "sink.h"

extern void
sink_set_volume(struct pulseaudio_connection *pac,
                struct sink *info,
                uint32_t volume);

extern void
sink_set_volume_relative(struct pulseaudio_connection *pac,
                         struct sink *info,
                         int32_t delta);

extern void
sink_set_mute(struct pulseaudio_connection *pac,
              struct sink *info,
              uint32_t mute);

#endif
