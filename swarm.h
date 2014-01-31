#ifndef __SWARM_H__
#define __SWARM_H__

#include "particle.h"
#include "simulation.h"
#include "filter.h"
#include "random.h"

typedef struct swarm_member {
  particle p;
} swarm_member;

static const int SWARM_TOP_COUNT = 25;

void swarm_init();
void swarm_reset();
int swarm_get_size();
void swarm_add_particle(particle);
void swarm_bubble_up(int);
static int swarm_parent(int);
static int swarm_left_child(int);
static int swarm_right_child(int);
void swarm_move_particle(particle, double, double, double);
int swarm_find_particle(particle);
void swarm_particle_update_score(particle, double);
void swarm_all_particles(particle[MAX_PARTICLES]);
void swarm_top_particles(particle[SWARM_TOP_COUNT]);
particle swarm_pop_particle();
void swarm_filter_particles(sensor_scan);
void swarm_delete_particle(particle);
particle swarm_get_random_particle();

#endif
