/*
 *--------------------------------------
 * Program Name: Plane Jump
 * Author: idk anymore
 * License: MIT
 * Description: Roll a ball and jump over gaps.
 *--------------------------------------
*/

/* TODO: MAYBE ADD ACTUAL LEVELS INSTEAD OF SOME PIECE OF CRAP INITIAL 
   THING WHICH IS BUGGED TO HELL AND THEN RELY ON THE RANDOM NUMBER GOD
   TO GENERATE FAVOR WITH THE MASSES.
   
   AND MAYBE SIZE-OPTIMIZE THE GAME ITSELF. Because making the graphics
   crappier to get them to compress better shouldn't be the go-to solution.
*/

#define VERSION "v0.3"

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

#define GRAVITY ((int)(0.38 * 256))
#define INITIAL_VELOCITY ((int)(9*256))

// Two lines of text, y positions centered on the game over banner.
#define GAMEOVER_YPOS_2L_1 (240/2-30/2+5)
#define GAMEOVER_YPOS_2L_2 (240/2-30/2+18)
// One line of text, y position centered on the game over banner.
#define GAMEOVER_YPOS_1L_1 (240/2-30/2+11)

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
#include <sys/rtc.h>

/* External library headers */
#include <debug.h>
#include <keypadc.h>
#include <graphx.h>
#include <fileioc.h>
#include <compression.h>

#include "draw.h"
#include "levels.h"
#include "gfx/out/gfx.h"
#include "gfx/out/ball_ts.h"
#include "gfx/out/explosion_ts_2.h"
#include "gfx/out/title.h"

//COLOR_OFFSET defined in gfx/convimg.yaml
//NOTE: using title_palette_offset since that is the least likely name to change.
#define COLOR_OFFSET(x) (title_palette_offset+x)
#define COLOR_TRANSPARENT COLOR_OFFSET(0)
#define COLOR_GOLD        COLOR_OFFSET(1)
#define COLOR_GRAY        COLOR_OFFSET(2)
#define COLOR_BLACK       COLOR_OFFSET(3)
#define COLOR_BACKGROUND  COLOR_OFFSET(4)
#define COLOR_DAKRBLUE	  COLOR_OFFSET(5)
//End fixed-entry range

typedef struct thing {
	uint8_t ypos;
	int startx;
	uint8_t w[4];
} scanline_t;

typedef struct { // :)
	float x;
	float y;
	float z;
} vector_t;

