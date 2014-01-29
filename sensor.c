#include "sensor.h"

static tPort *port;
static int **buffer;
static raw_sensor_scan lidar_data;

int in_bounds(int x, int y) {
  return (x > 0 && x < ARENA_WIDTH && y > 0 && y < ARENA_HEIGHT && y + 2*x > 800);
}

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
  while (in_bounds(nx, ny)) {
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
    if (in_bounds(nx, ny))
      a = c;
    else
      b = c;
  }

  return a;
}

sensor_scan sensor_distance(particle p) {
  sensor_scan s;
  int i;
  for (i = 0; i < SENSOR_DISTANCES; i++)
    s.distances[i] = sensor_distance_offset(p, sensor_distance_index_to_radians(i));
  return s;
}

sensor_scan sensor_read() {
  sensor_scan s;
  raw_sensor_scan raw = sensor_read_scan();

  double spacing = (double)RAW_SENSOR_DISTANCES/SENSOR_DISTANCES;

  // downsample
  int i;
  for (i = 0; i < SENSOR_DISTANCES; i++)
    s.distances[i] = raw.distances[(int)(i*spacing)];
  return s;
}

double sensor_distance_index_to_degrees(int i) {
  // spread over 240 degrees, centered on 0
  return i*240/SENSOR_DISTANCES - 120;
}

double sensor_distance_index_to_radians(int i) {
  // 240 degrees = 2/3rds of 2*pi
  return i*4*M_PI/(3*SENSOR_DISTANCES) - M_PI/3;
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
      // Copy the data into the static variable
      lidar_data.distances[i] = distance;
    }
  }
  return lidar_data;
}


