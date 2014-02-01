#ifndef __ARENA_H__
#define __ARENA_H__

const static int OBSTACLE_COUNT = 6;

// ARENA_WIDTH and ARENA_HEIGHT must be defines
// to simplify buffer allocation
// all distances in this file are specified in millimeters

/*
// real arena
#define ARENA_WIDTH 7380
#define ARENA_HIEGHT 3880
const static int START_END = 1500;
// TODO we may want to add some tolerance to MINE_BEGIN so we make sure
// we don't mine early
const static  int MINE_BEGIN = 4440;
*/

/*
// lab arena
#define ARENA_WIDTH 3250
#define ARENA_HEIGHT 1700
const static int START_END = 500;
const static  int MINE_BEGIN = 1250;
*/


// home arena
#define ARENA_WIDTH 1765
#define ARENA_HEIGHT 800
const static int START_END = 500;
const static  int MINE_BEGIN = 1250;

#endif
