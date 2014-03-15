#ifndef __SWARM_H__
#define __SWARM_H__

#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "const.h"
#include "particle.h"
#include "random.h"
#include "buffer.h"

__declspec(dllexport) void swarm_init(int, int, int, int, int);
__declspec(dllexport) void swarm_move(int, int, int);
__declspec(dllexport) void swarm_update(int*);
__declspec(dllexport) int swarm_get_best_x();
__declspec(dllexport) int swarm_get_best_y();
__declspec(dllexport) int swarm_get_best_theta();
__declspec(dllexport) void swarm_get_best_buffer(uint8_t*);
int in_arena(int, int);

#endif
