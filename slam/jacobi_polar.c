#include "jacobi_polar.h"

void jacobian_polar(particle p) {
  int i, d;
  // TODO: _ETH
  for (i = 0; i < RAW_SENSOR_DISTANCES_USB; i++) {
    d = landmark_map_find_distance(p.map, i, p);
  }
}
