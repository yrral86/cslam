#ifndef __HYPOTHESIS_H__
#define __HYPOTHESIS_H__

#include "observation.h"
#include "const.h"

typedef struct hypothesis {
  struct hypothesis* parent;
  struct hypothesis* children[PARTICLE_COUNT+10];
  double x;
  double y;
  double theta;
  observations *obs;
  struct map_node *map;
  uint8_t *buffer;
  int references;
  int child_count;
} hypothesis;

#include "map.h"

void hypothesis_reference(hypothesis*);
void hypothesis_dereference(hypothesis*);
hypothesis* hypothesis_new(hypothesis*, double, double, double);


#endif
