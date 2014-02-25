#ifndef __SWARM_H__
#define __SWARM_H__

#include <string.h>

#include "const.h"
#include "particle.h"
#include "sensor.h"
#include "slam.h"

void swarm_init();
void swarm_filter(raw_sensor_scan*, uint8_t*, int);
particle swarm_get_best();

#endif
