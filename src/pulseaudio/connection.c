#include <stdlib.h>
#include <pulse/pulseaudio.h>

#include "../util/debug.h"
#include "connection.h"

static void
context_state_callback(__attribute__ ((unused)) pa_context *c, void *userdata) {
	pa_threaded_mainloop_signal((pa_threaded_mainloop *)(userdata), 0);
}

extern pulseaudio_connection_t *
pulseaudio_connect() {
	pulseaudio_connection_t *connection;

	if ((connection = malloc(sizeof(pulseaudio_connection_t)))) {
		connection->mainloop = pa_threaded_mainloop_new();
		connection->api = pa_threaded_mainloop_get_api(connection->mainloop);
		connection->context = pa_context_new(connection->api, NULL);

		pa_context_set_state_callback(connection->context, context_state_callback, connection->mainloop);
		pa_threaded_mainloop_lock(connection->mainloop);
		pa_threaded_mainloop_start(connection->mainloop);
		pa_context_connect(connection->context, NULL, PA_CONTEXT_NOFAIL, NULL);

		while (pa_context_get_state(connection->context) != PA_CONTEXT_READY) {
			pa_threaded_mainloop_wait(connection->mainloop);
		}

		return connection;
	}

	die("error while calling malloc, no memory available");

	return (void *)(0);
}

extern void
pulseaudio_disconnect(pulseaudio_connection_t *connection) {
	pa_context_unref(connection->context);
	pa_threaded_mainloop_unlock(connection->mainloop);
	pa_threaded_mainloop_free(connection->mainloop);
	free(connection);
}
