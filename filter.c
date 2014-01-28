#include "filter.h"

int filter_particle(particle p, sensor_scan scan) {
  int filter = 0;
  sensor_scan particle_sensors = sensor_distance(p);
  int i;
  for (i = 0; i < SENSOR_DISTANCES; i++)
    if (particle_sensors.distances[i] < scan.distances[i])
      filter = 1;
  return filter;
}
