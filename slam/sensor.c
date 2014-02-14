#include "sensor.h"

const static int eth_poll_time = 25000;
const static int usb_poll_time = 100000;

static urg_t eth_connection;
static urg_t usb_connection;
static long *buffer;
static raw_sensor_scan lidar_data;
static raw_sensor_scan n_scans[MAX_SCANS];
static int last_poll, n;

pthread_t sensor_init_thread () {
  pthread_t t;
  pthread_create(&t, NULL, sensor_init, NULL);
  return t;
}

void* sensor_init(void *null_pointer) {
  urg_connection_type_t type = URG_ETHERNET;
  char *device = "192.168.0.10";
  if (urg_open(&eth_connection, type, device, 10940) < 0) {
    printf("Could not connect to the sensor\n");
    exit(-1);
  }

  //  max_data_size = urg_max_data_size(&eth_connection);
  buffer = malloc(sizeof(int)*RAW_SENSOR_DISTANCES);

  // start measurement
  // (connection, type, scan times (0 is keep going), skip)
  urg_start_measurement(&eth_connection, type, 0, 0);

  last_poll = utime() - eth_poll_time;
}


raw_sensor_scan sensor_read_raw() {
  // Initialize parameters for laser scanning
  int step_num;
  long timestamp;

  int sleep_time = eth_poll_time - (utime() - last_poll);
  if (sleep_time > 0)
    usleep(sleep_time);
  else printf("sleepless\n");

  step_num = urg_get_distance(&eth_connection, buffer, &timestamp);
  
  last_poll = utime();

  int i, distance;
  for (i = 0 ; i < step_num ; i++) {
    distance = buffer[i];
    if ((distance >= SENSOR_MIN) && (distance <= SENSOR_MAX))
      // Copy the data into the static variable
      lidar_data.distances[i] = distance;
  }
  return lidar_data;
}

void* sensor_read_raw_n(void *null_pointer) {
  int i;
  for (i = 0; i < n && i < MAX_SCANS; i++)
    n_scans[i] = sensor_read_raw();
}

pthread_t sensor_read_raw_n_thread(int requested_n) {
  pthread_t t;
  n = requested_n;
  assert(pthread_create(&t, NULL, sensor_read_raw_n, NULL) == 0);
  return t;
}

raw_sensor_scan sensor_fetch_index(int index) {
  return n_scans[index];
}

uint64_t utime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}
