#ifndef __pjump_levels__
#define __pjump_levels__

#include <stdint.h>

/* leveldata may be null if level type does not require it
   num_segments may not be zero. If it is, it's considered a zero-terminator.
*/

//No packing. Going to use this format for now
//until we need to start saving space.
#define LDAT(a,b,c,d) ((a<<0)|(b<<1)|(c<<2)|(d<<3)),
//packs to nibbles. nib0 is LDAT0, nib1 is LDAT1
#define LDAT1(a,b,c,d) ((a<<0)|(b<<1)|(c<<2)|(d<<3)|
#define LDAT2(e,f,g,h) (e<<4)|(f<<5)|(g<<6)|(h<<7)),

//Level types
#define LEVEL_TYPE_NORMAL 0
#define LEVEL_TYPE_RANDOM 1
#define LEVEL_TYPE_BINARY 2
//For constructing level packs more concise.
#define GET_NUMSEG(x) ((sizeof(x)/sizeof(x[0]))/16)

typedef struct thing_lv1 {
	uint8_t *leveldata;
	uint8_t num_segments;
	uint8_t level_type;
} level_t;
extern uint8_t level1[];
extern level_t levelpack[];

#endif