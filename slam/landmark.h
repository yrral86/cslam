#ifndef __LANDMARK_H__
#define __LANDMARK_H__

#include <assert.h>
#include <stdint.h>

#include "const.h"
#include "particle.h"

landmark_map* landmark_map_copy(landmark_map*);
void landmark_map_free(landmark_map*);
void landmark_map_reference(landmark_map*);
void landmark_map_dereference(landmark_map*);
landmark_map* landmark_map_init(int);
void landmark_set_seen(landmark_map*, int);
void landmark_set_seen_value(landmark_map*, int, int);
void landmark_set_unseen(landmark_map*, int);
void landmark_set_unseen_value(landmark_map*, int, int);
void landmark_write_map(landmark_map*, uint8_t*);
void landmark_write_map_subtree(landmark_map*, uint8_t*);
double landmark_seen_probability(landmark_map*, int);
double landmark_unseen_probability(landmark_map*, int);
int landmark_map_find_distance(landmark_map*, int, particle);
void landmark_map_simulate_scan(particle, int*, int);

#endif