typedef struct {
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
void printTextCenter(const char *s,int y);
void drawTextBlock(const char *sa[],uint8_t sa_len);
void drawTitleFrame(void);
void genSection(uint8_t init);  //generates latter 16 blocks of level
gamedata_t gamedata;


/* 	Global graphics objects. Most objects will be unpacked into RAM.
	What we won't be unpacking are the explosion sprites. We have all
	the time in the world to unpack them as we go.
*/
gfx_UninitedRLETSprite(title_banner, title_size);
gfx_UninitedRLETSprite(ball_1, 1+ 32 * 32);
gfx_UninitedRLETSprite(ball_2, 1+ 32 * 32);
gfx_UninitedRLETSprite(ball_3, 1+ 32 * 32);
gfx_UninitedRLETSprite(ball_4, 1+ 32 * 32);
gfx_UninitedRLETSprite(ball_5, 1+ 32 * 32);
gfx_UninitedRLETSprite(ball_6, 1+ 32 * 32);
gfx_UninitedRLETSprite(ball_7, 1+ 32 * 32);
gfx_UninitedRLETSprite(ball_8, 1+ 32 * 32);
gfx_UninitedSprite(normalball_1, 32, 32);
gfx_UninitedSprite(normalball_2, 32, 32);
gfx_UninitedSprite(normalball_3, 32, 32);
gfx_UninitedSprite(normalball_4, 32, 32);
gfx_UninitedSprite(normalball_5, 32, 32);
gfx_UninitedSprite(normalball_6, 32, 32);
gfx_UninitedSprite(normalball_7, 32, 32);
gfx_UninitedSprite(normalball_8, 32, 32);
gfx_rletsprite_t *ballanim[8];
gfx_sprite_t *normalballanim[8];
gfx_UninitedSprite(explosion, 24, 24);
gfx_UninitedSprite(resized_ball, 64, 64);
gfx_UninitedSprite(ball_temp, 32, 32); 	//For undoing RLET during scaling.



//Globals. For performance reasons, locals may not be more than 128 bytes
//total per function (or scope?), so things too large should also be here.
uint8_t track[32];
scanline_t translate[240+32+32];  //32 above and below
int translate_last_usable;
level_t *level_pack;     //level pack currently loaded

int8_t object99[] = {0,0,-1};			//For use in shake effect.
int8_t *next_x_offset = &object99[1]; 	//For inputs -1,0, and 1 to be correct.
const char *titleopts[] = {
	"Start game",
	"Help",
	"About",
	"Quit"
};
const char *helptext[] = {
	"[2nd] : Jump",
	"[MODE] : Quit",
	"[Up] : Go forward",
	"[Left] : Go left",
	"[Right] : Go right"
};
const char *creditstext[] = {
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

/* ************************************************************************ */

int main(void) {
	ti_var_t file;
	kb_key_t keys;
	int8_t tile_passed;     //increments. once 16, reset and gen new section
	uint8_t state,i,j,k;
	uint8_t tile_px_passed;  //0-32, is vertical offset of current tile.
	uint8_t y;
	uint8_t ball_counter;    //0-7, index to ballanim
	uint8_t going;
	uint8_t lives;
	uint8_t curopt;
	uint8_t nodeath;
	unsigned int score;
	int x;
	int ball_x;
	int ball_min_x;
	int ball_max_x;
	int ball_y;
	int jumping;     //continue to use this as the y offset. is 16.8 fp
	int y_velocity;  //16.8fp
	int x_offset;    //for subtle shake effect on landing
	void *unsafe_ball_pointer;	//Shrodinger's ball sprite. RLET or not?
	
	/* 
		Initialize draw system
	*/
	gfx_Begin();
	gfx_SetColor(COLOR_BLACK);
	gfx_SetTransparentColor(COLOR_TRANSPARENT);
	gfx_SetDrawBuffer();
	memcpy(&gfx_palette[COLOR_OFFSET(0)], game_set, sizeof game_set);
	zx0_Decompress(title_banner, title_compressed);
	ballanim[0] = ball_1;
	ballanim[1] = ball_2;
	ballanim[2] = ball_3;
	ballanim[3] = ball_4;
	ballanim[4] = ball_5;
	ballanim[5] = ball_6;
	ballanim[6] = ball_7;
	ballanim[7] = ball_8;
	normalballanim[0] = normalball_1;
	normalballanim[1] = normalball_2;
	normalballanim[2] = normalball_3;
	normalballanim[3] = normalball_4;
	normalballanim[4] = normalball_5;
	normalballanim[5] = normalball_6;
	normalballanim[6] = normalball_7;
	normalballanim[7] = normalball_8;
	for (i=0; i<ball_ts_num_tiles; ++i) {
		zx0_Decompress(resized_ball, ball_ts_tiles_compressed[i]);
		gfx_ConvertToRLETSprite(resized_ball, ballanim[i]);
		gfx_ConvertFromRLETSprite(ballanim[i], normalballanim[i]);
	}

	/* 
		Initialize game state
	*/

	level_pack = levelpack; //INTERNAL LEVEL PACK
	y_velocity = x_offset = jumping = score = 0;
	nodeath = curopt = going = ball_counter = lives = 0;
	tile_px_passed = tile_passed = state = 0;

	//Generates the translate array inline with this function
	{
		int yi,wi,werr;
		float yf,x1,y1,z1,x2;
		
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
	}

	ball_max_x = ball_min_x = translate[translate_last_usable+32].startx;
	for (i=0;i<4;++i) ball_max_x += translate[translate_last_usable+32].w[i];
	ball_max_x -= (32);
	ball_x = ball_min_x;
	ball_y = (240-32-8);
	
	//Reading previously saved data
	if ((file=ti_Open("PlJmpDat","r"))) {
		ti_Read(&gamedata, 1, sizeof gamedata, file);
		ti_Close(file);
	} else {
		memset(&gamedata, 0, sizeof gamedata);
	}
		
	/*
		Main game loop. State-based.
	*/

	while (1) {
		kb_Scan();
		keys = kb_Data[7] | kb_Data[1];

		if (state == GS_TITLE || state == GS_HELP || state == GS_CREDITS) {
			// The gfx_SwapDraw() inside this does double duty of having
			// something nontrivial to run while debouncing while retaining
			// the perception of responsiveness during otherwise dead time.
			while (kb_AnyKey()) gfx_SwapDraw();
		}

		//Quick exit without needing to resort to GOTOs.
		if (state == GS_QUIT) break;
		
		switch (state) {
			case GS_TITLE:
				srandom(rtc_Time());  //cycle the seeds for RNG
				if (keys & kb_2nd) {
					// Action key. This is where we process the consequences
					// of our selection.
					if (!curopt) {
						if (keys & (kb_Left | kb_Right)) {
							nodeath = 1;
						} else { 
							nodeath = 0; 
						}
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
						keywait();
					} else if (curopt == 1) {
						state = GS_HELP;
					} else if (curopt == 2) {
						state = GS_CREDITS;
					} else if (curopt == 3) {
						state = GS_QUIT;
					}
						
				} else {
					if (keys & kb_Mode) state = GS_QUIT;
					if (keys & kb_Down) curopt = (curopt+1) & 3;
					if (keys & kb_Up  ) curopt = (curopt-1) & 3;
					drawTitleFrame();
					// Inlined draw menu function, for title screen.
					gfx_SetTextScale(2,2);
					y = ((240-(4*25)+title_height) / 2) ;
					for (i=0; i<4; ++i, y+=25) {
						const char *s = titleopts[i];
						//Draw text shadow.
						gfx_SetTextFGColor(COLOR_BLACK);
						int x = 160-(gfx_GetStringWidth(s)>>1);
						gfx_PrintStringXY(s, x+1, y+1);
						//gfx_PrintStringXY(s, x+0, y+1);
						//gfx_PrintStringXY(s, x+1, y+0);
						if (i==curopt)	gfx_SetTextFGColor(COLOR_GOLD);
						else			gfx_SetTextFGColor(COLOR_GRAY);
						printTextCenter(s,y);
					}
					//
					gfx_SetTextFGColor(COLOR_BLACK);
					gfx_SetTextScale(1,1);
					gfx_PrintStringXY("High score: ",5,230);
					gfx_PrintUInt(gamedata.hiscore,7);
				}
				break;
			
			case GS_GAMEPLAY:
				if (keys & kb_Left && ball_x>=ball_min_x) {
					ball_x-=7;
				}
				if (keys & kb_Right && ball_x<ball_max_x) {
					ball_x+=7;
				}
				if (keys & kb_Up) {
					going = 1;
				} 
				//NOTE: 2nd and MODE are handled in the jump logic below.
				//NOTE: These notes are for future me. I spent an embarrasingly
				// long time trying to figure out where the 2nd key was handled.
				//NOTE: NEVER AGAIN.
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
					if (keys & kb_2nd && !jumping) {
						y_velocity = jumping = INITIAL_VELOCITY;
					}
					//Located here to prevent mid-air quitting.
					//I'm going to pretend this fix is to prevent fat-fingering
					//the quit button while jumping. The real reason is because
					//I optimized away the explosion animation during a jump.
					if (keys & kb_Mode) {
						lives = 1;
						state = GS_QUITTING;
						ball_counter = 0;
						gamedata.number_of_times_quitted++;
						continue;
					}
					// As a reminder, this branch is what happens when you
					// are NOT jumping. My screen isn't tall. And I need to
					// add more jump-dependent logic.
					unsafe_ball_pointer = ballanim[(ball_counter>>1)&7];
					k = 32;  //default size of ball
				} 
				if (jumping) {
					// This code must run on jump state change, which is why
					// it's here instead of the above's else statement.
					// This fixes a glitch that really should've caused a crash.
					y_velocity -= GRAVITY;
					jumping += y_velocity;
					if (jumping <= 0) {
						jumping = 0;
						x_offset = 1;
						unsafe_ball_pointer = ballanim[(ball_counter>>1)&7];
					} else {
						i = 127&(jumping>>8);
						j = i>>3;  //scale ball size between 0-15 -> 32-47 wrt yoffset
						k = 32+j;  //get new width of ball given height
						j = j>>1;  //retain ball centering by adjusting based on new w
						unsafe_ball_pointer = normalballanim[(ball_counter>>1)&7];
						resized_ball->width = k;
						resized_ball->height = k;
						gfx_ScaleSprite(unsafe_ball_pointer, resized_ball);
						unsafe_ball_pointer = resized_ball;
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
				x_offset = next_x_offset[x_offset];
				gfx_SetTextXY(5,5);

				//Begin graphics render section.
				gfx_Wait();
				drawBG();
				if (score<=999999)	gfx_PrintUInt(score,6);
				else				gfx_PrintString("u broek score >:(");
				drawGameField(tile_passed,tile_px_passed,x_offset);
				if (jumping) {
					gfx_TransparentSprite_NoClip(unsafe_ball_pointer,ball_x-j, ball_y-i-j);
				} else {
					gfx_RLETSprite_NoClip(unsafe_ball_pointer,ball_x, ball_y);
				}
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
				gfx_RLETSprite(ballanim[(++ball_counter>>1)&7],ball_x,ball_y);
				tile_px_passed += 1;
				if (tile_px_passed>=32) {
					tile_px_passed -= 32;
					if (--tile_passed<=0) {
						tile_passed = 16;
						genSection(GEN_CONTINUE);
					}
				}
				drawGameField(tile_passed,tile_px_passed,x_offset);
				break;
				
			case GS_QUITTING:
				if (ball_counter>=40) {
					state = GS_GAMEOVER;
					continue;
				}
				gfx_Wait();
				drawBG();
				drawGameField(tile_passed,tile_px_passed,x_offset);
				if (ball_counter<20) {
					zx0_Decompress(explosion, explosion_ts_2_tiles_compressed[ball_counter>>1]);
					resized_ball->width = 64;
					resized_ball->height = 64;
					//Reusing the resized ball as a temporary buffer.
					gfx_ScaleSprite(explosion, resized_ball);
					gfx_TransparentSprite(resized_ball, ball_x-16, ball_y-32);
				}
				++ball_counter;
				break;
			
			case GS_GAMEOVER:
				ball_y = (240-32-8);  //reset after ball falls off edge
				--lives;
				// On next life (if any) fill out track to prevent softlock.
				for (i=3;i<5;++i) track[i+tile_passed] = 0xFF;
				gfx_SetTextFGColor(COLOR_GOLD);
				gfx_FillRectangle(0,(240/2-30/2),320,30);
				if (lives) {
					state = GS_GAMEPLAY;
					jumping = going = 0;
					if (lives>1) {
						printTextCenter("Lives remaining",GAMEOVER_YPOS_2L_1);
						gfx_SetTextXY(((320-8)/2),GAMEOVER_YPOS_2L_2);
						gfx_PrintUInt(lives,1);
					} else  {
						printTextCenter("This is your last chance",GAMEOVER_YPOS_1L_1);
					}
				} else {
					state = GS_TITLE;
					if (score > gamedata.hiscore) {
						y = GAMEOVER_YPOS_2L_1;
						printTextCenter("You achieved a new high score!",GAMEOVER_YPOS_2L_2);
						gamedata.hiscore = score;
					} else {
						y = GAMEOVER_YPOS_1L_1;
					}
					printTextCenter("Game Over",y);
				}
				gfx_SetTextFGColor(COLOR_BLACK);
				gfx_SwapDraw();
				keywait();
				while (!kb_AnyKey());
				keywait();
				continue;	//No need to gfx_SwapDraw twice.
				
			case GS_HELP :
				if (keys) state = GS_TITLE;
				drawTitleFrame();
				drawTextBlock(helptext,5);
				gfx_SetTextXY(5, 210);
				gfx_PrintUInt(gamedata.number_of_times_died, 6);
				gfx_PrintString(" - Deaths");
				gfx_SetTextXY(5, 220);
				gfx_PrintUInt(gamedata.number_of_times_quitted, 6);
				gfx_PrintString(" - Quits");
				gfx_SetTextXY(5, 230);
				gfx_PrintUInt(gamedata.number_of_times_ran, 6);
				gfx_PrintString(" - Runs");
				break;
			
			case GS_CREDITS :
				if (keys) state = GS_TITLE;
				drawTitleFrame();
				drawTextBlock(creditstext,7);
				break;
				
			default:
				state = GS_QUIT;
				break;
		}
		// This is supposed to be the final action of the loop.
		// Any and all drawing routines are intended to be as close to this
		// side of the loop as possible. All non-rendering logic needs to
		// be logically closest to the top of the loop as possible. "Logically"
		// because the size of those switch cases can warp the perception of
		// where the "top" of the loop is.
		gfx_SwapDraw();
	}

	gfx_End();
	
	//Saving game data.

	if ((file=ti_Open("PlJmpDat","w"))) {
		ti_Write(&gamedata,1,sizeof gamedata, file);
		ti_SetArchiveStatus(true, file);
		ti_Close(file);
	} else {
		asm_ClrLCDFull();
		os_NewLine();
		os_PutStrFull("Could not save game data.");
	}
}


void keywait(void) {
	while (kb_AnyKey());  //Pauses until keys are released
}

void drawTitleFrame(void) {
	gfx_FillScreen(COLOR_BACKGROUND);
	gfx_RLETSprite_NoClip(title_banner,((320-255)/2),10);
	gfx_PrintStringXY(VERSION,320-32,240-10);
}


void printTextCenter(const char *s, int y) {
	int w;
	w = gfx_GetStringWidth(s);
	gfx_PrintStringXY(s,(320-w)/2,y);
}

void drawTextBlock(const char *sa[], uint8_t sa_len) {
	uint8_t i,y;
	int x;
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







