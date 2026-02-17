
#include <stdio.h>
#include <stdint.h>
#include "levels.h"
/* Note: Level pack data has to occur after all the other level definitions.
		Something about unknown array lengths, which can't resolve until
		after it runs through it?
*/

/* 	Level construction notes:
	Segments contain 16 LDAT()s, which are 4-bit values representing the
	presence of tiles in each column of the segment.
	Each level must contain at least one segment. All segments must be
	whole (16 LDATs). The game will read in segments until it reaches the end of
	the level, then move on to the next level. The game will read in levels until
	it reaches the end of the level pack, at which point it will generate random
	segments for the rest of the run.
*/
uint8_t level1[] = {
	//Segment 1
	LDAT(1,1,0,0)
	LDAT(1,1,0,0)
	LDAT(0,1,0,0)
	LDAT(0,1,0,0)
	LDAT(0,1,0,0)
	LDAT(1,1,0,0)
	LDAT(1,1,0,0)
	LDAT(1,1,1,0)
	LDAT(0,1,1,0)
	LDAT(0,1,1,1)
	LDAT(0,0,1,1)
	LDAT(0,0,1,1)
	LDAT(0,1,1,0)
	LDAT(0,1,1,0)
	LDAT(1,1,1,1)
	LDAT(1,1,1,1)
	//Segment 2
	LDAT(1,0,0,1)
	LDAT(1,0,0,1)
	LDAT(1,0,0,1)
	LDAT(1,0,0,1)
	LDAT(1,1,1,1)
	LDAT(1,1,1,1)
	LDAT(0,1,1,0)
	LDAT(0,1,1,0)
	LDAT(0,1,0,1)
	LDAT(0,1,0,1)
	LDAT(0,1,1,1)
	LDAT(0,1,1,1)
	LDAT(1,0,1,0)
	LDAT(1,0,1,0)
	LDAT(1,0,1,0)
	LDAT(1,0,1,0)
	//Segment 3
	LDAT(1,0,0,0)
	LDAT(1,0,0,0)
	LDAT(1,0,0,0)
	LDAT(1,0,0,0)
	LDAT(1,0,0,0)
	LDAT(1,1,0,0)
	LDAT(1,1,1,0)
	LDAT(1,1,1,1)
	LDAT(1,0,0,0)
	LDAT(1,1,0,0)
	LDAT(1,1,1,0)
	LDAT(1,1,1,1)
	LDAT(1,0,0,0)
	LDAT(1,1,0,0)
	LDAT(1,1,1,0)
	LDAT(1,1,1,1)
};


level_t levelpack[] = {
	{level1,GET_NUMSEG(level1),LEVEL_TYPE_NORMAL },
	{NULL,0,0},
	
};





