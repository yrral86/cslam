#ifndef __CHECKPOINT_H__
#define __CHECKPOINT_H__

#include "map.h"

typedef struct checkpoint {
  unsigned int x;
  unsigned int y;
  int theta;
  double information;
  unsigned int size;
  map_node *observation;
  struct checkpoint *next;
  struct checkpoint *previous;
} checkpoint;

#endif
