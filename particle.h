#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "landmark_types.h"

typedef struct particle {
  double p;
  double x;
  double y;
  double theta;
  double x_var;
  double y_var;
  double theta_var;
  landmark_map *map;
} particle;

#include "swarm.h"
#include "landmark.h"

particle particle_sample_motion(particle, int, int, int);
particle particle_sample_normal(particle, int);
particle particle_init(int, int, int);

#endif
