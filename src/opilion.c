/*
	Copyright (C) 2022-2024 <alpheratz99@protonmail.com>

	This program is free software; you can redistribute it and/or modify it
	under the terms of the GNU General Public License version 2 as published by
	the Free Software Foundation.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
	FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
	more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc., 59
	Temple Place, Suite 330, Boston, MA 02111-1307 USA

	 _________________________
	( bind super+v to opilion )
	 -------------------------
	    o               ,-----._
	  .  o         .  ,'        `-.__,------._
	 //   o      __\\'                        `-.
	((    _____-'___))                           |
	 `:='/     (alf_/                            |
	 `.=|      |='                               |
	    |)   O |                                  \
	    |      |                               /\  \
	    |     /                          .    /  \  \
	    |    .-..__            ___   .--' \  |\   \  |
	   |o o  |     ``--.___.  /   `-'      \  \\   \ |
	    `--''        '  .' / /             |  | |   | \
	                 |  | / /              |  | |   mmm
	                 |  ||  |              | /| |
	                 ( .' \ \              || | |
	                 | |   \ \            // / /
	                 | |    \ \          || |_|
	                /  |    |_/         /_|
	               /__/

*/

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_keysyms.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xcb/xkb.h>
#include <sys/file.h>

#include "pa.h"
#include "utils.h"
#include "sink-selector.h"
#include "text-renderer.h"
#include "pixbuf.h"
#include "log.h"

static Pixbuf_t *pb;
static PulseAudioConnection_t *pac;
static PulseAudioSinkList_t *sinks;
static SinkSelector_t *sink_selector;
static xcb_connection_t *conn;
static xcb_screen_t *scr;
static xcb_window_t win, revert_focus;
static xcb_key_symbols_t *ksyms;
static bool should_close;

#define OPILION_WM_NAME "opilion"
#define OPILION_WM_CLASS "opilion\0opilion\0"

static xcb_atom_t
get_x11_atom(const char *name)
{
	xcb_atom_t atom;
	xcb_generic_error_t *error;
	xcb_intern_atom_cookie_t cookie;
	xcb_intern_atom_reply_t *reply;

	cookie = xcb_intern_atom(conn, 0, strlen(name), name);
	reply = xcb_intern_atom_reply(conn, cookie, &error);

	if (NULL != error)
		die("xcb_intern_atom failed with error code: %hhu",
				error->error_code);

	atom = reply->atom;
	free(reply);

	return atom;
}

static xcb_window_t
get_focused_window(void)
{
	xcb_window_t win;
	xcb_generic_error_t *error;
	xcb_get_input_focus_cookie_t cookie;
	xcb_get_input_focus_reply_t *reply;

	cookie = xcb_get_input_focus(conn);
	reply = xcb_get_input_focus_reply(conn, cookie, &error);

	if (NULL != error)
		die("xcb_get_input_focus failed with error code: %hhu",
				error->error_code);

	win = reply->focus;
	free(reply);

	return win;
}

static void
set_focused_window(xcb_window_t win)
{
	xcb_generic_error_t *error;
	xcb_void_cookie_t cookie;

	cookie = xcb_set_input_focus_checked(conn,
			XCB_INPUT_FOCUS_POINTER_ROOT, win, XCB_CURRENT_TIME);
	xcb_flush(conn);
	error = xcb_request_check(conn, cookie);

	if (NULL != error)
		die("xcb_set_input_focus failed with error code: %hhu",
				error->error_code);
}

