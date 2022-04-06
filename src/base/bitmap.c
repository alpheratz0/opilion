#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../util/numdef.h"
#include "../util/debug.h"
#include "bitmap.h"

extern bitmap_t *
bitmap_create(u32 width, u32 height, u32 color) {
	bitmap_t *bmp;

	if ((bmp = malloc(sizeof(bitmap_t)))) {
		bmp->width = width;
		bmp->height = height;
		if ((bmp->px = malloc(4*width*height))) {
			for (u32 i = 0; i < width * height; ++i) {
				bmp->px[i] = color;
			}
			return bmp;
		}
	}

	die("error while calling malloc, no memory available");

	return (void *)(0);
}

extern void
bitmap_set(bitmap_t *bmp, u32 x, u32 y, u32 color) {
	if (x < bmp->width && y < bmp->height) {
		bmp->px[y*bmp->width+x] = color;
	}
}

extern void
bitmap_set_unsafe(bitmap_t *bmp, u32 x, u32 y, u32 color) {
	bmp->px[y*bmp->width+x] = color;
}

extern u32
bitmap_get(bitmap_t *bmp, u32 x, u32 y) {
	if (x < bmp->width && y < bmp->height) {
		return bmp->px[y*bmp->width+x];
	}
	return 0;
}

extern u32
bitmap_get_unsafe(bitmap_t *bmp, u32 x, u32 y) {
	return bmp->px[y*bmp->width+x];
}

extern void
bitmap_rect(bitmap_t *bmp, u32 x, u32 y, u32 width, u32 height, u32 color) {
	for (u32 i = 0; i < width; ++i) {
		for (u32 j = 0; j < height; ++j) {
			bitmap_set(bmp, x + i, y + j, color);
		}
	}
}

extern void
bitmap_clear(bitmap_t *bmp, u32 color) {
	for (u32 x = 0; x < bmp->width; ++x) {
		for (u32 y = 0; y < bmp->height; ++y) {
			bitmap_set_unsafe(bmp, x, y, color);
		}
	}
}

extern void
bitmap_copy(bitmap_t *from, bitmap_t *to, u32 x, u32 y) {
	for (u32 i = 0; i < from->width; ++i) {
		for (u32 j = 0; j < from->height; ++j) {
			bitmap_set(to, x + i, y + j, bitmap_get_unsafe(from, i, j));
		}
	}
}

extern void
bitmap_save(bitmap_t *bmp, const char *path) {
	FILE *file;

	if ((file = fopen(path, "wb"))) {
		fprintf(file, "P6\n%d %d 255\n", bmp->width, bmp->height);

		for (u32 y = 0; y < bmp->height; ++y) {
			for (u32 x = 0; x < bmp->width; ++x) {
				u8 pixel[3] = {
					(bmp->px[y*bmp->width+x] & 0xff0000) >> 16,
					(bmp->px[y*bmp->width+x] & 0xff00) >> 8,
					(bmp->px[y*bmp->width+x] & 0xff)
				};

				fwrite(pixel, sizeof(pixel), 1, file);
			}
		}

		fclose(file);
		return;
	}

	dief("fopen failed: %s", strerror(errno));
}

extern void
bitmap_free(bitmap_t *bmp) {
	free(bmp->px);
	free(bmp);
}
