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
  struct checkpoint *head;
} checkpoint;

checkpoint* checkpoint_new();
checkpoint* checkpoint_path_new();
checkpoint* checkpoint_path_append(checkpoint*,checkpoint*);
checkpoint* checkpoint_path_end(checkpoint*);
int checkpoint_path_length(checkpoint*);
map_node* checkpoint_path_write_map(checkpoint*);

#endif
