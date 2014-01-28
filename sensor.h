#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <math.h>

#include "particle.h"

#define SENSOR_DISTANCES 4

typedef struct sensor_scan {
  int distances[SENSOR_DISTANCES];
} sensor_scan;

int sensor_distance_offset(particle, double);
sensor_scan sensor_distance(particle);

#endif
