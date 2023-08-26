/*
	Copyright (C) 2022 <alpheratz99@protonmail.com>

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

#ifndef __XPAVM_BASE_LINKEDLIST_H__
#define __XPAVM_BASE_LINKEDLIST_H__

#include <stdint.h>

struct linkedlist {
	struct linkedlist *next;
	void *data;
};

extern void
linkedlist_append(struct linkedlist **ref, void *data);

extern void *
linkedlist_get(struct linkedlist *list, uint32_t pos);

extern uint32_t
linkedlist_length(struct linkedlist *list);

extern void
linkedlist_free(struct linkedlist *list);

#endif
