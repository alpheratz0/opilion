# Copyright (C) 2022-2023 <alpheratz99@protonmail.com>
# This program is free software.

VERSION = 0.9.1

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man
APPPREFIX = $(PREFIX)/share/applications

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

PKG_CONFIG = pkg-config

PINCS = -Iinclude \
		-I$(X11INC) \
		`$(PKG_CONFIG) --cflags fcft` \
		`$(PKG_CONFIG) --cflags xcb` \
		`$(PKG_CONFIG) --cflags xcb-shm` \
		`$(PKG_CONFIG) --cflags xcb-image` \
		`$(PKG_CONFIG) --cflags xcb-keysyms` \
		`$(PKG_CONFIG) --cflags xcb-xkb` \
		`$(PKG_CONFIG) --cflags libpulse` \
		`$(PKG_CONFIG) --cflags pixman-1`

PLIBS = -L$(X11LIB) -lxcb -lxcb-shm -lxcb-image \
		-lxcb-keysyms -lxcb-xkb -lpulse \
		`$(PKG_CONFIG) --libs fcft` \
		`$(PKG_CONFIG) --libs pixman-1`

CFLAGS = -Wall -Wextra -pedantic -Os $(PINCS) $(CPPFLAGS) \
		 -DVERSION=\"$(VERSION)\"

LDFLAGS = -s $(PLIBS)

CC = cc
