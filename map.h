#ifndef __MAP_H__
#define __MAP_H__

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include "landmark_types.h"
#include "hypothesis.h"

//#define __MAP_TYPE_TREE__
#define __MAP_TYPE_HEAP__

typedef struct map_node {
#ifdef __MAP_TYPE_TREE__
  uint8_t new;
  unsigned int x_min;
  unsigned int x_max;
  unsigned int y_min;
  unsigned int y_max;
  unsigned int out_of_bounds;
  landmark landmarks[4];
  struct map_node* children[4];
  struct map_node* root;
  unsigned int x;
  unsigned int y;
  landmark landmark;
#endif
#ifdef __MAP_TYPE_HEAP__
  unsigned int max_size;
  unsigned int current_size;
  unsigned int heap_sorted;
  unsigned int index;
  struct map_pixel *heap;
#endif
} map_node;

#ifdef __MAP_TYPE_HEAP__
typedef struct map_pixel {
  unsigned int x;
  unsigned int y;
  landmark l;
  hypothesis *h;
  unsigned int obs_index;
} map_pixel;

map_node* map_new_from_hypothesis(hypothesis);
void map_generate_mask(int);
map_node* map_get_shifted_mask(int, int);
void map_add_pixel(map_node*, map_pixel);
void map_double_max_size(map_node*);
void map_reheapify_up(map_node*);
void map_reheapify_down(map_node*);
void map_reheapify_down_root(map_node*, int);
map_node* map_intersection(map_node*, map_node*);
map_node* map_from_mask_and_hypothesis(map_node*, hypothesis*);
map_node* map_merge(map_node*, map_node*);
inline int map_pixel_need_swap(map_pixel, map_pixel);
inline int map_parent_index(int);
inline int map_left_index(int);
inline int map_right_index(int);
map_pixel map_pop_pixel(map_node*);
double map_merge_variance(map_node*, map_node*);
map_node* map_sort(map_node*);
  //map_node* map_merge_hypothesis(map_node*, hypothesis);
double map_variance(map_node*);

#endif

#include "hypothesis.h"
#include "landmark.h"
#include "sensor.h"

#ifdef __MAP_TYPE_TREE
map_node* map_new_from_observation(int*);
map_node* map_node_new(int, int, int, int);
void map_node_spawn_child(map_node*, int);
void map_merge(map_node*, map_node*, int, int, int);
void map_merge_aligned(map_node*, map_node*);
int map_node_index_from_x_y(map_node*, int, int);
void map_node_ranges_from_index(map_node*, int, int*, int*, int*, int*);
void map_set_seen(map_node*, int, int);
void map_set_unseen(map_node*, int, int);
void map_increase_seen(map_node*, int, int, int);
void map_increase_unseen(map_node*, int, int, int);
double map_seen_probability(map_node*, int, int);
double map_unseen_probability(map_node*, int, int);
//void map_node_split(map_node*,int);
//void map_landmark_check_split(map_node*, int);
void map_node_write_buffer(map_node*, uint8_t*);
double map_get_info(map_node*);
int map_get_size(map_node*);
#endif

map_node* map_new(int, int);
map_node* map_dup(map_node*);
void map_deallocate(map_node*);
void map_write_buffer(map_node*, uint8_t*);
void map_debug(map_node*);

#endif
