#ifndef __XPAVM_PULSEAUDIO_VOLUME_H__
#define __XPAVM_PULSEAUDIO_VOLUME_H__

#include "../util/numdef.h"
#include "connection.h"
#include "sink.h"

extern void
sink_set_volume(pulseaudio_connection_t *pac, sink_t *info, u32 volume);

extern void
sink_set_volume_relative(pulseaudio_connection_t *pac, sink_t *info, i32 relative);

extern void
sink_set_mute(pulseaudio_connection_t *pac, sink_t *info, u32 mute);

#endif
