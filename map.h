#ifndef __MAP_H__
#define __MAP_H__

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include "landmark_types.h"

typedef struct map_node {
  uint8_t new;
  unsigned int x;
  unsigned int y;
  unsigned int x_min;
  unsigned int x_max;
  unsigned int y_min;
  unsigned int y_max;
  unsigned int out_of_bounds;
  landmark landmarks[4];
  landmark landmark;
  struct map_node* children[4];
  struct map_node* root;
} map_node;

#include "landmark.h"
#include "sensor.h"

map_node* map_new(int, int);
map_node* map_new_from_observation(int*);
map_node* map_node_new(int, int, int, int);
map_node* map_dup(map_node*);
void map_node_spawn_child(map_node*, int);
void map_merge(map_node*, map_node*, int, int, int);
int map_node_index_from_x_y(map_node*, int, int);
void map_node_ranges_from_index(map_node*, int, int*, int*, int*, int*);
//void map_node_split(map_node*,int);
void map_deallocate(map_node*);
void map_set_seen(map_node*, int, int);
void map_set_unseen(map_node*, int, int);
void map_increase_seen(map_node*, int, int, int);
void map_increase_unseen(map_node*, int, int, int);
double map_seen_probability(map_node*, int, int);
double map_unseen_probability(map_node*, int, int);
//void map_landmark_check_split(map_node*, int);
void map_write_buffer(map_node*, uint8_t*);
void map_node_write_buffer(map_node*, uint8_t*);
double map_get_info(map_node*);
int map_get_size(map_node*);
void map_debug(map_node*);

#endif
