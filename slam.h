#ifndef __SLAM_H__
#define __SLAM_H__

#include <math.h>
#include <pthread.h>

#include "buffer.h"
#include "landmark.h"
#include "particle.h"
#include "random.h"
#include "sensor.h"
#include "swarm.h"

void record_map_position(int, int, int, uint8_t);

#endif
