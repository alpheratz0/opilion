/*
	Copyright (C) 2025 <alpheratz99@protonmail.com>

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

*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pixman.h>

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "pixbuf.h"
#include "render-util.h"
#include "utils.h"
#include "icons/generic.c"
#include "icons/microphone.c"
#include "icons/speakers.c"
#include "icon.h"

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

struct Icon {
	const char *name, *path;
	pixman_image_t *img;
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

static Icon_t *
icon_from_pixels(const char *icon_name, const char *icon_path, const uint8_t *pixels, int w, int h)
{
	Icon_t *icon = xmalloc(sizeof(Icon_t));

	// premultiply alpha
	uint32_t *pixels_pa = xmalloc(sizeof(uint32_t) * w * h);
	for (size_t y = 0; y < h; ++y) {
		for (size_t x = 0; x < w; ++x) {
			const uint8_t *base = &pixels[(y * w + x) * 4];
			pixels_pa[y * w + x] = (base[3] << 24) |
				(((base[0] * base[3]) / 0xff) << 16) |
				(((base[1] * base[3]) / 0xff) << 8) |
				((base[2] * base[3]) / 0xff);
		}
	}

	icon->name = xstrdup(icon_name);
	icon->path = xstrdup(str_fallback(icon_path, ""));
	icon->img = pixman_image_create_bits(PIXMAN_a8r8g8b8, w, h, pixels_pa, w * 4);

	return icon;
}

extern const Icon_t *
icon_from_name(const char *icon_name)
{
	static Icon_t *icons_store[16] = {0};

	static const char *icon_dirs[] = {
		"/usr/share/icons/hicolor/48x48/apps/",
		"/usr/share/icons/hicolor/32x32/apps/",
		"/usr/share/icons/hicolor/24x24/apps/",
		"/usr/share/icons/hicolor/16x16/apps/",
		"/usr/share/pixmaps/"
	};

	int empty_slot_index = -1;

	if (icons_store[0] == NULL) {
		icons_store[0] = icon_from_pixels("audio-x-generic", NULL, audio_x_generic_px,
				AUDIO_X_GENERIC_WIDTH, AUDIO_X_GENERIC_HEIGHT);

		icons_store[1] = icon_from_pixels("audio-input-microphone", NULL, audio_input_microphone_px,
				AUDIO_INPUT_MICROPHONE_WIDTH, AUDIO_INPUT_MICROPHONE_HEIGHT);

		icons_store[2] = icon_from_pixels("audio-speakers", NULL, audio_speakers_px,
				AUDIO_SPEAKERS_WIDTH, AUDIO_SPEAKERS_HEIGHT);
	}

	for (size_t i = 0; i < LEN(icons_store); ++i) {
		if (icons_store[i] == NULL)
			empty_slot_index = i;
		else if (strcmp(icons_store[i]->name, icon_name) == 0)
			return icons_store[i];
	}

	if (empty_slot_index == -1)
		return icons_store[0];

	char icon_path[512];
	bool icon_path_found = false;
	for (size_t i = 0; i < LEN(icon_dirs); ++i) {
		snprintf(icon_path, sizeof(icon_path), "%s%s.png", icon_dirs[i], icon_name);
		if (access(icon_path, R_OK) == 0) {
			icon_path_found = true;
			break;
		}
	}

	if (!icon_path_found)
		return icons_store[0];

	int w, h, channels;
	uint8_t *pixels = stbi_load(icon_path, &w, &h, &channels, 4);
	if (!pixels) return icons_store[0];

	icons_store[empty_slot_index] = icon_from_pixels(icon_name, icon_path, pixels, w, h);
	stbi_image_free(pixels);

	return icons_store[empty_slot_index];
}

extern void
icon_render_to(const Icon_t *icon, Pixbuf_t *pb, int x, int y, int w, int h)
{
	int src_w = pixman_image_get_width(icon->img);
	int src_h = pixman_image_get_height(icon->img);

	pixman_transform_t transform;
	pixman_transform_init_identity(&transform);

	pixman_fixed_t scale_x = pixman_double_to_fixed((double)src_w / w);
	pixman_fixed_t scale_y = pixman_double_to_fixed((double)src_h / h);

	pixman_transform_scale(&transform, NULL, scale_x, scale_y);
	pixman_image_set_transform(icon->img, &transform);

	pixman_image_set_filter(icon->img, PIXMAN_FILTER_BILINEAR, NULL, 0);

	pixman_image_composite32(
		PIXMAN_OP_OVER,
		icon->img,
		NULL,
		pixbuf_get_pixman_image(pb),
		0, 0,
		0, 0,
		x, y,
		w, h
	);
}
