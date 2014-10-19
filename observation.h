#ifndef __OBSERVATION_H__
#define __OBSERVATION_H__

#include "sensor.h"

typedef struct observation {
  unsigned int r;
  double theta;
} observation;

typedef struct observations {
  observation list[RAW_SENSOR_DISTANCES_USB];
  struct hypothesis **hypotheses;
} observations;

#endif
