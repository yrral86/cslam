#ifndef __SWARM_H__
#define __SWARM_H__

#include <string.h>
#include <math.h>

#include "const.h"
#include "particle.h"
#include "random.h"

void swarm_init(int, int, int, int, int);
void swarm_move(int, int, int);
void swarm_update(int*);
int swarm_get_best_x();
int swarm_get_best_y();
int swarm_get_best_theta();

#endif
