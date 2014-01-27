#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <math.h>

#include "particle.h"

int sensor_distance_offset(particle, double);
int sensor_distance_forward(particle);
int sensor_distance_reverse(particle);
int sensor_distance_left(particle);
int sensor_distance_right(particle);

#endif
