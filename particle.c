#include "particle.h"

particle particle_sample_motion(particle parent, int dx, int dy, int dtheta) {
  particle p = particle_sample_normal(parent, 0);
  p.x += dx;
  p.y += dy;
  p.theta += dtheta;
  return p;
}

particle particle_sample_normal(particle parent, int iterations) {
  particle p;
  p.p = parent.p;
  p.x = parent.x + rand_normal(parent.x_var);
  p.y = parent.y + rand_normal(parent.y_var);
  p.theta = parent.theta + rand_normal(parent.theta_var);
  /*  if (iterations == 0) {
    p.x_var = INITIAL_POSITION_VARIANCE;
    p.y_var = INITIAL_POSITION_VARIANCE;
    p.theta_var = INITIAL_ANGLE_VARIANCE;
  } else if (parent.x_var > BUFFER_FACTOR) {
    p.x_var = 0.8*parent.x_var;
    p.y_var = 0.8*parent.y_var;
    p.theta_var = 0.8*parent.theta_var;
    } else {*/
    p.x_var = parent.x_var;
    p.y_var = parent.y_var;
    p.theta_var = parent.theta_var;
    p.h = parent.h;
    p.resampled = 0;
  // }
  //  p.map = landmark_map_copy(parent.map);

  return p;
}

particle particle_init(int x, int y, int theta) {
  particle p;
  p.p = 1;
  p.x = x;
  p.y = y;
  p.theta = theta;
  // initial variance from const.h
  p.x_var = INITIAL_POSITION_VARIANCE;
  p.y_var = INITIAL_POSITION_VARIANCE;
  p.theta_var = INITIAL_ANGLE_VARIANCE;
  p.resampled = 0;
  p.h = NULL;

  return p;
}
