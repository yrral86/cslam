#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "urg_driver/urg_sensor.h"

#define SENSOR_MIN 20
#define SENSOR_MAX_ETH 60000
#define SENSOR_MAX_USB 5600
#define RAW_SENSOR_DISTANCES_ETH 1081
#define RAW_SENSOR_DISTANCES_USB 681
#define MAX_SCANS 30
#define SENSOR_RANGE_ETH 270.0
#define SENSOR_RANGE_USB 240.0
#define SENSOR_SPACING_ETH (SENSOR_RANGE_ETH/RAW_SENSOR_DISTANCES_ETH)
#define SENSOR_SPACING_USB (SENSOR_RANGE_USB/RAW_SENSOR_DISTANCES_USB)

// use max for all sensors
typedef struct raw_sensor_scan {
  int distances[RAW_SENSOR_DISTANCES_ETH];
} raw_sensor_scan;

pthread_t sensor_init_thread();
void* sensor_init(void*);
raw_sensor_scan sensor_read_raw();
void* sensor_read_raw_n(void*);
pthread_t sensor_read_raw_n_thread(int);
raw_sensor_scan sensor_fetch_index(int);
uint64_t utime();

#endif
