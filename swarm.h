#ifndef __SWARM_H__
#define __SWARM_H__

#include "particle.h"
#include "sensor.h"
#include "slam.h"

#define PARTICLE_COUNT 200
#define INITIAL_SCANS 20
// 50 mm
#define INITIAL_POSITION_VARIANCE 50
#define INITIAL_ANGLE_VARIANCE 15

void swarm_init();
void swarm_filter(raw_sensor_scan*, uint8_t*, int);
particle swarm_get_best();

#endif
