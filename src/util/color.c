#include "numdef.h"
#include "color.h"

static u8
blerp(u8 from, u8 to, u8 v) {
	return from + ((to - from) * v) / 0xff;
}

extern u32
color_lerp(u32 from, u32 to, u8 v) {
	u8 r = blerp((from >> 16) & 0xff, (to >> 16) & 0xff, v);
	u8 g = blerp((from >> 8) & 0xff, (to >> 8) & 0xff, v);
	u8 b = blerp(from & 0xff, to & 0xff, v);

	return (r << 16) | (g << 8) | b;
}
