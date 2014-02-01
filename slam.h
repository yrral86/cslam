#ifndef __SLAM_H__
#define __SLAM_H__

#include <math.h>

#include "random.h"
#include "sensor.h"
#include "arena.h"
#include "particle.h"

void init_map();
void record_distance_init(int, double);
void record_distance(int, int, double);
uint64_t utime();
void record_map_position(int, int, int, uint8_t);
int index_from_x_y(int, int);
int in_bounds(int, int);

#endif
