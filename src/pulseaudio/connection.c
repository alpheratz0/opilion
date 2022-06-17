#include <stdlib.h>
#include <pulse/pulseaudio.h>

#include "../util/debug.h"
#include "connection.h"

static void
context_state_cb(__attribute__((unused)) pa_context *c, void *userdata)
{
	pa_threaded_mainloop *mainloop;

	mainloop = userdata;

	pa_threaded_mainloop_signal(mainloop, 0);
}

extern pulseaudio_connection_t *
pulseaudio_connect(void)
{
	pulseaudio_connection_t *pac;

	if (NULL == (pac = malloc(sizeof(pulseaudio_connection_t)))) {
		die("error while calling malloc, no memory available");
	}

	pac->mainloop = pa_threaded_mainloop_new();
	pac->api = pa_threaded_mainloop_get_api(pac->mainloop);
	pac->context = pa_context_new(pac->api, NULL);

	pa_context_set_state_callback(pac->context, context_state_cb, pac->mainloop);
	pa_threaded_mainloop_lock(pac->mainloop);
	pa_threaded_mainloop_start(pac->mainloop);
	pa_context_connect(pac->context, NULL, PA_CONTEXT_NOFAIL, NULL);

	while (pa_context_get_state(pac->context) != PA_CONTEXT_READY) {
		pa_threaded_mainloop_wait(pac->mainloop);
	}

	return pac;
}

extern void
pulseaudio_disconnect(pulseaudio_connection_t *pac)
{
	pa_context_unref(pac->context);
	pa_threaded_mainloop_unlock(pac->mainloop);
	pa_threaded_mainloop_free(pac->mainloop);
	free(pac);
}
