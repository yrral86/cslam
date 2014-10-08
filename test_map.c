#include "map.h"
#include "lazygl.h"
#include "checkpoint.h"

static FILE *data;
static int initialized = 0;
static ssize_t read_status = 0;
static size_t length = 0;
static char *line = NULL;
static int parsed_line[RAW_SENSOR_DISTANCES_USB];

int* next_observation();
int more_observations();

int main(int argc, char **argv) {
  int i, j, x, y, theta, last_x, last_y, last_theta, size;
  int width = 20000;
  int height = 20000;
  int *obs;
  double information;
  map_node *map_all;
  checkpoint *cp;
  checkpoint *path = malloc(sizeof(checkpoint));
  path->next = NULL;
  path->previous = NULL;
  path->information = 0;
  path->size = 0;

  uint8_t *buffer_current = malloc((width + 1)*(height + 1));
  uint8_t *buffer_all = malloc((width + 1)*(height + 1));
  glutInit(&argc, argv);
  initGL(buffer_current, buffer_all, width + 1, height + 1, 400, 400);
  map_all = map_new(width, height);

  swarm_init(RAW_SENSOR_DISTANCES_USB, SENSOR_RANGE_USB, width + 1, height + 1, width/2, 0);

  swarm_set_map(map_all);

  i = 0;
  while (more_observations()) {
    obs = next_observation();
    path->observation = map_new_from_observation(obs);
    do {
      swarm_move(0, 0, 360);
      swarm_update(obs);
    } while(swarm_converged() == 0);
    last_x = x;
    last_y = y;
    last_theta = theta;
    x = swarm_get_best_x();
    y = swarm_get_best_y();
    theta = swarm_get_best_theta();
    map_merge(map_all, path->observation, x, y, theta);
    printf("(%d, %d, %d)\n", x, y, theta);
    printf("(%d, %d, %d)\n", x - last_x, y - last_y, theta - last_theta);
    //    printf("iteration\tinfo\tsize\tinfo/size\n");
    information = map_get_info(map_all);
    size = map_get_size(map_all);
    printf("%d\t%g\t%d\t%g\n", i, information, size, information/size);
    map_write_buffer(path->observation, buffer_current);
    if (information < 1.1*path->information && size < 1.1*path->size)
      map_deallocate(path->observation);
    else {
      path->x = x;
      path->y = y;
      path->theta = theta;
      path->information = information;
      path->size = size;
      path->next = malloc(sizeof(checkpoint));
      path->next->previous = path;
      path = path->next;
      path->next = NULL;
      path->information = information;
      path->size = size;

      j = 0;
      cp = path;
      while(cp->previous != NULL) {
	cp = cp->previous;
	j++;
      }
      printf("Checkpoint #%d\n", j);
    }
    map_write_buffer(map_all, buffer_all);

    display();

    glutMainLoopEvent();

    i++;
  }

  fclose(data);

  glutMainLoop();

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
