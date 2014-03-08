#include "particle.h"

particle particle_sample_motion(particle parent, double dx, double dy, double dtheta) {
  particle p;
  p.p = parent.p;
  p.x = parent.x + dx + rand_normal(parent.x_var);
  p.y = parent.y + dy + rand_normal(parent.y_var);
  p.theta = parent.theta + dtheta + rand_normal(parent.theta_var);
  p.x_var = parent.x_var;
  p.y_var = parent.y_var;
  p.theta_var = parent.theta_var;
  p.map = landmark_map_copy(parent.map);

  return p;
}

particle particle_init(double x, double y, double theta) {
  particle p;
  p.p = 1.0;
  p.x = x;
  p.y = y;
  p.theta = theta;
  // initial variance from swarm.h
  p.x_var = INITIAL_POSITION_VARIANCE;
  p.y_var = INITIAL_POSITION_VARIANCE;
  p.theta_var = INITIAL_ANGLE_VARIANCE;
  p.map = landmark_map_copy(NULL);
  return p;
}
