#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
/*
#include "scip/scipBase.h"
#include "scip/scipUtil.h"
*/

#include "urg_driver/urg_sensor.h"

#define SENSOR_MIN 20
//#define SENSOR_MAX 5600
#define SENSOR_MAX 60000
#define RAW_SENSOR_DISTANCES 1081
#define MAX_SCANS 30
//#define SENSOR_RANGE 240.0
#define SENSOR_RANGE 270.0
#define SENSOR_SPACING (SENSOR_RANGE/RAW_SENSOR_DISTANCES)

typedef struct raw_sensor_scan {
  int distances[RAW_SENSOR_DISTANCES];
} raw_sensor_scan;

pthread_t sensor_init_thread();
void* sensor_init(void*);
raw_sensor_scan sensor_read_raw();
void* sensor_read_raw_n(void*);
pthread_t sensor_read_raw_n_thread(int);
raw_sensor_scan sensor_fetch_index(int);
uint64_t utime();

#endif
