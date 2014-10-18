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
  int i, length;//, size, old_size;
  int width = MAP_SIZE + 1;
  int height = MAP_SIZE + 1;
  observations *obs;
  double x, y, theta;
  //  double last_x, last_y, last_theta;
  //  map_node *map_all;
  map_node *h_map, *mask_map;
  hypothesis *latest_h, *root_h;
  checkpoint *path_end = checkpoint_path_new();

  uint8_t *buffer_tmp;
  uint8_t *buffer_all = malloc(width*height/(BUFFER_FACTOR*BUFFER_FACTOR));

  glutInit(&argc, argv);
  initGL(buffer_all, buffer_all, width/BUFFER_FACTOR, height/BUFFER_FACTOR, 1200, 1200);

  x = width/2;
  y = height/2;
  theta = 0;

  //  needed to set up width and height for maps
  //  map_all = map_new(width, height, width/2, height/2);
  //  map_dereference(map_all);

  // generate presorted mask for O(a) copies
  map_generate_mask(SENSOR_MAX_USB);

  i = 0;
  obs = next_observation();
  // set up initial hypothesis
  root_h = hypothesis_new(NULL, x, y, theta);
  root_h->obs = obs;
  // get mask
  mask_map = map_get_shifted_mask(root_h->x, root_h->y);

  // generate map
  root_h->map = map_from_mask_and_hypothesis(mask_map, root_h);
  map_write_buffer(root_h->map);

  // copy cp into new checkpoint after path
  path_end = checkpoint_path_append(path_end, root_h);

  // leave root_h referenced so we can trace ancestry and prune
  //  TODO: trace ancestry and prune

  length = checkpoint_path_length(path_end);
  printf("Checkpoint #%d\n", length);

  // init swarm
  swarm_set_initial_hypothesis(root_h);
  swarm_init(RAW_SENSOR_DISTANCES_USB, SENSOR_RANGE_USB, width, height, width/2, 0);
  //  swarm_set_map(buffer_all);

  printf("calling display\n");
  display(root_h->map);
  printf("looping\n");

  glutMainLoopEvent();

  while (more_observations() && i < 100) {
    printf("iteration: %i\n", i);
    obs = next_observation();
    //    swarm_reset_convergence();
    //    do {
    swarm_move(0, 0, 0);
    swarm_update(obs);
      //      printf("converged: %i\n", swarm_converged());
      //    } while(swarm_converged() == 0);

    // set up checkpoint
    latest_h = obs->hypotheses;
    printf("(%g, %g, %g)\n", latest_h->x, latest_h->y, latest_h->theta);
    /*    old_size = size;
    size = buffer_hypothesis_distance(buffer_all, latest_h, 0, 10);

    printf("size: %d change: %d\n", size, size - old_size);
    */
 //   map_current = map_new_from_hypothesis(latest_h);
//    map_write_buffer(map_current, buffer_current);
  //  map_deallocate(map_current);

    // copy cp into new checkpoint after path if more than
    // 10 cm or 3 degrees
    // checkpoint based on size <= 12
    //    if ((latest_h.x-last_x)*(latest_h.x-last_x) +
    //	(latest_h.y-last_y)*(latest_h.y-last_y) > 10000 ||
    //	abs(latest_h.theta-last_theta) > 3) {
    //    if (size == 0) {

      path_end = checkpoint_path_append(path_end, latest_h);

      length = checkpoint_path_length(path_end);
      printf("Checkpoint #%d\n", length);
      /*
      printf("adding best hypothesis to map\n");
      //    map_deallocate(map_all);
      mask_map = map_get_shifted_mask(latest_h->x, latest_h->y);
      h_map = map_from_mask_and_hypothesis(mask_map, latest_h);
      map_deallocate(mask_map);
      map_all = map_merge(map_all, h_map);
      // map merge frees previous map_all and h_map
      printf("merged, writing buffer\n");
      */
      printf("showing buffer from best hypothesis local map\n");
      display(latest_h->map);
      //    }

    //    swarm_set_map(buffer_all);

    // rewrite from checkpoints
    //  printf("Rewritting map_all from refined path checkpoints\n");
    //  map_deallocate(map_all);
    //  map_all = checkpoint_path_write_map(path_end);
    //  map_write_buffer(map_all, buffer_all);
    //    swarm_set_map(buffer_all);
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

    glutMainLoopEvent();

    i++;
  }

  fclose(data);
  /*
  printf("Running ga to refine path\n");
  printf("path_end: (%g,%g,%g)\n", path_end->h->x, path_end->h->y, path_end->h->theta);
  path_end = checkpoint_path_end(checkpoint_path_refine(path_end));
  printf("Rewritting map_all from refined path checkpoints\n");
  map_deallocate(map_all);
  map_all = checkpoint_path_write_map(path_end);
  map_write_buffer(map_all, buffer_all);
  display();

  glutMainLoopEvent();
  */
  checkpoint_path_deallocate(path_end);
  map_dereference_mask();

  //  glutMainLoop();

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
