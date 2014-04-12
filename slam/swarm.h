#ifndef __SWARM_H__
#define __SWARM_H__

#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "const.h"
#include "particle.h"
#include "random.h"
#include "buffer.h"

#ifndef LINUX
__declspec(dllexport) void swarm_init(int, int, int, int, int);
__declspec(dllexport) void swarm_move(int, int, int);
__declspec(dllexport) void swarm_update(int*);
__declspec(dllexport) void swarm_map(int*)
__declspec(dllexport) int swarm_get_best_x();
__declspec(dllexport) int swarm_get_best_y();
__declspec(dllexport) int swarm_get_best_theta();
void swarm_init_internal(int, int, int, int, int);
void swarm_move_internal(int, int, int);
void swarm_update_internal(int*);
void swarm_map_internal(int*);
int swarm_get_best_x_internal();
int swarm_get_best_y_internal();
int swarm_get_best_theta_internal();
#endif
#ifdef LINUX
void swarm_init(int, int, int, int, int);
void swarm_move(int, int, int);
void swarm_update(int*);
void swarm_map(int*);
int swarm_get_best_x();
int swarm_get_best_y();
int swarm_get_best_theta();
#endif
void swarm_get_best_buffer(uint8_t*);
void swarm_get_map_buffer(uint8_t*);
landmark_map swarm_get_map();
int in_arena(int, int);

#endif