static bool
grab_keyboard(void)
{
	bool grabbed;
	xcb_generic_error_t *error;
	xcb_grab_keyboard_cookie_t cookie;
	xcb_grab_keyboard_reply_t *reply;

	grabbed = false;
	cookie = xcb_grab_keyboard(conn, 1, win, XCB_CURRENT_TIME,
			XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	reply = xcb_grab_keyboard_reply(conn, cookie, &error);

	if (NULL != error)
		die("xcb_grab_keyboard failed with error code: %hhu",
				error->error_code);

	grabbed = NULL != reply && reply->status == XCB_GRAB_STATUS_SUCCESS;
	free(reply);

	return grabbed;
}

static void
xwininit(void)
{
	xcb_atom_t _NET_WM_NAME;

	xcb_atom_t WM_PROTOCOLS,
			   WM_DELETE_WINDOW;

	xcb_atom_t UTF8_STRING;

	uint32_t override_redirect;

	int grab_attempt;

	conn = xcb_connect(NULL, NULL);

	if (xcb_connection_has_error(conn))
		die("can't open display");

	scr = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;

	if (NULL == scr)
		die("can't get default screen");

	revert_focus = get_focused_window();

	ksyms = xcb_key_symbols_alloc(conn);
	win = xcb_generate_id(conn);

	xcb_create_window_aux(
		conn, scr->root_depth, win, scr->root, 0, 0,
		scr->width_in_pixels, scr->height_in_pixels,
		0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
		scr->root_visual, XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
		(const xcb_create_window_value_list_t []) {{
			.background_pixel = 0x000000,
			.event_mask = XCB_EVENT_MASK_EXPOSURE |
			              XCB_EVENT_MASK_KEY_PRESS |
			              XCB_EVENT_MASK_STRUCTURE_NOTIFY
		}}
	);

	_NET_WM_NAME = get_x11_atom("_NET_WM_NAME");
	UTF8_STRING = get_x11_atom("UTF8_STRING");

	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, win, _NET_WM_NAME,
			UTF8_STRING, 8, sizeof(OPILION_WM_NAME) - 1, OPILION_WM_NAME);

	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, win, XCB_ATOM_WM_CLASS,
		XCB_ATOM_STRING, 8, sizeof(OPILION_WM_CLASS) - 1, OPILION_WM_CLASS);

	WM_PROTOCOLS = get_x11_atom("WM_PROTOCOLS");
	WM_DELETE_WINDOW = get_x11_atom("WM_DELETE_WINDOW");

	xcb_change_property(conn, XCB_PROP_MODE_REPLACE, win,
		WM_PROTOCOLS, XCB_ATOM_ATOM, 32, 1, &WM_DELETE_WINDOW);

	override_redirect = 1;
	xcb_change_window_attributes(conn, win, XCB_CW_OVERRIDE_REDIRECT,
		&override_redirect);

	xcb_xkb_use_extension(conn, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);

	xcb_xkb_per_client_flags(
		conn, XCB_XKB_ID_USE_CORE_KBD,
		XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT, 1, 0, 0, 0
	);

	pb = pixbuf_new(conn, win, 500, 1000);
	pixbuf_set_container_size(pb, scr->width_in_pixels, scr->height_in_pixels);

	xcb_map_window(conn, win);

	for (grab_attempt = 10; grab_attempt >= 1; --grab_attempt) {
		if (grab_keyboard()) break;
		if (grab_attempt == 1) die("failed to grab keyboard");
		usleep(100000);
	}

	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, win,
			XCB_CURRENT_TIME);
	xcb_flush(conn);
}

static void
xwindestroy(void)
{
	set_focused_window(revert_focus);
	pixbuf_free(pb);
	xcb_key_symbols_free(ksyms);
	xcb_destroy_window(conn, win);
	xcb_disconnect(conn);
}

static void
h_client_message(xcb_client_message_event_t *ev)
{
	xcb_atom_t WM_DELETE_WINDOW;

	WM_DELETE_WINDOW = get_x11_atom("WM_DELETE_WINDOW");

	/* check if the wm sent a delete window message */
	/* https://www.x.org/docs/ICCCM/icccm.pdf */
	if (ev->data.data32[0] == WM_DELETE_WINDOW)
		should_close = true;
}

static void
h_expose(UNUSED xcb_expose_event_t *ev)
{
	pixbuf_render(pb);
}

