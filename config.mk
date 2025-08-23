# Copyright (C) 2022-2025 <alpheratz99@protonmail.com>
# This program is free software.

VERSION = 0.14.2

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man
APPPREFIX = $(PREFIX)/share/applications

PKG_CONFIG = pkg-config

DEPENDENCIES = fcft xcb xcb-shm xcb-image xcb-keysyms xcb-xkb xcb-cursor libpulse pixman-1

INCS = $(shell $(PKG_CONFIG) --cflags $(DEPENDENCIES)) -Iinclude
LIBS = $(shell $(PKG_CONFIG) --libs $(DEPENDENCIES)) -lm -lgrapheme

CFLAGS = -Os $(INCS) -DVERSION=\"$(VERSION)\"
LDFLAGS = -s $(LIBS)

CC = cc
