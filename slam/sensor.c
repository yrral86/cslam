#include "sensor.h"

const static int poll_time_eth = 25000;
const static int poll_time_usb = 100000;

static urg_t connection_eth;
static urg_t connection_usb;
static long *buffer_eth;
static long *buffer_usb;
static raw_sensor_scan lidar_data;
static raw_sensor_scan n_scans[MAX_SCANS];
static int last_poll_usb, last_poll_eth, n, eth, usb;

pthread_t sensor_init_thread () {
  pthread_t t;
  pthread_create(&t, NULL, sensor_init, NULL);
  return t;
}

void* sensor_init(void *null_pointer) {
  int max_data_size;

  urg_connection_type_t type_eth = URG_ETHERNET;
  char *device_eth = "192.168.0.10";

  urg_connection_type_t type_usb = URG_SERIAL;
  char *device_usb = "/dev/ttyACM0";

  eth = 0;
  usb = 0;

  if (urg_open(&connection_eth, type_eth, device_eth, 10940) < 0) {
    printf("Could not connect to the ethernet sensor\n");
  } else eth = 1;

  if (urg_open(&connection_usb, type_usb, device_usb, 19200) < 0) {
    printf("Could not connect to the usb sensor\n");
  } else usb = 1;

  // start measurement
  if (eth) {
    max_data_size = urg_max_data_size(&connection_eth);
    buffer_eth = malloc(sizeof(int)*max_data_size);
    urg_start_measurement(&connection_eth, type_eth, 0, 0);
    last_poll_eth = utime() - poll_time_eth;
  }

  if (usb) {
    max_data_size = urg_max_data_size(&connection_usb);
    buffer_usb = malloc(sizeof(int)*RAW_SENSOR_DISTANCES_USB);
    urg_start_measurement(&connection_usb, type_usb, 0, 0);
    last_poll_usb = utime() - poll_time_usb;
  }
}


raw_sensor_scan sensor_read_raw() {
  // Initialize parameters for laser scanning
  int step_num;
  long timestamp;

  int sleep_time = poll_time_usb - (utime() - last_poll_usb);
  if (sleep_time > 0)
    usleep(sleep_time);
  else printf("sleepless\n");

  if (usb)
    step_num = urg_get_distance(&connection_usb, buffer_usb, &timestamp);
  
  last_poll_usb = utime();

  int i, distance;
  for (i = 0 ; i < step_num ; i++) {
    distance = buffer_usb[i];
    if ((distance >= SENSOR_MIN) && (distance <= SENSOR_MAX_USB))
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
