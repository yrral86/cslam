#include "sensor.h"
#include <pthread.h>

int main(int argc, char **argv) {
  int i, j;
  pthread_t sensor_thread;
  raw_sensor_scan *scans;
  FILE *log;

  sensor_thread = sensor_init_thread();

  scans = malloc(sizeof(raw_sensor_scan));

  // wait for sensor
  assert(pthread_join(sensor_thread, NULL) == 0);

  // start a scan
  sensor_thread = sensor_read_raw_n_thread(1);

  // open log
  log = fopen("sensor.log", "w");

  i = 0;
  while (i < 1000) {
    // wait for sensor
    assert(pthread_join(sensor_thread, NULL) == 0);

    // get samples
    for (j = 0; j < 1; j++) {
      scans[j] = sensor_fetch_index(j);
    }

    for (j = 0; j < RAW_SENSOR_DISTANCES_USB - 1; j++)
      fprintf(log, "%d,", scans[0].distances[j]);
    fprintf(log, "%d\n", scans[0].distances[j]);

    // start a scan
    sensor_thread = sensor_read_raw_n_thread(1);

    i++;
  }

  fclose(log);

  // wait for sensor
  assert(pthread_join(sensor_thread, NULL) == 0);

  return 0;
}
