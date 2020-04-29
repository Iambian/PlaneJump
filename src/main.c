/*
 *--------------------------------------
 * Program Name: Plain Jump
 * Author: idk anymore
 * License: rawrf.
 * Description: rawrf.
 *--------------------------------------
*/

#define VERSION "0.1"

#define GS_TITLE 0
#define GS_GAMEPLAY 1
#define GS_GAMEOVER 2
#define GS_CREDITS 3
#define GS_QUIT 255


//These color things. Most of them are guesses.
//My eyes are not clever things.
#define COLOR_SKYBLUE 0xBF
#define COLOR_BLACK 0
#define COLOR_WHITE 255
#define COLOR_TRANSPARENT COLOR_WHITE
#define COLOR_RED 0x80
#define COLOR_GREEN 0x04
#define COLOR_PURPLE 0x78
#define COLOR_GOLD 0xE4




/* Keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

/* Standard headers (recommended) */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External library headers */
#include <debug.h>
#include <keypadc.h>
#include <graphx.h>
#include <fileioc.h>
#include <compression.h>

#include "draw.h"
#include "gfx/out/sprites_gfx.h"

typedef struct thing {
	uint8_t ypos;    //0-240. is 255 if scanline is out of range
	int     startx;  //reasonably could be a uint8_t, but doing for easy load
	int     width;   //8.8 fixed point
} scanline_t;

typedef struct thing2 { // :)
	float x;
	float y;
	float z;
} vector_t;

vector_t empty_point = {0.0f,0.0f,0.0f};
//Function prototypes
void keywait(void);
void keyconfirm(void);
void printTextCenter(char *s,int y);
void drawMenu(char *sa[],uint8_t curopt,uint8_t maxopt);
void drawTitleFrame(void);
void genSection(int8_t gentype);  //generates latter 16 blocks of level
vector_t proj(vector_t point);

void init_xlate(int angle);
//void drawGameField(uint8_t tile_offset,uint8_t pixel_offset);


//Globals. For performance reasons, locals may not be more than 128 bytes
//total per function (or scope?), so things too large should also be here.
uint8_t track[32];
scanline_t translate[240+32+32];  //32 above and below
gfx_UninitedSprite(ball0,32,32);
gfx_UninitedSprite(ball1,32,32);
gfx_UninitedSprite(ball2,32,32);
gfx_UninitedSprite(ball3,32,32);
gfx_UninitedSprite(ball4,32,32);
gfx_UninitedSprite(ball5,32,32);
gfx_UninitedSprite(ball6,32,32);
gfx_UninitedSprite(ball7,32,32);
gfx_sprite_t *ballanim[8];

//add sprites later.

void main(void)
{
	kb_key_t kdir,kact;
	uint8_t state,i;
	uint8_t stage_state;
	int8_t tile_px_passed;  //0-32, is vertical offset of current tile.
	int8_t tile_passed;     //increments. once 16, reset and gen new section
	int x,tx;
	uint8_t y,ty;
	int score,hiscore;
	uint8_t ball_counter;    //0-3, index to ballanim
	int ball_x;
	int ball_min_x;
	int ball_max_x;
	uint8_t ball_y;
	
	
	
	gfx_Begin();
	gfx_SetTransparentColor(COLOR_WHITE);
	zx7_Decompress(ball0,ball0_compressed); 
	zx7_Decompress(ball1,ball1_compressed);
	zx7_Decompress(ball2,ball2_compressed);
	zx7_Decompress(ball3,ball3_compressed);
	zx7_Decompress(ball4,ball4_compressed); 
	zx7_Decompress(ball5,ball5_compressed);
	zx7_Decompress(ball6,ball6_compressed);
	zx7_Decompress(ball7,ball7_compressed);
	init_xlate(0);
	ballanim[0] = ball7;
	ballanim[1] = ball6;
	ballanim[2] = ball5;
	ballanim[3] = ball4;  //this is so messed up. should've been able to do this in a single statement.
	ballanim[4] = ball3;
	ballanim[5] = ball2;
	ballanim[6] = ball1;
	ballanim[7] = ball0;  //this is so messed up. should've been able to do this in a single statement.
	
	
	state = GS_TITLE;
	stage_state = 0;
	hiscore = score = 0;  //change how hiscore is init'd later
	tile_px_passed = tile_passed = 0;
	
	ball_counter = 0;
	ball_max_x = ball_min_x = translate[239+32].startx;
	for (i=0;i<4;++i) ball_max_x += ((uint8_t*)(&translate[239+32].w1))[i];
	ball_x = ball_min_x;
	ball_y = (240-32);
		
	while (1) {
		kdir = kb_Data[7];
		kact = kb_Data[1];
		
		//Quick exit without needing to resort to GOTOs.
		if (state==GS_TITLE && kact&kb_Mode) break;
		gfx_FillScreen(COLOR_WHITE);
		
		switch (state) {
			case GS_TITLE   :
				//Debugging: Immediately init and go to game mode.
				memset(track,0,sizeof track);
				genSection(-1);
				genSection(stage_state);
				genSection(stage_state);
				state = GS_GAMEPLAY;
				tile_passed = tile_px_passed = 0;
				break;
			
			case GS_GAMEPLAY:
				if (kact&kb_Mode) { keywait(); gfx_End(); return; }
				//Let's just show a scrolling field for now.
				//for (i=0;i<32;++i) {
					//slow down rendering significantly
					//gfx_FillScreen(COLOR_WHITE);
					drawGameField(tile_passed,tile_px_passed);
					//gfx_SwapDraw();
				//}
				//Move down
				tile_px_passed += 4;
				if (tile_px_passed>32) {
					tile_px_passed -= 32;
					tile_passed -= 1;
					if (tile_passed<=0) {
						tile_passed = 16;
						genSection(stage_state);
					}
				}
				
				//we need more frames.
				ball_counter += 1;
				gfx_TransparentSprite_NoClip(ballanim[(ball_counter>>1)&7],ball_x,ball_y);
				
				
				
				
				break;
			
			case GS_GAMEOVER:
				break;
			
			case GS_CREDITS :
				break;
			
			default:
				break;
		}
		gfx_SwapDraw();
		
		//Debouncing, but not in-game.
		if ((kact|kdir) && (state != GS_GAMEPLAY)) {
			kact = kdir = 0;
			keywait();
		}
	}
	
	gfx_End();
    return;
}


