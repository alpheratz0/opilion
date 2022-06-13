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
