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
#include "arena.h"

const static int PARTICLE_SIZE = 10;

void simulate();
static gboolean loop_iteration(gpointer);
static gboolean draw_iteration(gpointer);
void draw();

#endif
