/*
 *--------------------------------------
 * Program Name: Plane Jump
 * Author: idk anymore
 * License: rawrf.
 * Description: rawrf.
 *--------------------------------------
*/

/* TODO: MAYBE ADD ACTUAL LEVELS INSTEAD OF SOME PIECE OF CRAP INITIAL 
   THING WHICH IS BUGGED TO HELL AND THEN RELY ON THE RANDOM NUMBER GOD
   TO GENERATE FAVOR WITH THE MASSES.
   
   AND MAYBE SIZE-OPTIMIZE THE GAME ITSELF. Because making the graphics
   crappier to get them to compress better shouldn't be the go-to solution.
*/

#define VERSION "v0.1"

#define GS_TITLE 0
#define GS_GAMEPLAY 1
#define GS_GAMEOVER 2
#define GS_CREDITS 3
#define GS_HELP 4
#define GS_FALLING 5
#define GS_QUITTING 6
#define GS_QUIT 255

#define GEN_INIT 1
#define GEN_CONTINUE 0

//These color things. Most of them are guesses.
//My eyes are not clever things.
#define COLOR_WHITE 255
#define COLOR_GOLD  254
#define COLOR_GRAY  253
#define COLOR_DGRAY 252
#define COLOR_BLACK 0
//Reserved range of colors
#define COLOR_EXPLODESTART 245
#define COLOR_EXPLODEEND   250
//End range
#define COLOR_TRANSPARENT COLOR_WHITE

#define Q_CALCX(x1,x2,xv) ((xv*xv)-xv*(x1+x2)+x1*x2)
#define Q_SOLVEA(x1,x2,xv,yv) (yv/Q_CALCX(x1,x2,xv))
#define QUADR(x1,x2,xv,yv,curx) (Q_SOLVEA(x1,x2,xv,yv)*curx*curx-curx*(x1+x2)+x1*x2)
#define GRAVITY ((int)(0.38 * 256))
#define INITIAL_VELOCITY ((int)(9*256))

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
#include "levels.h"
#include "gfx/out/sprites_gfx.h"
#include "gfx/out/title_gfx.h"
#include "gfx/out/explode_gfx.h"

typedef struct thing {
	uint8_t ypos;
	int startx;
	uint8_t w[4];
} scanline_t;

typedef struct thing2 { // :)
	float x;
	float y;
	float z;
} vector_t;

typedef struct anotherthing {
	unsigned int hiscore;
	unsigned int number_of_times_ran;
	unsigned int number_of_times_quitted;
	unsigned int number_of_times_died;
	uint8_t something;
	uint8_t reserved[20];
} gamedata_t;


vector_t empty_point = {0.0f,0.0f,0.0f};
//Function prototypes
void keywait(void);
void keyconfirm(void);
void printTextCenter(char *s,int y);
//void drawMenu(char *sa[],uint8_t curopt,uint8_t maxopt);
void drawTextBlock(char *sa[],uint8_t sa_len);
void showStats(int data, char *s, uint8_t ypos);
void drawTitleFrame(void);
void genSection(uint8_t init);  //generates latter 16 blocks of level
void changePalette(uint16_t *palette);
void blankScreen();               //prevents artifacting between palette swaps
vector_t proj(vector_t point);
gamedata_t gamedata;

void init_xlate(void);
//void drawGameField(uint8_t tile_offset,uint8_t pixel_offset);


//Globals. For performance reasons, locals may not be more than 128 bytes
//total per function (or scope?), so things too large should also be here.
uint8_t track[32];
scanline_t translate[240+32+32];  //32 above and below
int translate_last_usable;
gfx_UninitedSprite(curball,48,48);
gfx_UninitedSprite(explosion,64,64);
gfx_sprite_t *ballanim[8];
gfx_UninitedSprite(titlebanner,title_width,title_height);
level_t *level_pack;     //level pack currently loaded

uint16_t game_palette[256];
uint16_t title_palette[256];

