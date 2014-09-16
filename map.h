#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include "landmark_types.h"

typedef struct map_node {
  unsigned int x_min;
  unsigned int x_max;
  unsigned int y_min;
  unsigned int y_max;
  landmark landmarks[4];
  landmark landmark;
  struct map_node* children[4];
} map_node;

map_node* map_new(int, int);
map_node* map_node_new(int, int, int, int);
int map_node_index_from_x_y(map_node*, int, int);
void map_node_ranges_from_index(map_node*, int, int*, int*, int*, int*);
void map_node_split(map_node*,int);
void map_deallocate(map_node*);
void map_set_seen(map_node*, int, int);
void map_set_unseen(map_node*, int, int);
void map_landmark_check_split(map_node*, int);
void map_write_buffer(map_node*, uint8_t*);
void map_node_write_buffer(map_node*, uint8_t*);

#endif