void keywait(void) {
	while (kb_AnyKey());  //Pauses until keys are released
}
void keyconfirm(void) {
	keywait();
	while (!kb_AnyKey());
	keywait();
}


void drawTitleFrame(void) {
	gfx_SetTextScale(3,3);
	gfx_SetTextFGColor(COLOR_BLACK);
	printTextCenter("Plain Jump",10);
	gfx_SetTextScale(1,1);
	gfx_PrintStringXY(VERSION,320-32,240-10);
}


void printTextCenter(char *s,int y) {
	int w;
	w = gfx_GetStringWidth(s);
	gfx_PrintStringXY(s,(320-w)/2,y);
}

/* Vertically-centered menu */
void drawMenu(char *sa[],uint8_t curopt,uint8_t maxopt) {
	uint8_t i,y;
	
	y = (240 - (maxopt * 20)) / 2;
	gfx_SetTextScale(2,2);
	
	for (i=0; i<maxopt; ++i,y+=20) {
		if (i==curopt) {
			gfx_SetTextFGColor(COLOR_GOLD);
		}
		else {
			gfx_SetTextFGColor(COLOR_BLACK);
		}
		printTextCenter(sa[i],y+4);
	}
}

void genSection(int8_t gentype) {
	uint8_t i,d;
	uint24_t dt;
	static uint8_t state=0;
	
	memcpy(&track[16],&track[0],16);
	//reinits state to beginning for level progression purposes
	if (gentype == -1) {
		state = 0;
		return;
	}
	//randgen, 2-wide blocks
	if (!gentype) {
		for (i=0;i<16;i+=2) {
			dt = random();
			d = (dt&255)^((dt>>8)&255)^((dt>>16)&255);
			track[i] = d;
			track[i+1] = d;
		}
	}
	
	
	return;
}



#define CAM_DIST 3.0f
#define PLANE_DIST 2.0f
//stripped down, as we aren't going to rotate the camera at all.
//also performs translation to physical screen.
vector_t proj(vector_t point) {
	float z;
	vector_t result;
	
	z = -(point.z-CAM_DIST);
	
	//Translation to 2D, then to screen buffer coord system
	//Adjustment factors tweaked via trial-and-error
	result.x = (((PLANE_DIST / z) * point.x) + 5.0f) * 32;
	result.y = (((PLANE_DIST / z) * point.y) + 2.5f) * 38;
	
	return result;
}

#define IN_ANGLE ((0.0f+20.0f) * (3.14159265359f / 180.0f))

//old size: 3287. Should not be greater than ~6200 bytes at end
//do nothing for angle at the moment. just do direct translation
//I don't have a plan.
void init_xlate(int angle) {
	///*
	float x,y,a,w;
	uint8_t xi;
	int yi;
	int temp;
	vector_t basepoint,point1,point2;
	vector_t point,point0,pointw,pointh,rpoint;
	a = IN_ANGLE; //(float)angle;
	
	
	basepoint = empty_point; //okay. that made the compiler shut up.
	for (y = -(3.5f+1.0f), yi = -32; yi < (240+32) ; y += (1.0f/32.0f), ++yi) {
		//do outside-buffer-clipping later.
		basepoint.x = (-2.0f);
		basepoint.y = (y+0.0f)*cos(a);
		basepoint.z = (y+0.0f)*sin(a);
		point1 = proj(basepoint);
		basepoint.x = (-1.0f);
		point2 = proj(basepoint);
		if ( ((unsigned int)yi)<240 && ((unsigned int)point1.y)<240)
			translate[yi+32].ypos   = (uint8_t) point1.y;
		else
			translate[yi+32].ypos   = 255;
		translate[yi+32].startx = (int) point1.x;
		//changing width to 8.8 fixed point
		translate[yi+32].w1)[xi] = (point2.x-point1.x)*256;
		dbg_sprintf(dbgout,"yidx %i, ypos %i, xstart %i, w1 %i, w2 %i, w3 %i, w4 %i\n",yi,translate[yi+32].ypos,translate[yi+32].startx,translate[yi+32].width);
	}
	//*/
	/*
	int y;
	//int x;
	//int tx,ty;
	//uint8_t i;
	
	for (y = -32; y < (240+32); ++y) {
		translate[y+32].ypos = (y>=0 && y<240) ? y : 255;
		translate[y+32].startx = 96;
		translate[y+32].w1 = 32;
		translate[y+32].w2 = 32;
		translate[y+32].w3 = 32;
		translate[y+32].w4 = 32;
	}
	*/
}













