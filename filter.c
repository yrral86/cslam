#include "filter.h"

int filter_particle(particle p, sensor_scan scan) {
  int filter = 0;
  sensor_scan particle_sensors = sensor_distance(p);
  int i;
  int count = 0;
  for (i = 0; i < SENSOR_DISTANCES; i++)
    // filter out the particle if there is a known obstacle
    // that is closer than the scan detected
    if (particle_sensors.distances[i] < scan.distances[i])
      count++;

  if (count > 1)
    filter = 1;
  return filter;
}
