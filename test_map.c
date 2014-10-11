#include "map.h"
#include "lazygl.h"
#include "checkpoint.h"
#include "hypothesis.h"

static FILE *data;
static int initialized = 0;
static ssize_t read_status = 0;
static size_t length = 0;
static char *line = NULL;
static int parsed_line[RAW_SENSOR_DISTANCES_USB];

observations* next_observation();
int more_observations();

int main(int argc, char **argv) {
  int i, x, y, theta, last_x, last_y, last_theta, size, length;
  int width = MAP_SIZE + 1;
  int height = MAP_SIZE + 1;
  observations *obs;
  double information;
  map_node *map_all;
  map_node *map_current;
  checkpoint *cp = checkpoint_path_new();;
  checkpoint *path_end = checkpoint_path_new();

  uint8_t *buffer_current = malloc(width*height);
  uint8_t *buffer_all = malloc(width*height);
  glutInit(&argc, argv);
  initGL(buffer_all, buffer_all, width, height, 1200, 1200);
  map_all = map_new(width-1, height-1);

  swarm_init(RAW_SENSOR_DISTANCES_USB, SENSOR_RANGE_USB, width + 1, height + 1, width/2, 0);

  x = width/2;
  y = height/2;
  theta = 0;

  i = 0;
  obs = next_observation();
  // set up checkpoint
  cp->h.x = x;
  cp->h.y = y;
  cp->h.theta = theta;
  cp->h.obs = obs;
  // copy cp into new checkpoint after path
  path_end = checkpoint_path_append(path_end, cp);

  length = checkpoint_path_length(path_end);
  printf("Checkpoint #%d\n", length);

  // rewrite from checkpoints
  printf("Rewritting map_all from refined path checkpoints\n");
  map_deallocate(map_all);
  map_all = checkpoint_path_write_map(path_end);
  map_write_buffer(map_all, buffer_all);
  swarm_set_map(buffer_all);

  display();
  glutMainLoopEvent();

  while (more_observations()) {
    obs = next_observation();
    do {
      swarm_move(0, 0, 360);
      swarm_update(obs);
      x = swarm_get_best_x();
      y = swarm_get_best_y();
      theta = swarm_get_best_theta();
      printf("(%d, %d, %d)\n", x, y, theta);
      printf("converged: %i\n", swarm_converged());
    } while(swarm_converged() == 0);

    // set up checkpoint
    cp->h.x = x;
    cp->h.y = y;
    cp->h.theta = theta;
    cp->h.obs = obs;

    map_current = map_new_from_hypothesis(cp->h);
    map_write_buffer(map_current, buffer_current);
    map_deallocate(map_current);

    // copy cp into new checkpoint after path
    path_end = checkpoint_path_append(path_end, cp);

    length = checkpoint_path_length(path_end);
    printf("Checkpoint #%d\n", length);

    // rewrite from checkpoints
    printf("Rewritting map_all from refined path checkpoints\n");
    map_deallocate(map_all);
    map_all = checkpoint_path_write_map(path_end);
    map_write_buffer(map_all, buffer_all);
    swarm_set_map(buffer_all);

    /*
    last_x = x;
    last_y = y;
    last_theta = theta;

    cp->observation = map_new_from_observation(obs);
    map_merge(map_all, cp->observation, x, y, theta);
    printf("(%d, %d, %d)\n", x - last_x, y - last_y, theta - last_theta);
    //    printf("iteration\tinfo\tsize\tinfo/size\n");
    information = map_get_info(map_all);
    size = map_get_size(map_all);
    printf("%d\t%g\t%d\t%g\n", i, information, size, information/size);
    if (information > 1.1*cp->information || size > 1.1*cp->size) {

      // refine with ga
      if (0 && length >= 20 && length % 5 == 0) {
	printf("Running ga to refine path\n");
	printf("path_end: (%d,%d,%d)\n", path_end->x, path_end->y, path_end->theta);
	path_end = checkpoint_path_end(checkpoint_path_refine(path_end));
	cp->information = map_get_info(map_all);
	cp->size = map_get_size(map_all);
      }

      map_deallocate(map_all_with_path);
      map_all_with_path = checkpoint_path_write_map_with_path(path_end);

      map_write_buffer(cp->observation, buffer_current);
      map_write_buffer(map_all_with_path, buffer_all);

    } else
      map_deallocate(cp->observation);
    */

    display();
    glutMainLoopEvent();

    i++;
  }

  fclose(data);

  printf("Running ga to refine path\n");
  printf("path_end: (%d,%d,%d)\n", path_end->h.x, path_end->h.y, path_end->h.theta);
  path_end = checkpoint_path_end(checkpoint_path_refine(path_end));
  printf("Rewritting map_all from refined path checkpoints\n");
  map_deallocate(map_all);
  map_all = checkpoint_path_write_map(path_end);
  map_write_buffer(map_all, buffer_all);
  display();

  glutMainLoopEvent();

  checkpoint_path_deallocate(path_end);
  checkpoint_path_deallocate(cp);

  glutMainLoop();

  return 0;
}

observations* next_observation() {
  char *int_string;
  observations *obs = malloc(sizeof(observations));
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

  for (i = 0; i < RAW_SENSOR_DISTANCES_USB; i++) {
    obs->list[i].r = parsed_line[i];
    obs->list[i].theta = (-SENSOR_RANGE_USB/2.0 + i*SENSOR_SPACING_USB);
  }

  read_status = getline(&line, &length, data);

  return obs;
}

int more_observations() {
  return read_status != -1;
}
