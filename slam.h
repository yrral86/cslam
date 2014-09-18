#ifndef __SLAM_H__
#define __SLAM_H__

#include <math.h>
#include <pthread.h>

#include "arena.h"
#include "buffer.h"
#include "landmark.h"
#include "particle.h"
#include "random.h"
#include "sensor.h"
#include "swarm.h"
#include "map.h"

map_node* build_submap(raw_sensor_scan*);
void crossover(int*, int, int, int, int);
void mutate(int*, int);

#endif
