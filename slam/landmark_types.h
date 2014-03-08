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
  landmark map[BUFFER_SIZE];
} landmark_map;

#endif
