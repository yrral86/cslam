#include "sensor.h"

static tPort *port;
static int **buffer;
static raw_sensor_scan lidar_data;

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


raw_sensor_scan sensor_read_scan() {
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
      // Copy the data into the temp data set
      lidar_data.distances[i] = distance;
    } else {
      printf("out of range: %i\n", distance);
    }
  }
  return lidar_data;
}


