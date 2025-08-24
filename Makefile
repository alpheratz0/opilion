.POSIX:
.PHONY: all clean install uninstall dist

include config.mk

OBJ=\
	src/opilion.o \
	src/pixbuf.o \
	src/pa.o \
	src/log.o \
	src/icon.o \
	src/pixman-image-cache.o \
	src/render-util.o \
	src/sink-selector.o \
	src/text-renderer.o \
	src/utils.o \
	vendor/libgrapheme/libgrapheme.a

all: opilion

vendor/libgrapheme/libgrapheme.a:
	test -d vendor/libgrapheme || git clone --depth 1 https://git.suckless.org/libgrapheme vendor/libgrapheme
	./vendor/libgrapheme/configure
	make -C ./vendor/libgrapheme

src/opilion.o: include/config.h

opilion: $(OBJ)
	$(CC) $(OBJ) -o opilion $(LDFLAGS)

clean:
	rm -f opilion $(OBJ) opilion-$(VERSION).tar.gz

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f opilion $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/opilion
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f opilion.1 $(DESTDIR)$(MANPREFIX)/man1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/opilion.1
	mkdir -p $(DESTDIR)$(APPPREFIX)
	cp -f extra/opilion.desktop $(DESTDIR)$(APPPREFIX)/opilion.desktop
	chmod 644 $(DESTDIR)$(APPPREFIX)/opilion.desktop

dist: clean
	mkdir -p opilion-$(VERSION)
	cp -R COPYING config.mk Makefile README opilion.1 src include \
		extra opilion-$(VERSION)
	tar -cf opilion-$(VERSION).tar opilion-$(VERSION)
	gzip opilion-$(VERSION).tar
	rm -rf opilion-$(VERSION)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/opilion
	rm -f $(DESTDIR)$(MANPREFIX)/man1/opilion.1
	rm -f $(DESTDIR)$(APPPREFIX)/opilion.desktop
