#include "filter.h"

int filter_particle_forward(particle p, int distance) {
  int filter = 0;
  if (sensor_distance_forward(p) < distance)
    filter = 1;
  return filter;
}

int filter_particle_reverse(particle p, int distance) {
  int filter = 0;
  if (sensor_distance_reverse(p) < distance)
    filter = 1;
  return filter;
}

int filter_particle_left(particle p, int distance) {
  int filter = 0;
  if (sensor_distance_left(p) < distance)
    filter = 1;
  return filter;
}

int filter_particle_right(particle p, int distance) {
  int filter = 0;
  if (sensor_distance_right(p) < distance)
    filter = 1;
  return filter;
}
