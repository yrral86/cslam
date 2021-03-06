#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "landmark_types.h"
#include <stdint.h>

typedef struct particle {
  double p;
  double x;
  double y;
  double theta;
  double x_var;
  double y_var;
  double theta_var;
  int resampled;
  struct hypothesis* h;
} particle;

#include "hypothesis.h"
#include "map.h"
#include "swarm.h"
#include "landmark.h"

particle particle_sample_motion(particle, int, int, int);
particle particle_sample_normal(particle, int);
particle particle_init(int, int, int);

#endif
