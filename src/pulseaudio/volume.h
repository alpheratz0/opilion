#ifndef __XPAVM_PULSEAUDIO_VOLUME_H__
#define __XPAVM_PULSEAUDIO_VOLUME_H__

#include <stdint.h>

#include "connection.h"
#include "sink.h"

extern void
sink_set_volume(pulseaudio_connection_t *pac, sink_t *info, uint32_t volume);

extern void
sink_set_volume_relative(pulseaudio_connection_t *pac,
                         sink_t *info,
                         int32_t relative);

extern void
sink_set_mute(pulseaudio_connection_t *pac, sink_t *info, uint32_t mute);

#endif
