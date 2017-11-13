#ifndef __CONST_H__
#define __CONST_H__

//#include "arena.h"

#define PARTICLE_COUNT 20000
#define INITIAL_POSITION_VARIANCE 150
#define INITIAL_ANGLE_VARIANCE 5
#define INITIAL_PARTICLE_FACTOR 1
// CULLING_FACTOR: 1 -> no culling
//                 2 -> evaluate 50% then cull
//                 10 -> evaluate 10% then cull
//                 100 -> evaluate 1% then cull
#define CULLING_FACTOR 50
#define CULLING_PERCENT 0.9

#define BUFFER_FACTOR 20
#define BORDER_WIDTH 150
/*
#define ARENA_WIDTH 7380
#define ARENA_HEIGHT 3880
#define BUFFER_WIDTH (ARENA_WIDTH/BUFFER_FACTOR)
#define BUFFER_HEIGHT (ARENA_HEIGHT/BUFFER_FACTOR)
#define BUFFER_SIZE (BUFFER_WIDTH*BUFFER_HEIGHT)
*/
#endif
