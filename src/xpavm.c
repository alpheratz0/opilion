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

	 _______________________
	( bind super+v to xpavm )
	 -----------------------
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/file.h>
#include <xcb/xproto.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include "base/bitmap.h"
#include "base/font.h"
#include "base/linkedlist.h"
#include "pulseaudio/connection.h"
#include "pulseaudio/sink.h"
#include "pulseaudio/volume.h"
#include "ui/sink_selector.h"
#include "util/debug.h"
#include "x11/window.h"

static struct window *window;
static struct pulseaudio_connection *pac;
static struct sink_selector *selector;

static void
key_press_callback(xcb_keysym_t key)
{
	struct sink *sink;
	sink = sink_selector_get_selected(selector);

	switch (key) {
		case XKB_KEY_Escape:
		case XKB_KEY_q:
			window_loop_end(window);
			return;
		case XKB_KEY_m:
			sink_set_mute(pac, sink, !sink->mute);
			break;
		case XKB_KEY_h:
			sink_set_volume_relative(pac, sink, -1);
			break;
		case XKB_KEY_l:
			sink_set_volume_relative(pac, sink, 1);
			break;
		case XKB_KEY_j:
			sink_selector_select_down(selector);
			break;
		case XKB_KEY_k:
			sink_selector_select_up(selector);
			break;
		case XKB_KEY_1: case XKB_KEY_2: case XKB_KEY_3:
		case XKB_KEY_4: case XKB_KEY_5: case XKB_KEY_6:
		case XKB_KEY_7: case XKB_KEY_8: case XKB_KEY_9:
			sink_set_volume(pac, sink, ((key - XKB_KEY_1 + 1) * 10));
			break;
		case XKB_KEY_0:
			sink_set_volume(pac, sink, 100);
			break;
	}

	bitmap_clear(window->bmp, 0x000000);
	sink_selector_render_onto(selector, window->bmp);
	window_force_redraw(window);
}

static void
usage(void)
{
	puts("usage: xpavm [-hv]");
	exit(0);
}

static void
version(void)
{
	puts("xpavm version "VERSION);
	exit(0);
}

static int
is_instance_running(void)
{
	int fd, rc;

	fd = open("/tmp/xpavm.pid", O_CREAT | O_RDWR, 0666);
	rc = flock(fd, LOCK_EX | LOCK_NB);

	return rc && errno == EWOULDBLOCK;
}

int
main(int argc, char **argv)
{
	struct font *font;
	struct linkedlist *sinks;
	struct sink_style snormal, sselected;

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
	sinks = sink_get_all_input_sinks(pac);

	if (linkedlist_length(sinks) == 0)
		die("no sinks were found");

	window = window_create("xpavm", "xpavm");
	font = font_load("Iosevka", 12);
	snormal = sink_style_from(font, 450, 23, 0xffffff, 0x555555, 0x333333);
	sselected = sink_style_from(font, 450, 23, 0xa0e547, 0x5e5eed, 0x333333);
	selector = sink_selector_create(sinks, &snormal, &sselected);

	sink_selector_render_onto(selector, window->bmp);
	window_set_key_press_callback(window, key_press_callback);
	window_loop_start(window);

	sink_list_free(sinks);
	pulseaudio_disconnect(pac);
	font_unload(font);
	sink_selector_free(selector);
	window_free(window);

	return 0;
}
