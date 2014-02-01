#ifndef __RECORD_H__
#define __RECORD_H__

#include <math.h>

#include "random.h"
#include "sensor.h"
#include "arena.h"

void init_map();
void record_distance_init(int, double);
void record_distance(int, double);
uint64_t utime();
void record_map_position(int, int, int, uint8_t);

#endif
