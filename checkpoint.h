#ifndef __CHECKPOINT_H__
#define __CHECKPOINT_H__

#include "map.h"
#include "random.h"

typedef struct checkpoint {
  unsigned int x;
  unsigned int y;
  int theta;
  double information;
  unsigned int size;
  map_node *observation;
  struct checkpoint *next;
  struct checkpoint *previous;
  struct checkpoint *head;
} checkpoint;

checkpoint* checkpoint_new();
checkpoint* checkpoint_path_new();
checkpoint* checkpoint_path_append(checkpoint*,checkpoint*);
checkpoint* checkpoint_path_end(checkpoint*);
int checkpoint_path_length(checkpoint*);
map_node* checkpoint_path_write_map(checkpoint*);
void checkpoint_path_deallocate(checkpoint*);
checkpoint* checkpoint_path_dup_with_deltas(checkpoint*, int*);
checkpoint* checkpoint_path_refine(checkpoint*);
void checkpoint_path_refine_crossover(int*, int, int, int, int);
void checkpoint_path_refine_mutate(int*, int);
void checkpoint_path_debug(checkpoint*);

#endif
