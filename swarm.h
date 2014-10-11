#ifndef __SWARM_H__
#define __SWARM_H__

#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "const.h"
#include "particle.h"
#include "random.h"
#include "buffer.h"
#include "map.h"

#ifndef LINUX
__declspec(dllexport) void swarm_init(int, int, int, int, int, int);
__declspec(dllexport) void swarm_move(int, int, int);
__declspec(dllexport) void swarm_update(int*);
__declspec(dllexport) void swarm_update_finalize();
__declspec(dllexport) void swarm_map(int*);
__declspec(dllexport) int swarm_converged();
__declspec(dllexport) int swarm_get_best_x();
__declspec(dllexport) int swarm_get_best_y();
__declspec(dllexport) int swarm_get_best_theta();
void swarm_init_internal(int, int, int, int, int);
void swarm_move_internal(int, int, int);
void swarm_update_internal(int*);
void swarm_map_internal(int*);
int swarm_converged_internal();
int swarm_get_best_x_internal();
int swarm_get_best_y_internal();
int swarm_get_best_theta_internal();
int swarm_get_x(int);
int swarm_get_y(int);
int swarm_get_theta(int);
void swarm_get_best_buffer(uint8_t*);
void swarm_get_map_buffer(uint8_t*);
map_node* swarm_get_map();
void swarm_get_all_particles(particle**);
void swarm_map(int*);
void swarm_map_current(int*);
void swarm_map_reset_current();
#endif
#ifdef LINUX
void swarm_init(int, int, int, int, int, int);
void swarm_move(int, int, int);
void swarm_update(observations*);
int swarm_converged();
int swarm_get_best_x();
int swarm_get_best_y();
int swarm_get_best_theta();
#endif
int in_arena(int, int);
void swarm_set_map(uint8_t*);
void swarm_normalize();
void swarm_sort(int, int);
int swarm_partition(int, int);
void swarm_reset_convergence();

#endif