int8_t shadow_offsets[] = {0,1,1,1,1,0};  //For menu shadows
int8_t object99[] = {0,0,-1};
int8_t *next_x_offset = &object99[1]; //for inputs -1,0, and 1 to be correct.
char *titleopts[] = {"Start game","Help","About","Quit"};
char *helptext[] = {
	"[2nd]   : Makes the ball jump",
	"[MODE]  : Quits the game",
	"[Up]    : Makes the ball move forward",
	"[Left]  : Makes the ball move left",
	"[Right] : Makes the ball move right"
};
char *creditstext[] = {
	"Based on Plain Jump 1.1 by Andreas Ess",
	"Game written by Iambian",
	"Coffee and sanity provided by Tim",
	"Thanks Xeda, Eeems, kg583, and fghsgh",
	"for suggestions and the chats",
	"Thanks Cemetech, CodeBros, and Omnimaga",
	"for putting up with my shenanigans",
	
};

#define CAM_DIST 3.0f
#define PLANE_DIST 2.0f
#define IN_ANGLE  ((0.0f+40.0f) * (3.14159265359f / 180.0f))
#define IN_ANGLE2 ((0.0f+40.0f+90.0f) * (3.14159265359f / 180.0f))
#define SIN_FN(x) ((float)(x-(x*x*x)/(3.0f*2)+(x*x*x*x*x)/(5*4*3*2)-(x*x*x*x*x*x*x)/(7*6*5*4*3*2)))
#define SIN_PR SIN_FN(IN_ANGLE)
#define COS_PR SIN_FN(IN_ANGLE2)

