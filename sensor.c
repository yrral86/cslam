#include "sensor.h"

int sensor_distance_offset(particle p, double offset) {
  int a, b, c;
  double dx, dy, nx, ny;
  a = 0;
  b = 10;
  double t = p.theta*M_PI/180 + offset;
  dx = b*cos(t);
  dy = b*sin(t);
  nx = p.x + dx;
  ny = p.y + dy;

  // bracket between a and b
  while (nx > 0 && nx < ARENA_WIDTH && ny > 0 && ny < ARENA_HEIGHT) {
    a = b;
    b += 10;
    dx = b*cos(t);
    dy = b*sin(t);
    nx = p.x + dx;
    ny = p.y + dy;
  }

  // binary search
  while (a - b > 0) {
    c = (a+b)/2;
    dx = c*cos(t);
    dy = c*sin(t);
    nx = p.x + dx;
    ny = p.y + dy;
    if (nx > 0 && nx < ARENA_WIDTH && ny > 0 && ny < ARENA_HEIGHT)
      a = c;
    else
      b = c;
  }

  return a;
}

sensor_scan sensor_distance(particle p) {
  sensor_scan s;
  double normalization_angle = p.theta*M_PI/180;
  int i;
  for (i = 0; i < SENSOR_DISTANCES; i++)
    s.distances[i] = sensor_distance_offset(p, sensor_distance_index_to_radians(i) -
					    normalization_angle);
  return s;
}

double sensor_distance_index_to_degrees(int i) {
  return i*360/SENSOR_DISTANCES;
}

double sensor_distance_index_to_radians(int i) {
  return i*2*M_PI/SENSOR_DISTANCES;
}
