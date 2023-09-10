.POSIX:
.PHONY: all clean install uninstall dist

include config.mk

OBJ=\
	src/xpavm.o \
	src/pixbuf.o \
	src/pa.o \
	src/log.o \
	src/render-util.o \
	src/sink-selector.o \
	src/text-renderer.o \
	src/utils.o

all: xpavm

xpavm: $(OBJ)
	$(CC) $(LDFLAGS) -o xpavm $(OBJ)

clean:
	rm -f xpavm $(OBJ) xpavm-$(VERSION).tar.gz

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f xpavm $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/xpavm
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f xpavm.1 $(DESTDIR)$(MANPREFIX)/man1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/xpavm.1
	mkdir -p $(DESTDIR)$(APPPREFIX)
	cp -f extra/xpavm.desktop $(DESTDIR)$(APPPREFIX)/xpavm.desktop
	chmod 644 $(DESTDIR)$(APPPREFIX)/xpavm.desktop

dist: clean
	mkdir -p xpavm-$(VERSION)
	cp -R COPYING config.mk Makefile README xpavm.1 src include \
		extra xpavm-$(VERSION)
	tar -cf xpavm-$(VERSION).tar xpavm-$(VERSION)
	gzip xpavm-$(VERSION).tar
	rm -rf xpavm-$(VERSION)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/xpavm
	rm -f $(DESTDIR)$(MANPREFIX)/man1/xpavm.1
	rm -f $(DESTDIR)$(APPPREFIX)/xpavm.desktop
