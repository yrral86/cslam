#ifndef __HYPOTHESIS_H__
#define __HYPOTHESIS_H__

#include "observation.h"
#include "const.h"

typedef struct hypothesis {
  struct hypothesis* parent;
  struct hypothesis* children[5*PARTICLE_COUNT];
  double x;
  double y;
  double theta;
  observations *obs;
  struct map_node *map;
  int references;
  int child_count;
} hypothesis;

#include "map.h"

void hypothesis_reference(hypothesis*);
void hypothesis_dereference(hypothesis*);
void hypothesis_remove_child(hypothesis*, hypothesis*);
hypothesis* hypothesis_new(hypothesis*, double, double, double);
int hypothesis_tree_size(hypothesis*);

#endif
