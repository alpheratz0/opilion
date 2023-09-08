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

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_image.h>
#include <xcb/shm.h>
#include <sys/shm.h>

#include "pixbuf.h"
#include "utils.h"
#include "log.h"

struct Pixbuf {
	struct {
		float x;
		float y;
	} pos;

	int viewport_width;
	int viewport_height;

	int width;
	int height;
	uint32_t *px;

	/* X11 */
	xcb_connection_t *conn;
	xcb_screen_t *scr;
	xcb_window_t win;
	int shm;
	xcb_gcontext_t gc;
	union {
		struct {
			int id;
			xcb_shm_seg_t seg;
			xcb_pixmap_t pixmap;
		} shm;
		xcb_image_t *image;
	} x;
};

static int
__x_check_mit_shm_extension(xcb_connection_t *conn)
{
	xcb_generic_error_t *error;
	xcb_shm_query_version_cookie_t cookie;
	xcb_shm_query_version_reply_t *reply;
	int supported;

	cookie = xcb_shm_query_version(conn);
	reply = xcb_shm_query_version_reply(conn, cookie, &error);
	supported = !error && reply && reply->shared_pixmaps;

	free(error); free(reply);

	return supported;
}

static void
__pixbuf_set_size(Pixbuf_t *pb, int w, int h)
{
	if (pb->shm) {
		if (pb->px) {
			shmctl(pb->x.shm.id, IPC_RMID, NULL);
			xcb_shm_detach(pb->conn, pb->x.shm.seg);
			shmdt(pb->px);
			xcb_free_pixmap(pb->conn, pb->x.shm.pixmap);
		}

		pb->x.shm.seg = xcb_generate_id(pb->conn);
		pb->x.shm.pixmap = xcb_generate_id(pb->conn);
		pb->x.shm.id = shmget(IPC_PRIVATE, w*h*4, IPC_CREAT | 0600);

		if (pb->x.shm.id < 0)
			die("shmget:");

		pb->px = shmat(pb->x.shm.id, NULL, 0);

		if (pb->px == (void *) -1) {
			shmctl(pb->x.shm.id, IPC_RMID, NULL);
			die("shmat:");
		}

		xcb_shm_attach(pb->conn, pb->x.shm.seg, pb->x.shm.id, 0);
		shmctl(pb->x.shm.id, IPC_RMID, NULL);

		xcb_shm_create_pixmap(pb->conn, pb->x.shm.pixmap, pb->win, w, h,
				pb->scr->root_depth, pb->x.shm.seg, 0);
	} else {
		if (pb->px)
			xcb_image_destroy(pb->x.image);

		if (w > 2000) w = 2000;
		if (h > 2000) h = 2000;

		pb->px = xmalloc(w*h*4);

		pb->x.image = xcb_image_create_native(pb->conn, w, h,
				XCB_IMAGE_FORMAT_Z_PIXMAP, pb->scr->root_depth, pb->px,
				w*h*4, (uint8_t*)pb->px);
	}

	pb->width = w;
	pb->height = h;
}

extern Pixbuf_t *
pixbuf_new(xcb_connection_t *conn, xcb_window_t win, int w, int h)
{
	xcb_screen_t *scr;
	Pixbuf_t *pb;

	scr = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;

	if (NULL == scr)
		die("can't get default screen");

	pb = xcalloc(1, sizeof(Pixbuf_t));

	pb->conn = conn;
	pb->win = win;
	pb->scr = scr;
	pb->viewport_width = w;
	pb->viewport_height = h;

	pb->gc = xcb_generate_id(conn);
	pb->shm = __x_check_mit_shm_extension(conn) ? 1 : 0;

	xcb_create_gc(conn, pb->gc, win, 0, NULL);

	__pixbuf_set_size(pb, w, h);

	return pb;
}

extern int
pixbuf_get_width(Pixbuf_t *pb)
{
	return pb->width;
}

extern int
pixbuf_get_height(Pixbuf_t *pb)
{
	return pb->height;
}

extern int
pixbuf_get(Pixbuf_t *pb, int x, int y, uint32_t *color)
{
	if (x >= 0 && y >= 0 && x < pb->width && y < pb->height) {
		*color = pb->px[y*pb->width+x];
		return 1;
	}
	return 0;
}

extern void
pixbuf_set(Pixbuf_t *pb, int x, int y, uint32_t color)
{
	if (x >= 0 && y >= 0 && x < pb->width && y < pb->height)
		pb->px[y*pb->width+x] = color;
}

extern void
pixbuf_set_viewport(Pixbuf_t *pb, int vw, int vh)
{
	pb->pos.x += ((float)vw - pb->viewport_width) / 2;
	pb->pos.y += ((float)vh - pb->viewport_height) / 2;

	pb->viewport_width = vw;
	pb->viewport_height = vh;
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
pixbuf_render(Pixbuf_t *pb)
{
	if (pb->pos.y > 0)
		xcb_clear_area(pb->conn, 0, pb->win, 0, 0, pb->viewport_width, pb->pos.y);

	if (pb->pos.y + pb->height < pb->viewport_height)
		xcb_clear_area(pb->conn, 0, pb->win, 0, pb->pos.y + pb->height,
				pb->viewport_width, pb->viewport_height - (pb->pos.y + pb->height));

	if (pb->pos.x > 0)
		xcb_clear_area(pb->conn, 0, pb->win, 0, 0, pb->pos.x, pb->viewport_height);

	if (pb->pos.x + pb->width < pb->viewport_width)
		xcb_clear_area(pb->conn, 0, pb->win, pb->pos.x + pb->width, 0,
				pb->viewport_width - (pb->pos.x + pb->width), pb->viewport_height);

	if (pb->shm) {
		xcb_copy_area(pb->conn, pb->x.shm.pixmap, pb->win,
				pb->gc, 0, 0, pb->pos.x, pb->pos.y, pb->width, pb->height);
	} else {
		xcb_image_put(pb->conn, pb->win, pb->gc,
				pb->x.image, pb->pos.x, pb->pos.y, 0);
	}

	xcb_flush(pb->conn);
}

extern void
pixbuf_free(Pixbuf_t *pb)
{
	xcb_free_gc(pb->conn, pb->gc);

	if (pb->shm) {
		shmctl(pb->x.shm.id, IPC_RMID, NULL);
		xcb_shm_detach(pb->conn, pb->x.shm.seg);
		shmdt(pb->px);
		xcb_free_pixmap(pb->conn, pb->x.shm.pixmap);
	} else {
		xcb_image_destroy(pb->x.image);
	}

	free(pb);
}
