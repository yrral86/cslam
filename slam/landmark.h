#ifndef __LANDMARK_H__
#define __LANDMARK_H__

#include <assert.h>
#include <stdint.h>

#include "const.h"
#include "particle.h"

landmark_tree_node* landmark_tree_copy(landmark_tree_node*);
void landmark_tree_node_dereference(landmark_tree_node*);
landmark_tree_node* landmark_build_subtree(int, int);
void landmark_set_seen(landmark_tree_node*, int);
void landmark_set_seen_value(landmark_tree_node*, int, int);
void landmark_set_unseen(landmark_tree_node*, int);
void landmark_set_unseen_value(landmark_tree_node*, int, int);
void landmark_write_map(landmark_tree_node*, uint8_t*);
void landmark_write_map_subtree(landmark_tree_node*, uint8_t*);
double landmark_seen_probability(landmark_tree_node*, int);
double landmark_unseen_probability(landmark_tree_node*, int);
int landmark_tree_node_find_distance(landmark_tree_node*, int, particle);
raw_sensor_scan landmark_tree_simulate_scan(particle);

#endif
