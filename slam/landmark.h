#ifndef __LANDMARK_H__
#define __LANDMARK_H__

#include <assert.h>
#include <stdint.h>

#include "const.h"
#include "particle.h"

landmark_map_node* landmark_map_copy(landmark_map_node*);
void landmark_map_node_dereference(landmark_map_node*);
landmark_map_node* landmark_build_subtree(int, int);
void landmark_set_seen(landmark_map_node*, int);
void landmark_set_seen_value(landmark_map_node*, int, int);
void landmark_set_unseen(landmark_map_node*, int);
void landmark_set_unseen_value(landmark_map_node*, int, int);
void landmark_write_map(landmark_map_node*, uint8_t*);
void landmark_write_map_subtree(landmark_map_node*, uint8_t*);
double landmark_seen_probability(landmark_map_node*, int);
double landmark_unseen_probability(landmark_map_node*, int);
int landmark_map_node_find_distance(landmark_map_node*, int, particle);
raw_sensor_scan landmark_map_simulate_scan(particle);

#endif
