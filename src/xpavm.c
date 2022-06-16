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
#include <stdbool.h>
#include <string.h>
#include <sys/file.h>

#include "base/bitmap.h"
#include "base/font.h"
#include "base/linkedlist.h"
#include "pulseaudio/connection.h"
#include "pulseaudio/sink.h"
#include "pulseaudio/volume.h"
#include "ui/sink_selector.h"
#include "util/debug.h"
#include "util/numdef.h"
#include "x11/keys.h"
#include "x11/window.h"

static window_t *window;
static pulseaudio_connection_t *pac;
static sink_selector_t *selector;

static bool
match_opt(const char *in, const char *sh, const char *lo)
{
	return (strcmp(in, sh) == 0) ||
		   (strcmp(in, lo) == 0);
}

static void
key_press_callback(u32 key)
{
	sink_t *sink;
	sink = sink_selector_get_selected(selector);

	switch (key) {
		case KEY_ESCAPE:
		case KEY_Q:
			window_loop_end(window);
			return;
		case KEY_M:
			sink_set_mute(pac, sink, !sink->mute);
			break;
		case KEY_H:
			sink_set_volume_relative(pac, sink, -1);
			break;
		case KEY_L:
			sink_set_volume_relative(pac, sink, 1);
			break;
		case KEY_J:
			sink_selector_select_down(selector);
			break;
		case KEY_K:
			sink_selector_select_up(selector);
			break;
		case KEY_1: case KEY_2: case KEY_3: case KEY_4: case KEY_5:
		case KEY_6: case KEY_7: case KEY_8: case KEY_9: case KEY_0:
			sink_set_volume(pac, sink, (key - KEY_1 + 1) * 10);
			break;
	}

	bitmap_clear(window->bmp, 0x000000);
	sink_selector_render_onto(selector, window->bmp);
	window_force_redraw(window);
}

static inline void
print_opt(const char *sh, const char *lo, const char *desc)
{
	printf("%7s | %-25s %s\n", sh, lo, desc);
}

static void
usage(void)
{
	puts("Usage: xpavm [ -hkv ]");
	puts("Options are:");
	print_opt("-h", "--help", "display this message and exit");
	print_opt("-k", "--keybindings", "display the keybindings");
	print_opt("-v", "--version", "display the program version");
	exit(0);
}

static void
keybindings(void)
{
	puts("Keybindings are:");
	puts("j/k: sink selection");
	puts("h/l: decrease/increase volume");
	puts("m: toggle mute");
	puts("q/esc: exit");
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
	int pidfd, rc;

	pidfd = open("/tmp/xpavm.pid", O_CREAT | O_RDWR, 0666);
	rc = flock(pidfd, LOCK_EX | LOCK_NB);

	return rc && errno == EWOULDBLOCK;
}

int
main(int argc, char **argv)
{
	font_t *font;
	linkedlist_t *sinks;
	sink_style_t snormal, sselected;

	if (++argv, --argc > 0) {
		if (match_opt(*argv, "-k", "--keybindings")) keybindings();
		else if (match_opt(*argv, "-h", "--help")) usage();
		else if (match_opt(*argv, "-v", "--version")) version();
		else if (**argv == '-') dief("invalid option %s", *argv);
		else dief("unexpected argument: %s", *argv);
	}

	if (is_instance_running()) {
		die("another instance is already running");
	}

	pac = pulseaudio_connect();
	sinks = sink_get_all_input_sinks(pac);

	if (linkedlist_length(sinks) == 0) {
		die("no sinks were found");
	}

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
