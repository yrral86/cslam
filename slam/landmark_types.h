#ifndef __LANDMARK_TYPES_H__
#define __LANDMARK_TYPES_H__

#include "const.h"

typedef struct landmark {
  double x;
  double y;
  unsigned int seen;
  unsigned int unseen;
} landmark;

typedef struct landmark_map {
  int references;
  landmark *map;
} landmark_map;

#endif
