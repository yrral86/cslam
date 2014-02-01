#ifndef __SENSOR_H__
#define __SENSOR_H__

#include "scip/scipBase.h"
#include "scip/scipUtil.h"

#define SENSOR_MIN 20
#define SENSOR_MAX 5600
#define RAW_SENSOR_DISTANCES 681

typedef struct raw_sensor_scan {
  int distances[RAW_SENSOR_DISTANCES];
} raw_sensor_scan;

void sensor_init();
raw_sensor_scan sensor_read_raw();

#endif
