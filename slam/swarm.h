#ifndef __SWARM_H__
#define __SWARM_H__

#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define log2(x) (log(x)/log(2))

#include "const.h"
#include "particle.h"
#include "random.h"

void swarm_init(int, int, int, int, int);
void swarm_move(int, int, int);
void swarm_update(int*);
int swarm_get_best_x();
int swarm_get_best_y();
int swarm_get_best_theta();
void swarm_get_best_buffer(uint8_t*);
int in_arena(int, int);

#endif
