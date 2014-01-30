#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <math.h>

#include "scip/scipBase.h"
#include "scip/scipUtil.h"

#define SENSOR_MIN 20
#define SENSOR_MAX 5600
#define SENSOR_DISTANCES 16
#define RAW_SENSOR_DISTANCES 681

typedef struct sensor_scan {
  int distances[SENSOR_DISTANCES];
} sensor_scan;

typedef struct raw_sensor_scan {
  int distances[RAW_SENSOR_DISTANCES];
} raw_sensor_scan;

#include "particle.h"
#include "simulation.h"

int in_bounds(int, int);
int sensor_distance_offset(particle, double);
sensor_scan sensor_distance(particle);
sensor_scan sensor_read();
double sensor_distance_index_to_radians(int);
double sensor_distance_index_to_degrees(int);
void sensor_init();
raw_sensor_scan sensor_read_raw();

#endif
