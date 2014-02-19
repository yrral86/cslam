#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "landmark.h"

typedef struct particle {
  double p;
  double x;
  double y;
  double theta;
  double x_var;
  double y_var;
  double theta_var;
  landmark_tree_node *map;
} particle;

#include "swarm.h"

particle particle_sample_motion(particle, double, double, double);
particle particle_init(double, double, double);

#endif
