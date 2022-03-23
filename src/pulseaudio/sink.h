#ifndef __XPAVM_PULSEAUDIO_SINK_H__
#define __XPAVM_PULSEAUDIO_SINK_H__

#include "../base/linkedlist.h"
#include "../util/numdef.h"
#include "../util/strdef.h"
#include "connection.h"

typedef struct sink sink_t;

struct sink {
	str application_name;
	u32 id;
	u32 volume;
	u32 mute;
};

extern linkedlist_t *
sink_get_all_input_sinks(pulseaudio_connection_t *connection);

extern void
sink_list_free(linkedlist_t *sinks);

#endif
