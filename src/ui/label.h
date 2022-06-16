#ifndef __XPAVM_UI_LABEL_H__
#define __XPAVM_UI_LABEL_H__

#include <stdint.h>

#include "../base/bitmap.h"
#include "../base/font.h"

extern void
label_render_onto(const char *text,
                  font_t *font,
                  uint32_t color,
                  uint32_t x,
                  uint32_t y,
                  bitmap_t *bmp);

#endif
