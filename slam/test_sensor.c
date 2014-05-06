#include "urg_driver/urg_sensor.h"

int main(int argc, char **argv) {
  urg_t connection;
  urg_connection_type_t type = URG_ETHERNET;
  char *device = "192.168.0.10";
  int direction_count, max_data_size, i;
  long timestamp;
  long *data;

  // open connection
  if (urg_open(&connection, type, device, 10940) < 0)
    return 1;

  max_data_size = urg_max_data_size(&connection);
  data = malloc(sizeof(long)*max_data_size);

  // start measurement
  // (connection, type, scan times (0 is keep going), skip)
  urg_start_measurement(&connection, type, 0, 0);

  while (1) {
    direction_count = urg_get_distance(&connection, data, &timestamp);
    for (i = 0; i < max_data_size; i++) {
      printf("%i ", data[i]);
    }
    printf("\nmax_data_size: %i\n", max_data_size);
    sleep(1);
  }

  return 0;
}