static void
h_key_press(xcb_key_press_event_t *ev)
{
	xcb_keysym_t key;
	PulseAudioSink_t *sink;

	key = xcb_key_symbols_get_keysym(ksyms, ev->detail, 0);

	sink = sink_selector_get_selected(sink_selector);

	switch (key) {
	case XKB_KEY_Escape:
	case XKB_KEY_q:
		should_close = true;
		return;
	case XKB_KEY_m:
		pulseaudio_sink_toggle_mute(pac, sink);
		break;
	case XKB_KEY_h:
		pulseaudio_sink_increase_volume(pac, sink, -1);
		break;
	case XKB_KEY_l:
		pulseaudio_sink_increase_volume(pac, sink, 1);
		break;
	case XKB_KEY_j:
		sink_selector_select_down(sink_selector);
		break;
	case XKB_KEY_k:
		sink_selector_select_up(sink_selector);
		break;
	case XKB_KEY_F5:
		pulseaudio_sink_list_free(sinks);
		sinks = pulseaudio_get_all_input_sinks(pac);
		sink_selector_set_sink_list(sink_selector, sinks);
		break;
	case XKB_KEY_0: case XKB_KEY_1:
	case XKB_KEY_2: case XKB_KEY_3:
	case XKB_KEY_4: case XKB_KEY_5:
	case XKB_KEY_6: case XKB_KEY_7:
	case XKB_KEY_8: case XKB_KEY_9:
		pulseaudio_sink_set_volume(pac, sink,
				(((key - XKB_KEY_0 + 9) % 10 + 1) * 10));
		break;
	default:
		return;
	}

	pixbuf_clear(pb, 0x000000);
	sink_selector_render_to(sink_selector, pb);
	pixbuf_render(pb);
}

static void
h_configure_notify(xcb_configure_notify_event_t *ev)
{
	pixbuf_set_container_size(pb, ev->width, ev->height);
}

static void
h_mapping_notify(xcb_mapping_notify_event_t *ev)
{
	if (ev->count > 0)
		xcb_refresh_keyboard_mapping(ksyms, ev);
}

static void
usage(void)
{
	puts("usage: opilion [-hv]");
	exit(0);
}

static void
version(void)
{
	puts("opilion version "VERSION);
	exit(0);
}

static int
is_instance_running(void)
{
	int fd, rc;

	fd = open("/tmp/opilion.pid", O_CREAT | O_RDWR, 0666);
	rc = flock(fd, LOCK_EX | LOCK_NB);

	return rc && errno == EWOULDBLOCK;
}

int
main(int argc, char **argv)
{
	TextRenderer_t *text_renderer;
	SinkColorTheme_t ct_nor, ct_sel;
	xcb_generic_event_t *ev;

	while (++argv, --argc > 0) {
		if ((*argv)[0] == '-' && (*argv)[1] != '\0' && (*argv)[2] == '\0') {
			switch ((*argv)[1]) {
			case 'h': usage(); break;
			case 'v': version(); break;
			default: die("invalid option %s", *argv); break;
			}
		} else {
			die("unexpected argument: %s", *argv);
		}
	}

	if (is_instance_running())
		die("another instance is already running");

	pac = pulseaudio_connect();
	sinks = pulseaudio_get_all_input_sinks(pac);

	if (pulseaudio_sink_list_get_length(sinks) == 0)
		die("no sinks were found");

	xwininit();

	text_renderer = text_renderer_new("Terminus", 12);

	ct_nor = sink_color_theme_from(0xffffff, 0xffffff,
			(const uint32_t []){ 0x333333, 0x555555 });

	ct_sel = sink_color_theme_from(0xa0e547, 0xa0e547,
			(const uint32_t []){ 0x333333, 0x5e5eed });

	sink_selector = sink_selector_new(sinks, text_renderer, &ct_nor, &ct_sel);

	sink_selector_render_to(sink_selector, pb);

	while (!should_close && (ev = xcb_wait_for_event(conn))) {
		switch (ev->response_type & ~0x80) {
		case XCB_CLIENT_MESSAGE:     h_client_message((void *)(ev)); break;
		case XCB_EXPOSE:             h_expose((void *)(ev)); break;
		case XCB_KEY_PRESS:          h_key_press((void *)(ev)); break;
		case XCB_CONFIGURE_NOTIFY:   h_configure_notify((void *)(ev)); break;
		case XCB_MAPPING_NOTIFY:     h_mapping_notify((void *)(ev)); break;
		}

		free(ev);
	}

	pulseaudio_sink_list_free(sinks);
	pulseaudio_disconnect(pac);
	text_renderer_free(text_renderer);
	sink_selector_free(sink_selector);
	xwindestroy();

	return 0;
}
