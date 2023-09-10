/*
	Copyright (C) 2022-2023 <alpheratz99@protonmail.com>

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

#include <pixman.h>
#include <sys/shm.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_image.h>
#include <xcb/shm.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "pixbuf.h"
#include "utils.h"
#include "log.h"

#define SHM_INVALID_MEM ((void *)(-1))

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

typedef struct {
	xcb_drawable_t drawable;
	int width, height;
} PixbufContainer_t;

struct Pixbuf {
	xcb_connection_t *conn;
	xcb_screen_t *scr;
	xcb_gcontext_t gc;
	PixbufContainer_t cont;
	float x;
	float y;
	int width;
	int height;
	uint32_t *px;
	pixman_image_t *pixman_image;
	bool is_shm;
	union {
		struct {
			int id;
			xcb_shm_seg_t seg;
			xcb_pixmap_t pixmap;
		} shm;
		xcb_image_t *image;
	};
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

static bool
__is_shm_extension_available(xcb_connection_t *conn)
{
	xcb_generic_error_t *error;
	xcb_shm_query_version_cookie_t cookie;
	xcb_shm_query_version_reply_t *reply;
	bool supported;

	cookie = xcb_shm_query_version(conn);
	reply = xcb_shm_query_version_reply(conn, cookie, &error);
	supported = !error && reply && reply->shared_pixmaps;

	free(error); free(reply);

	return supported;
}

static xcb_screen_t *
__get_default_screen(xcb_connection_t *conn)
{
	const xcb_setup_t *setup;
	xcb_screen_iterator_t scr_iter;
	xcb_screen_t *scr;

	setup = xcb_get_setup(conn);
	scr_iter = xcb_setup_roots_iterator(setup);
	scr = scr_iter.data;

	if (NULL == scr) {
		die("can't get default screen");
	}

	return scr;
}

extern Pixbuf_t *
pixbuf_new(xcb_connection_t *conn, xcb_drawable_t container, int w, int h)
{
	Pixbuf_t *pb;

	pb = xcalloc(1, sizeof(Pixbuf_t));

	pb->conn = conn;
	pb->scr = __get_default_screen(conn);
	pb->gc = xcb_generate_id(conn);
	pb->cont.drawable = container;

	xcb_create_gc(conn, pb->gc, container, 0, NULL);

	if (__is_shm_extension_available(conn)) {

		pb->is_shm = true;

		pb->shm.seg = xcb_generate_id(conn);
		pb->shm.pixmap = xcb_generate_id(conn);
		pb->shm.id = shmget(IPC_PRIVATE, w*h*4, IPC_CREAT | 0600);

		if (pb->shm.id < 0) die("shmget:");

		pb->px = shmat(pb->shm.id, NULL, 0);

		if (SHM_INVALID_MEM == pb->px) {
			shmctl(pb->shm.id, IPC_RMID, NULL);
			die("shmat:");
		}

		xcb_shm_attach(conn, pb->shm.seg, pb->shm.id, 0);
		xcb_shm_create_pixmap(conn, pb->shm.pixmap, container,
				w, h, pb->scr->root_depth, pb->shm.seg, 0);

	} else {

		pb->is_shm = false;

		w = CLAMP(w, 0, 2000);
		h = CLAMP(h, 0, 2000);

		pb->px = xmalloc(w*h*4);

		pb->image = xcb_image_create_native(conn, w, h,
				XCB_IMAGE_FORMAT_Z_PIXMAP, pb->scr->root_depth,
				pb->px, w*h*4, (uint8_t *)pb->px);

	}

	pb->pixman_image = pixman_image_create_bits_no_clear(
			PIXMAN_a8r8g8b8, w, h, pb->px, w*4);

	pb->width  = pb->cont.width  = w;
	pb->height = pb->cont.height = h;

	return pb;
}

extern int
pixbuf_get_width(const Pixbuf_t *pb)
{
	return pb->width;
}

extern int
pixbuf_get_height(const Pixbuf_t *pb)
{
	return pb->height;
}

extern pixman_image_t *
pixbuf_get_pixman_image(const Pixbuf_t *pb)
{
	return pb->pixman_image;
}

extern void
pixbuf_set_container_size(Pixbuf_t *pb, int cw, int ch)
{
	pb->x += ((float)cw - pb->cont.width) / 2;
	pb->y += ((float)ch - pb->cont.height) / 2;
	pb->cont.width  = cw;
	pb->cont.height = ch;
}

extern void
pixbuf_rect(Pixbuf_t *pb, int x, int y, int w, int h, uint32_t color)
{
	int dx, dy;

	if (x < 0) w += x, x = 0;
	if (y < 0) h += y, y = 0;
	if (x + w >= pb->width) w = pb->width - x;
	if (y + h >= pb->height) h = pb->height - y;

	for (dy = 0; dy < h; ++dy)
		for (dx = 0; dx < w; ++dx)
			pb->px[(y+dy)*pb->width+x+dx] = color;
}

extern void
pixbuf_clear(Pixbuf_t *pb, uint32_t color)
{
	int i;
	for (i = 0; i < (pb->width*pb->height); ++i)
		pb->px[i] = color;
}

extern void
pixbuf_render(const Pixbuf_t *pb)
{
	//                  cont.width
	//  ◄───────────────────────────────────────►
	//  a───────────┬───────────────b────────────┐ ▲
	//  │           │               │            │ │
	//  │           │               │            │ │
	//  │           │               │            │ │
	//  │           │               │            │ │ c
	//  │           │               │            │ │ o
	//  ├───────────┼───────────────┼────────────┤ │ n
	//  │           │pixbuf         │            │ │ t
	//  │           │               │            │ │ .
	//  │           │               │            │ │ h
	//  │           │               │            │ │ e
	//  │           │               │            │ │ i
	//  c───────────┼───────────────┼────────────┤ │ g
	//  │           │               │            │ │ h
	//  │           │               │            │ │ t
	//  │           │               │            │ │
	//  │           │               │            │ │
	//  │           │               │            │ │
	//  └───────────┴───────────────┴────────────┘ ▼

	int a_x, a_y, b_x, b_y, c_x, c_y;

	a_x = 0;
	a_y = 0;

	b_x = pb->x + pb->width;
	b_y = 0;

	c_x = 0;
	c_y = pb->y + pb->height;

	if (pb->y > 0) {
		// Clear top area
		xcb_clear_area(pb->conn, 0, pb->cont.drawable, a_x, a_y,
				pb->cont.width, pb->y);
	}

	if (pb->x > 0) {
		// Clear left area
		xcb_clear_area(pb->conn, 0, pb->cont.drawable, a_x, a_y,
				pb->x, pb->cont.height);
	}

	if (pb->y + pb->height < pb->cont.height) {
		// Clear bottom area
		xcb_clear_area(pb->conn, 0, pb->cont.drawable, c_x, c_y,
				pb->cont.width, pb->cont.height - c_y);
	}

	if (pb->x + pb->width < pb->cont.width) {
		// Clear right area
		xcb_clear_area(pb->conn, 0, pb->cont.drawable, b_x, b_y,
				pb->cont.width - b_x, pb->cont.height);
	}

	if (pb->is_shm) {
		xcb_copy_area(pb->conn, pb->shm.pixmap, pb->cont.drawable,
				pb->gc, 0, 0, pb->x, pb->y, pb->width, pb->height);
	} else {
		xcb_image_put(pb->conn, pb->cont.drawable, pb->gc,
				pb->image, pb->x, pb->y, 0);
	}

	xcb_flush(pb->conn);
}

extern void
pixbuf_free(Pixbuf_t *pb)
{
	xcb_free_gc(pb->conn, pb->gc);

	if (pb->is_shm) {
		shmctl(pb->shm.id, IPC_RMID, NULL);
		xcb_shm_detach(pb->conn, pb->shm.seg);
		shmdt(pb->px);
		xcb_free_pixmap(pb->conn, pb->shm.pixmap);
	} else {
		xcb_image_destroy(pb->image);
	}

	pixman_image_unref(pb->pixman_image);

	free(pb);
}