void main(void)
{
	ti_var_t file;
	kb_key_t kdir,kact;
	int8_t tile_passed;     //increments. once 16, reset and gen new section
	uint8_t state,i,j,k;
	uint8_t tile_px_passed;  //0-32, is vertical offset of current tile.
	uint8_t y,ty;
	uint8_t ball_counter;    //0-7, index to ballanim
	uint8_t going;
	uint8_t lives;
	uint8_t curopt,maxopt;
	uint8_t nodeath;
	int score;
	int x,tx;
	int ball_x;
	int ball_min_x;
	int ball_max_x;
	int ball_y;
	int jumping;     //continue to use this as the y offset. is 16.8 fp
	int y_velocity;  //16.8fp
	int x_offset;    //for subtle shake effect on landing
	int idx;         //liek i, except longer.
	int yi,wi,werr;
	//int z_speed;
	float xf,yf,x1,y1,z1,x2;
	
	
	//13225
	//12686
	//12678
	
	
	gfx_Begin();
	gfx_SetTransparentColor(COLOR_WHITE);
	gfx_palette[COLOR_GOLD]  = gfx_RGBTo1555(0xFF,0xD7,0x00);
	gfx_palette[COLOR_GRAY]  = gfx_RGBTo1555(0x90,0x90,0x80);
	gfx_palette[COLOR_DGRAY] = gfx_RGBTo1555(0x40,0x40,0x40);
	memcpy(game_palette,gfx_palette,512);
	memcpy(game_palette+COLOR_EXPLODESTART,explode_gfx_pal,sizeof_explode_gfx_pal);
	memcpy(title_palette,gfx_palette,512);
	memcpy(title_palette,title_gfx_pal,sizeof_title_gfx_pal); 
	level_pack = levelpack; //INTERNAL LEVEL PACK
	game_palette[COLOR_BLACK] = gfx_RGBTo1555(0x00,0x00,0x00);
	/* Not needed anymore as we have dynamic background palette generation
	for (i=1;i<241;++i) {
		//game_palette[i] = gfx_Darken(gfx_RGBTo1555(0x87,0xCE,0xEB),255-i);
		game_palette[i] = gfx_RGBTo1555(((0x80-i<0)?0:0x80-i),((0xCE -i<0)?0:0xCE -i),0xEB-(i>>2));
	}
	*/
	zx7_Decompress(titlebanner,title_compressed);
	for(i=0;i<8;++i)
		zx7_Decompress(ballanim[i] = malloc(((32*32)+2)),ball_ts_tiles_compressed[i]);
	
	
	//Generates the translate array inline with this function
	for (yf = -(3.5f+1.0f), yi = -32; yi < (240+32) ; yf += (1.0f/32.0f), ++yi) {
		//condensed projection
		z1 = PLANE_DIST/-(yf*SIN_PR-CAM_DIST);
		x1 = ((z1*-2.0f    )+5.9f)*28;
		y1 = ((z1*yf*COS_PR)+1.5f)*28;
		x2 = ((z1*-1.0f    )+5.9f)*28;
		
		if ( ((unsigned int)yi)<240 && ((unsigned int)y1)<240) {
			translate[yi+32].ypos   = (uint8_t) y1;
			translate_last_usable   = yi;
		} else
			translate[yi+32].ypos   = 255;
		translate[yi+32].startx = (int) x1;
		for (i=0,wi=(x2-x1)*256,werr=0;i<4;++i,werr+=wi&255) {
			translate[yi+32].w[i] = wi>>8;
			if (werr&256) {
				werr -= 256;
				translate[yi+32].w[i]++;
			}
		}
	}
	
	//Initial state is zero (GS_TITLE)
	//state = GS_TITLE;
	y_velocity = x_offset = jumping = score = 0;
	nodeath = curopt = going = ball_counter = lives = tile_px_passed = tile_passed = state = 0;
	
	ball_max_x = ball_min_x = translate[translate_last_usable+32].startx;
	for (i=0;i<4;++i) ball_max_x += translate[translate_last_usable+32].w[i];
	ball_max_x -= (32);
	ball_x = ball_min_x;
	ball_y = (240-32-8);
	maxopt = 4;
	changePalette(title_palette);
	
	//Reading previously saved data
	ti_CloseAll();  //must use before any file i/o can be used
	//single-equal is correct. assigning result of open to file
	if (file=ti_Open("PlJmpDat","r")) {
		ti_Read(&gamedata,1,sizeof gamedata,file);
	} else {
		memset(&gamedata,0,sizeof gamedata);
	}
		
	while (1) {
		kb_Scan();
		kdir = kb_Data[7];
		kact = kb_Data[1];
		
		//Quick exit without needing to resort to GOTOs.
		if (state == GS_QUIT) break;
		
		switch (state) {
			case GS_TITLE   :
				srandom(random());  //cycle the seeds for RNG
				if (kact&kb_2nd) {
					if (!curopt) {
						if (kdir&(kb_Left|kb_Right)) nodeath = 1;
						else nodeath = 0;
						genSection(GEN_INIT);
						state = GS_GAMEPLAY;
						//tile_passed = tile_px_passed = 0;
						tile_px_passed = 32;
						tile_passed = 16;
						going = jumping = 0;
						lives = 5;
						score = 0;
						ball_x = (ball_min_x + ball_max_x - 32)/2;
						gamedata.number_of_times_ran++;
						changePalette(game_palette);
						keywait();
					} else if (curopt == 1) {
						state = GS_HELP;
					} else if (curopt == 2) {
						state = GS_CREDITS;
					} else if (curopt == 3) {
						state = GS_QUIT;
					}
						
				} else {
					if (kact&kb_Mode) state = GS_QUIT;
					if (kdir&kb_Down && curopt<3) ++curopt;
					if (kdir&kb_Up   && curopt>0) --curopt;
					drawTitleFrame();
					//inlined draw menu function
					gfx_SetTextScale(2,2);
					y = ((240-(4*25)+title_height) / 2) ;
					for (i=0; i<4; ++i,y+=25) {
						gfx_SetTextFGColor(COLOR_BLACK);
						for (j=0;j<6;j+=2) { //Draw shadows
							gfx_PrintStringXY( titleopts[i],
								(320-gfx_GetStringWidth(titleopts[i]))/2+shadow_offsets[j+0],
								y+shadow_offsets[j+1]
							);
						}
						if (i==curopt)	gfx_SetTextFGColor(COLOR_GOLD);
						else			gfx_SetTextFGColor(COLOR_GRAY);
						printTextCenter(titleopts[i],y);
					}
					//
					gfx_SetTextFGColor(COLOR_BLACK);
					gfx_SetTextScale(1,1);
					gfx_PrintStringXY("High score: ",5,230);
					gfx_PrintUInt(gamedata.hiscore,7);
				}
				break;
			
			case GS_GAMEPLAY:
				if (kact&kb_Mode) { 
					lives = 1;
					state = GS_QUITTING;
					ball_counter = 0;
					gamedata.number_of_times_quitted++;
					continue;
				}
				if (kdir&kb_Left && ball_x>=ball_min_x) {
					ball_x-=7;
				}
				if (kdir&kb_Right && ball_x<ball_max_x) {
					ball_x+=7;
				}
				if (kdir&kb_Up) going = 1;
				if (!jumping) {
					x = translate[translate_last_usable+32].startx;
					for (j=0,k=1;j<4;++j,k<<=1) {
						x += (unsigned int)translate[translate_last_usable+32].w[j];
						if ((ball_x+10) < x) break;
					}
					j = tile_passed+4;
					//if (tile_px_passed+16 > 32) j++;
					if (!(k&track[j]) & !nodeath) {
						state = GS_FALLING;
						continue;
					}
					y_velocity = 0;
					//Located here to prevent infinite air-jumping
					if (kact&kb_2nd && !jumping) {
						y_velocity = jumping = INITIAL_VELOCITY;
					}
				} else {
					y_velocity -= GRAVITY;
					jumping += y_velocity;
					if (jumping <= 0) {
						jumping = 0;
						x_offset = 1;
					}
				}
				//Move down
				if (going) {
					tile_px_passed += 3;
					ball_counter += 1;
				}
				if (tile_px_passed>=32) {
					tile_px_passed -= 32;
					if (!nodeath) ++score;
					if (--tile_passed<=0) {
						if (!nodeath) score += 7;
						tile_passed = 16;
						genSection(GEN_CONTINUE);
					}
				}
				gfx_SetTextFGColor(COLOR_BLACK);
				gfx_SetTextScale(1,1);
				gfx_SetTextXY(5,5);
				gfx_Wait();
				drawBG();
				if (score<=999999)	gfx_PrintUInt(score,6);
				else				gfx_PrintString("u broek score >:(");
				drawGameField(tile_passed,tile_px_passed,x_offset);
				//we need more frames.
				i = 127&(jumping>>8);
				j = i>>3;  //scale ball size between 0-15 -> 32-47 wrt yoffset
				k = 32+j;  //get new width of ball given height
				j = j>>1;  //retain ball centering by adjusting based on new w
				((uint8_t*)curball)[0] = k;
				((uint8_t*)curball)[1] = k;
				if (k==32) memcpy(curball,ballanim[(ball_counter>>1)&7],(32*32));
				else gfx_ScaleSprite(ballanim[(ball_counter>>1)&7],curball);
				//gfx_TransparentSprite_NoClip(ballanim[(ball_counter>>1)&7],ball_x,ball_y-(unsigned int)i);
				gfx_TransparentSprite_NoClip(curball,ball_x-j,ball_y-(unsigned int)i-j);
				//Changed out two conditionals for a lookup table
				x_offset = next_x_offset[x_offset];
				//if (x_offset == 1) x_offset = -1;
				//if (x_offset == -1) x_offset = 0;
				break;
			
			case GS_FALLING:
				if (ball_y > (1000)) {
					tile_px_passed = 0;
					state = GS_GAMEOVER;
					gamedata.number_of_times_died++;
					keywait();
					continue;
				}
				//Reverse the order of drawing ball and path to make it go under
				y_velocity -= GRAVITY;
				ball_y -= (y_velocity>>8);
				gfx_Wait();
				drawBG();
				gfx_TransparentSprite(ballanim[(++ball_counter>>1)&7],ball_x,ball_y);
				tile_px_passed += 1;
				if (tile_px_passed>=32) {
					tile_px_passed -= 32;
					if (--tile_passed<=0) {
						tile_passed = 16;
						genSection(GEN_CONTINUE);
					}
				}
				// gfx_SetTextXY(5,20);
				// gfx_PrintUInt(tile_passed,3);
				// gfx_PrintString(" : ");
				// gfx_PrintUInt(tile_px_passed,3);
				drawGameField(tile_passed,tile_px_passed,x_offset);
				break;
				
			case GS_QUITTING:
				if (ball_counter>=30) {
					state = GS_GAMEOVER;
					continue;
				}
				gfx_Wait();
				drawBG();
				drawGameField(tile_passed,tile_px_passed,x_offset);
				if (ball_counter<6) {
					zx7_Decompress(curball,explosion_ts_tiles_compressed[ball_counter]);
					for (idx=2;idx<(48*48+2);++idx) {
						((uint8_t*)curball)[idx] += COLOR_EXPLODESTART;
					}
					idx = 8+32;
					if (jumping) idx = (127&(jumping>>8)+16);
					gfx_SetTransparentColor(((uint8_t*)curball)[2]);
					((int*)explosion)[0] = (64+(256*64)); //sprdims: 64w,64h
					gfx_ScaleSprite(curball,explosion);
					gfx_TransparentSprite(explosion,ball_x-16,ball_y-idx);
					gfx_SetTransparentColor(COLOR_WHITE);
				}
				/*
				*/
				++ball_counter;
				break;
			
			case GS_GAMEOVER:
				ball_y = (240-32-8);  //reset after ball falls off edge
				gfx_SetColor(COLOR_BLACK);
				gfx_SetTextFGColor(COLOR_GOLD);
				gfx_FillRectangle(0,(240/2-30/2),320,30);
				--lives;
				if (lives) {
					if (lives>1) {
						printTextCenter("Lives remaining",(240/2-30/2+5));
						gfx_SetTextXY(((320-8)/2),(240/2-30/2+18));
						gfx_PrintUInt(lives,1);
					} else printTextCenter("This is your last chance",(240/2-30/2+11));
					for (i=3;i<5;++i) track[i+tile_passed] = 0xFF;
					state = GS_GAMEPLAY;
					jumping = going = 0;
				} else {
					if (score > gamedata.hiscore) {
						y = (240/2-30/2+5);
						printTextCenter("You achieved a new high score!",(240/2-30/2+18));
						gamedata.hiscore = score;
					} else {
						y = (240/2-30/2+11);
					}
					printTextCenter("Game Over",y);
					state = GS_TITLE;
				}
				gfx_SwapDraw();
				keyconfirm();
				if (state==GS_TITLE) changePalette(title_palette);
				continue;
				break;
				
			case GS_HELP :
				if (kact) state = GS_TITLE;
				drawTitleFrame();
				drawTextBlock(helptext,5);
				showStats(gamedata.number_of_times_died," - Deaths",210);
				showStats(gamedata.number_of_times_quitted," - Quits",220);
				showStats(gamedata.number_of_times_ran," - Runs",230);
				break;
			
			case GS_CREDITS :
				if (kact) state = GS_TITLE;
				drawTitleFrame();
				drawTextBlock(creditstext,7);
				break;
				
			default:
				break;
		}
		gfx_SwapDraw();
		
		//Debouncing, but not in-game.
		if ((kact|kdir) && !(state==GS_GAMEPLAY || state==GS_FALLING)) {
			kact = kdir = 0;
			keywait();
		}
	}

	gfx_End();
	
	//Saving game data.
	if (file=ti_Open("PlJmpDat","w")) {
		ti_Write(&gamedata,1,sizeof gamedata, file);
	} else {
		asm_ClrLCDFull();
		os_NewLine();
		os_PutStrFull("Could not save game data.");
	}
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
	gfx_FillScreen(((uint8_t*)titlebanner)[2]);
	gfx_Sprite(titlebanner,((320-255)/2),10);
	gfx_SetTextScale(1,1);
	gfx_SetTextFGColor(COLOR_BLACK);
	gfx_PrintStringXY(VERSION,320-32,240-10);
}


