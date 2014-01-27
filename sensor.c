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
  while (nx > 0 && nx < 738 && ny > 0 && ny < 738) {
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
    dx = b*cos(t);
    dy = b*sin(t);
    nx = p.x + dx;
    ny = p.y + dy;
    if (nx > 0 && nx < 738 && ny > 0 && ny < 738)
      a = c;
    else
      b = c;
  }

  return a;
}

int sensor_distance_forward(particle p) {
  return sensor_distance_offset(p, 0);
}

int sensor_distance_reverse(particle p) {
  return sensor_distance_offset(p, M_PI);
}

int sensor_distance_left(particle p) {
  return sensor_distance_offset(p, -M_PI/2);
}

int sensor_distance_right(particle p) {
  return sensor_distance_offset(p, M_PI/2);
}
