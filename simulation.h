#ifndef __SIMULATION_H__
#define __SIMULATION_H__

#include <clutter/clutter.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "particle.h"
#include "filter.h"
#include "fuzzy_controller.h"

void simulate();
static gboolean loop_iteration(gpointer);
void draw();
int rand_limit(int);

#endif
