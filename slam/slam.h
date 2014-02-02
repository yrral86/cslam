#ifndef __SLAM_H__
#define __SLAM_H__

#include <math.h>
#include <pthread.h>

#include "arena.h"
#include "buffer.h"
#include "particle.h"
#include "random.h"
#include "sensor.h"
#include "swarm.h"

void record_distance_init(int, double);
void record_distance(int, double);
void record_map_position(int, int, int, uint8_t);
int in_arena(int, int);

#endif
