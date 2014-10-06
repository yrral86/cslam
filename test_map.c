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
  int width = 10000;
  int height = 10000;
  map_node *map;

  uint8_t *buffer = malloc((width + 1)*(height + 1));
  glutInit(&argc, argv);
  initGL(buffer, buffer, width + 1, height + 1, 700, 700);

  while (more_observations()) {
    map = map_new_from_observation(next_observation());
    map_write_buffer(map, buffer);
    map_deallocate(map);

    display();

    glutMainLoop();

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
