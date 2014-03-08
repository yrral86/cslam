#include "jacobi_polar"

void jacobian_polar(particle p) {
  int i, d;
  // TODO: _ETH
  for (i = 0; i < RAW_SENSOR_DISTANCES_USB; i++) {
    d = landmark_map_node_find_distance(p.map, i);
  }
}
