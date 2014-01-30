#ifndef __SIMULATION_H__
#define __SIMULATION_H__

#include <clutter/clutter.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAX_PARTICLES 500

#include "particle.h"
#include "fuzzy_controller.h"
#include "robot.h"
#include "swarm.h"

//const static int ARENA_WIDTH = 7380;
const static int ARENA_WIDTH = 1765;
//const static int ARENA_HEIGHT = 3880;
const static int ARENA_HEIGHT = 800;
const static int START_END = 150;
const static  int MINE_BEGIN = 444;
const static int OBSTACLE_COUNT = 6;
const static int PARTICLE_SIZE = 10;

void simulate();
static gboolean loop_iteration(gpointer);
static gboolean draw_iteration(gpointer);
void draw();
int rand_limit(int);

#endif
