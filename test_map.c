#include "map.h"
#include "lazygl.h"

static FILE *data;
static int initialized = 0;
static ssize_t read_status = 0;
static size_t length = 0;
static char *line = NULL;
static int parsed_line[RAW_SENSOR_DISTANCES_USB];

int* next_observation();
int more_observations();

int main(int argc, char **argv) {
  int x, y, theta;
  int width = 10000;
  int height = 10000;
  int *obs;
  map_node *map, *map_all;

  uint8_t *buffer_current = malloc((width + 1)*(height + 1));
  uint8_t *buffer_all = malloc((width + 1)*(height + 1));
  glutInit(&argc, argv);
  initGL(buffer_current, buffer_all, width + 1, height + 1, 400, 400);
  map_all = map_new_from_observation(next_observation());

  swarm_init(RAW_SENSOR_DISTANCES_USB, SENSOR_RANGE_USB, 10001, 10001, 5000, 0);

  swarm_set_map(map_all);

  while (more_observations()) {
    obs = next_observation();
    map = map_new_from_observation(obs);
    do {
      swarm_move(0, 0, 360);
      swarm_update(obs);
    } while(swarm_converged() == 0 && 0);
    x = swarm_get_best_x() - 5000;
    y = swarm_get_best_y() - 5000;
    theta = swarm_get_best_theta();
    map_merge(map_all, map, x, y, theta);
    printf("(%d, %d, %d)\n", x, y, theta);

    map_write_buffer(map, buffer_current);
    map_deallocate(map);
    map_write_buffer(map_all, buffer_all);

    display();

    glutMainLoopEvent();
  }

  fclose(data);

  return 0;
}

int* next_observation() {
  char *int_string;
  int i;

  if (initialized == 0) {
    data = fopen("sensor.csv", "r");
    assert(data != NULL);
    read_status = getline(&line, &length, data);
    initialized = 1;
  }

  int_string = strtok(line, ",");
  sscanf(int_string, "%d", parsed_line);
  for (i = 1; i < RAW_SENSOR_DISTANCES_USB - 1; i++) {
      int_string = strtok(NULL, ",");
      sscanf(int_string, "%d", parsed_line + i);
  }
  int_string = strtok(NULL, ",");
  sscanf(int_string, "%d\n", parsed_line + i);

  read_status = getline(&line, &length, data);

  return &(*parsed_line);
}

int more_observations() {
  return read_status != -1;
}