void printTextCenter(char *s,int y) {
	int w;
	w = gfx_GetStringWidth(s);
	gfx_PrintStringXY(s,(320-w)/2,y);
}

/* Vertically-centered menu */
/*
void drawMenu(char *sa[],uint8_t curopt,uint8_t maxopt) {
	uint8_t i,y,j;
	y = ((240-(maxopt*25)+title_height) / 2) ;
	gfx_SetTextScale(2,2);
	for (i=0; i<maxopt; ++i,y+=25) {
		gfx_SetTextFGColor(COLOR_BLACK);  //draw shadow at btm-right
		for (j=0;j<6;j+=2) {
			gfx_PrintStringXY(
				sa[i],
				(320-gfx_GetStringWidth(sa[i]))/2+shadow_offsets[(j+0)],
				y+shadow_offsets[(j+1)]
			);
		}
		if (i==curopt)	gfx_SetTextFGColor(COLOR_GOLD);
		else			gfx_SetTextFGColor(COLOR_GRAY);
		printTextCenter(sa[i],y);
	}
}*/

void showStats(int data, char *s, uint8_t ypos) {
	gfx_SetTextXY(5,ypos);
	gfx_PrintUInt(data,6);
	gfx_PrintString(s);
}

void drawTextBlock(char *sa[],uint8_t sa_len) {
	uint8_t i,w,y;
	int x;
	gfx_SetTextScale(1,1);
	gfx_SetTextFGColor(COLOR_BLACK);
	y = (240 + title_height - 12 - sa_len*12)/2;
	for (i=0;i<sa_len;++i,y+=12) {
		x = (320-gfx_GetStringWidth(sa[i]))/2;
		gfx_PrintStringXY(sa[i],x,y);
	}
}

