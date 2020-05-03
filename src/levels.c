
#include <stdio.h>
#include <stdint.h>
#include "levels.h"
/* Note: Level pack data has to occur after all the other level definitions.
		Something about unknown array lengths, which can't resolve until
		after it runs through it?
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
	/*
	//Segment 4
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	//Segment 5
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	//Segment 6
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	//Segment 7
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	//Segment 8
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	LDAT(0,0,0,0)
	*/
};


level_t levelpack[] = {
	{level1,GET_NUMSEG(level1),LEVEL_TYPE_NORMAL },
	{NULL,0,0},
	
};





