.POSIX:
.PHONY: all clean install uninstall dist

VERSION = 0.1.4

CC      = cc
INCS    = -I/usr/include/freetype2
CFLAGS  = -std=c99 -pedantic -Wall -Wextra -Os $(INCS) -DVERSION=\"$(VERSION)\"
LDLIBS  = -lxcb -lxcb-image -lxcb-keysyms -lfontconfig -lfreetype -lpulse
LDFLAGS = -s


PREFIX    = /usr/local
MANPREFIX = $(PREFIX)/share/man
APPPREFIX = $(PREFIX)/share/applications

SRC = src/xpavm.c src/base/linkedlist.c src/base/font.c src/base/bitmap.c src/pulseaudio/connection.c src/pulseaudio/sink.c src/pulseaudio/volume.c \
	  src/util/debug.c src/util/color.c src/util/xmalloc.c src/ui/label.c src/ui/sink_selector.c src/x11/window.c

OBJ = src/xpavm.o src/base/linkedlist.o src/base/font.o src/base/bitmap.o src/pulseaudio/connection.o src/pulseaudio/sink.o src/pulseaudio/volume.o \
	  src/util/debug.o src/util/color.o src/util/xmalloc.o src/ui/label.o src/ui/sink_selector.o src/x11/window.o

all: xpavm

xpavm: $(OBJ)
	$(CC) $(LDFLAGS) -o xpavm $(OBJ) $(LDLIBS)

clean:
	rm -f xpavm xpavm.o xpavm-$(VERSION).tar.gz $(OBJ)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f xpavm $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/xpavm
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f man/xpavm.1 $(DESTDIR)$(MANPREFIX)/man1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/xpavm.1
	mkdir -p $(DESTDIR)$(APPPREFIX)
	cp -f misc/xpavm.desktop $(DESTDIR)$(APPPREFIX)/xpavm.desktop
	chmod 644 $(DESTDIR)$(APPPREFIX)/xpavm.desktop

dist: clean
	mkdir -p xpavm-$(VERSION)
	cp -R LICENSE Makefile README misc src man xpavm-$(VERSION)
	tar -cf xpavm-$(VERSION).tar xpavm-$(VERSION)
	gzip xpavm-$(VERSION).tar
	rm -rf xpavm-$(VERSION)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/xpavm
	rm -f $(DESTDIR)$(MANPREFIX)/man1/xpavm.1
	rm -f $(DESTDIR)$(APPPREFIX)/xpavm.desktop
