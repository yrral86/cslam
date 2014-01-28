#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <math.h>

#define SENSOR_DISTANCES 8

typedef struct sensor_scan {
  int distances[SENSOR_DISTANCES];
} sensor_scan;

#include "particle.h"
#include "simulation.h"

int sensor_distance_offset(particle, double);
sensor_scan sensor_distance(particle);
double sensor_distance_index_to_radians(int);

#endif
