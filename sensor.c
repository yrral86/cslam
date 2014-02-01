#include "sensor.h"

static tPort *port;
static int **buffer;
static raw_sensor_scan lidar_data;

void sensor_init () {
  port = scipConnect("/dev/ttyACM0");
  if (port == NULL) {
    perror("Could not connect to the sensor ");
    exit(EXIT_FAILURE);
  }

  // use SCIP2.0
  switchToScip2(port);
  if(scip2SetComSpeed(port,115200)!=0){
    fprintf(stderr,"Could not change speed\n");
    exit(EXIT_FAILURE);
  }
}


raw_sensor_scan sensor_read_raw() {
  // Initialize parameters for laser scanning
  int start_step = 44;
  int end_step = 725;
  int step_cluster = 1;
  int scan_interval = 0;
  int scan_num = 1;
  int step_num;

  buffer = scip2MeasureScan(port, start_step, end_step, step_cluster,
			    scan_interval, scan_num, ENC_3BYTE, &step_num);

  int i, distance;
  for (i = 0 ; i < step_num ; i++) {
    distance = buffer[0][i];
    if ((distance >= SENSOR_MIN) && (distance <= SENSOR_MAX)) {
      // Copy the data into the static variable
      lidar_data.distances[i] = distance;
    }
  }
  return lidar_data;
}


