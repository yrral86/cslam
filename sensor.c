#include "sensor.h"

const static int poll_time_eth = 25000;
const static int poll_time_usb = 100000;
static int poll_time;

static urg_t connection;
static tPort *connection_usb;
static long *buffer;
static int **buffer_usb;
static raw_sensor_scan lidar_data;
static raw_sensor_scan n_scans[MAX_SCANS];
static int last_poll, n, eth, usb;

pthread_t sensor_init_thread () {
  pthread_t t;
  pthread_create(&t, NULL, sensor_init, NULL);
  return t;
}

void* sensor_init(void *null_pointer) {
  int max_data_size;

  urg_connection_type_t m_type;
  urg_connection_type_t type_eth = URG_ETHERNET;
  urg_measurement_type_t m_type_eth = URG_MULTIECHO;
  char *device_eth = "192.168.0.10";
  char *device_usb = "/dev/ttyACM0";

  eth = 0;
  usb = 0;

  if (urg_open(&connection, type_eth, device_eth, 10940) < 0) {
    printf("Could not connect to the ethernet sensor\n");
  } else eth = 1;

  if (eth == 1) {
    poll_time = poll_time_eth;
    m_type = m_type_eth;
  }

  // dual sensors unsupported
  if (eth == 1)
      printf("Dual sensors unsupported, using ethernet\n");
  else {
    connection_usb = scipConnect(device_usb);
    if (connection_usb == NULL)   
      printf("Could not connect to the usb sensor\n");
    else usb = 1;
  }

  // must have a sensor
  assert(eth == 1 || usb == 1);

  if (usb == 1) {
    switchToScip2(connection_usb);
    scip2SetComSpeed(connection_usb, 115200);
    buffer_usb = NULL;
    poll_time = poll_time_usb;
  } else {
    // start measurement
    max_data_size = urg_max_data_size(&connection);
    buffer = malloc(sizeof(int)*max_data_size);
    urg_start_measurement(&connection, m_type, 0, 0);
  }

  last_poll = utime() - poll_time;

  return NULL;
}


raw_sensor_scan sensor_read_raw() {
  // Initialize parameters for laser scanning
  int start_step = 44;
  int end_step = 725;
  int step_cluster = 1;
  int scan_interval = 0;
  int scan_num = 1;
  int step_num;
  long timestamp;

  /*
  int sleep_time = poll_time - (utime() - last_poll);
  if (sleep_time > 0)
    usleep(sleep_time);
    else printf("sleepless for %g seconds\n", -sleep_time/1000000.0);

  last_poll = utime();
  */  

  if (eth)
    step_num = urg_get_distance(&connection, buffer, &timestamp);
  else
    buffer_usb = scip2MeasureScan(connection_usb, start_step, end_step, step_cluster,
				  scan_interval, scan_num, ENC_3BYTE, &step_num);

  int i, distance;
  for (i = 0 ; i < step_num ; i++) {
    if (eth)
      distance = buffer[i];
    else
      distance = buffer_usb[0][i];
    if ((distance >= SENSOR_MIN) && (distance <= SENSOR_MAX_USB))
      // Copy the data into the static variable
      lidar_data.distances[i] = distance;
  }

  if (usb) {
    // free usb_buffer, it is malloc'd every time we call scip2MeasureScan
    for (i = 0; i < scan_num; i++)
      free(buffer_usb[i]);
    free(buffer_usb);
  }

  return lidar_data;
}

void* sensor_read_raw_n(void *null_pointer) {
  int i;
  for (i = 0; i < n && i < MAX_SCANS; i++)
    n_scans[i] = sensor_read_raw();

  return NULL;
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