void genSection_Random(void) {
	uint8_t i,d,z,pd;
	uint32_t dt;
	d = track[16];
	do {
		z = 0;
		for (z=i=0;i<16;i+=2) {
			dt = random();
			pd = d;
			d  = 15&(dt^(dt>>4));
			if (!(d&pd)) d |= dt>>8;
			if (!(d&pd)) d |= dt>>12;
			if (!d) ++z;
			track[i+1] = track[i] = d;
		}
	} while (z>2);  //ensures the look keeps going if too many empty spaces
}
void genSection_BinaryFill(void) {
	uint8_t i,d;
	for (i=0,d=3&random();i<16;++i) track[i] = ((i<<d)|((i&15)>>(4-d)));
}
//Return vals: 0=success, 1=(end of level) 2=(end of level pack)
//uses global level_pack to read in current level pack.
uint8_t genSection_LoadLevel(uint8_t level, uint8_t section) {
	uint8_t i,*s,*d;
	level_t *L;
	L = &level_pack[level];
	if (!L->num_segments) return 2;
	if (section >= L->num_segments) return 1;
	switch (L->level_type) {
		case LEVEL_TYPE_NORMAL:
			s = L->leveldata + (16*section);
			d = track + 15;
			for (i=0;i<16;++i,--d,++s) *d = *s;
			break;
		case LEVEL_TYPE_RANDOM:
			genSection_Random();
			break;
		case LEVEL_TYPE_BINARY:
			genSection_BinaryFill();
			break;
		default: return 1; // Unknown level type. skip to next level.
	}
	return 0;
}

/*
void genSection_AdvanceLevel(void) {
	memcpy(track+16,track,16);
}*/
	
void genSection(uint8_t init) {
	static uint8_t state=0;
	static uint8_t count=0;
	uint8_t result;
	
	memcpy(track+16,track,16);
	if (init) {
		count = state = 0;
		memset(track+16,0xFF,16);
		genSection_LoadLevel(state,0);
	} else {
		result = genSection_LoadLevel(state,++count);
		if (result==1) genSection_LoadLevel(++state,count=0);
		if (result==2) genSection_Random();
	}
}

void changePalette(uint16_t *palette) {
	gfx_FillScreen(255);
	gfx_SwapDraw();
	gfx_FillScreen(255);
	gfx_SwapDraw();
	gfx_SetPalette(palette,512,0);
	
}








