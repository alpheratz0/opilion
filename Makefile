VERSION = 0.1.4
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
APPPREFIX = ${PREFIX}/share/applications
LDLIBS = -lxcb -lxcb-image -lpulse -lfontconfig -lfreetype
LDFLAGS = -s ${LDLIBS}
INCS = -I/usr/include -I/usr/include/freetype2
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Os ${INCS} -D_XOPEN_SOURCE=500 -DVERSION="\"${VERSION}\""
CC = cc

SRC = src/xpavm.c \
	  src/base/linkedlist.c \
	  src/base/font.c \
	  src/base/bitmap.c \
	  src/pulseaudio/connection.c \
	  src/pulseaudio/sink.c \
	  src/pulseaudio/volume.c \
	  src/util/debug.c \
	  src/util/color.c \
	  src/util/xmalloc.c \
	  src/ui/label.c \
	  src/ui/sink_selector.c \
	  src/x11/window.c


OBJ = ${SRC:.c=.o}

all: xpavm

${OBJ}:	src/base/linkedlist.h \
		src/base/font.h \
		src/base/bitmap.h \
		src/pulseaudio/connection.h \
		src/pulseaudio/sink.h \
		src/pulseaudio/volume.h \
		src/util/debug.h \
		src/util/color.h \
		src/util/xmalloc.h \
		src/ui/label.h \
		src/ui/sink_selector.h \
		src/x11/keys.h \
		src/x11/window.h

xpavm: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

install: all
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f xpavm ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/xpavm
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@cp -f man/xpavm.1 ${DESTDIR}${MANPREFIX}/man1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/xpavm.1
	@mkdir -p ${DESTDIR}${APPPREFIX}
	@cp -f misc/xpavm.desktop ${DESTDIR}${APPPREFIX}/xpavm.desktop
	@chmod 644 ${DESTDIR}${APPPREFIX}/xpavm.desktop

dist: clean
	@mkdir -p xpavm-${VERSION}
	@cp -R LICENSE Makefile README misc man src xpavm-${VERSION}
	@tar -cf xpavm-${VERSION}.tar xpavm-${VERSION}
	@gzip xpavm-${VERSION}.tar
	@rm -rf xpavm-${VERSION}

uninstall:
	@rm -f ${DESTDIR}${PREFIX}/bin/xpavm
	@rm -f ${DESTDIR}${MANPREFIX}/man1/xpavm.1
	@rm -f ${DESTDIR}${APPPREFIX}/xpavm.desktop

clean:
	@rm -f xpavm xpavm-${VERSION}.tar.gz ${OBJ}

.PHONY: all clean install uninstall dist
