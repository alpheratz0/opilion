#ifndef __XPAVM_PULSEAUDIO_SINK_H__
#define __XPAVM_PULSEAUDIO_SINK_H__

#include <stdint.h>

#include "../base/linkedlist.h"
#include "connection.h"

typedef struct sink sink_t;

struct sink {
	char *appname;
	uint32_t id;
	uint32_t volume;
	uint32_t mute;
};

extern linkedlist_t *
sink_get_all_input_sinks(pulseaudio_connection_t *pac);

extern void
sink_list_free(linkedlist_t *sinks);

#endif
