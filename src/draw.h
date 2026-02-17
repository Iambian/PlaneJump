#ifndef __pjump_draw__
#define __pjump_draw__

#include <stdint.h>

void drawGameField(uint8_t tile_offset,uint8_t pixel_offset,int x_offset);
void drawBG(void);
uint16_t hue_to_1555RGB(uint8_t hue);

#endif