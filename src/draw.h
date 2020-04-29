#ifndef __pjump_draw__
#define __pjump_draw__

#include <stdint.h>

void drawTrapezoid(int tx1, int tx2, int ty1, int ty2, int tw1, int tw2, int color);
void drawGameField(uint8_t tile_offset,uint8_t pixel_offset);

#endif