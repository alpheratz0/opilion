#include <stdint.h>

#include "color.h"

static inline uint8_t
blerp(uint8_t from, uint8_t to, uint8_t v)
{
	return from + ((to - from) * v) / 0xff;
}

extern uint32_t
color_lerp(uint32_t from, uint32_t to, uint8_t v)
{
	uint8_t r, g, b;

	r = blerp((from >> 16) & 0xff, (to >> 16) & 0xff, v);
	g = blerp((from >> 8) & 0xff, (to >> 8) & 0xff, v);
	b = blerp(from & 0xff, to & 0xff, v);

	return (r << 16) | (g << 8) | b;
}
