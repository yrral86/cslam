#ifndef __SIMULATION_H__
#define __SIMULATION_H__

#include <clutter/clutter.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "particle.h"
#include "filter.h"
#include "fuzzy_controller.h"
#include "robot.h"

const static int ARENA_WIDTH = 738;
const static int ARENA_HEIGHT = 388;
const static int START_END = 150;
const static  int MINE_BEGIN = 444;
const static int MAX_PARTICLES = 150;
const static int OBSTACLE_COUNT = 6;
const static int PARTICLE_SIZE = 10;

void initialize_swarm();
void simulate();
static gboolean loop_iteration(gpointer);
void draw();
int rand_limit(int);

#endif
