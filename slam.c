#include "slam.h"

#include "map.h"
#include "lazygl.h"

// 3, one for current, one for history, one for display
#define BUFFER_HISTORY 3
//static uint8_t* map[BUFFER_HISTORY];
static uint8_t* buffer;
static map_node *map;

static raw_sensor_scan *scans;
static particle current_particle;
//static int arena_width = 14940;
//static int arena_height = 6400;
//static int arena_width = 3740;
//static int arena_height = 1600;
static int width = 10000;
static int height = 10000;

int main (int argc, char **argv) {
  int i, j, iterations;
  pthread_t sensor_thread;
  // sample 3 times (0.3 sec)
  int sample_count = 1;
  double c, s, theta;
  int x, y, d;

  x = width/2;
  y = height/2;

  // connect to sensor in new thread while we
  // initialize everything else
  sensor_thread = sensor_init_thread();

  scans = malloc(sample_count*sizeof(raw_sensor_scan));

  // swarm_init will set up buffer sizing
  //  swarm_init(RAW_SENSOR_DISTANCES_USB, SENSOR_RANGE_USB, 7380, 3880, 1500);
  //  swarm_init(RAW_SENSOR_DISTANCES_USB, SENSOR_RANGE_USB, arena_width, arena_height, 2000, 0);

  // allocate buffers
  //  for (i = 0; i < BUFFER_HISTORY; i++)
  // map[i] = buffer_allocate();

  buffer = malloc((width + 1)*(height + 1));

  glutInit(&argc, argv);
  // pass size of buffer, then window size
  initGL(buffer, buffer, width + 1, height + 1, (width+1)/32, (height+1)/32);

  map = map_new(width, height);

  // wait for sensor
  assert(pthread_join(sensor_thread, NULL) == 0);

  // start a scan
  sensor_thread = sensor_read_raw_n_thread(sample_count);

  iterations = 0;

  while(1) {
      /*
    do {
      swarm_move(0, 0, 360);
      */

      // wait for sensor
      assert(pthread_join(sensor_thread, NULL) == 0);

      // get samples
      for (j = 0; j < sample_count; j++) {
	scans[j] = sensor_fetch_index(j);
      }

      // start a scan
      sensor_thread = sensor_read_raw_n_thread(sample_count);

      for (i = 0; i < RAW_SENSOR_DISTANCES_USB; i++) {
	theta = (-SENSOR_RANGE_USB/2.0 + i*SENSOR_SPACING_USB)*M_PI/180;
	c = cos(theta);
	s = sin(theta);
	d = scans[0].distances[i];
	//printf("i: %i d: %i\n", i, d);
	map_set_seen(map, x + d*c, height - y + d*s);
	for (j = 5; j < d; j += 5)
	  map_set_unseen(map, x + (d-j)*c, height - y + (d-j)*s);
      }

      map_write_buffer(map, buffer);

    /*
      if (iterations == 0) {
	// map initial location 10 times
	for (i = 0; i < 10; i++) {
	  // record each scan with the certainty of 50 observations
	  for (j = 0; j < 50*sample_count; j++)
	    swarm_map(scans[j % sample_count].distances);

	  // wait for sensor
	  assert(pthread_join(sensor_thread, NULL) == 0);

	  // get samples
	  for (j = 0; j < sample_count; j++) {
	    scans[j] = sensor_fetch_index(j);
	  }

	  // start a scan
	  sensor_thread = sensor_read_raw_n_thread(sample_count);
	}
	printf("initial scan completed\n");
      }

      // give the swarm the new scans and the historical map
      swarm_update(scans[0].distances);
    } while (swarm_converged() == 0);

    printf("converged, mapping\n");
    // update map
    for (j = 0; j < sample_count; j++)
      swarm_map(scans[j].distances);
    // update current map
    swarm_map_reset_current();
    for (j = 0; j < sample_count; j++)
      swarm_map_current(scans[j].distances);

    // update localization
    current_particle.x = swarm_get_best_x();
    current_particle.y = swarm_get_best_y();
    current_particle.theta = swarm_get_best_theta();

    // copy best map to buffer
    swarm_get_best_buffer(map[0]);
    swarm_get_map_buffer(map[2]);

    // draw position
    double s, c, t;
    t = current_particle.theta*M_PI/180;
    s = sin(t);
    c = cos(t);
    for (i = -5; i < 6; i++)
      for (j = -5; j < 6; j++ ) {
	record_map_position(0, current_particle.x + i*c - j*s,
			    current_particle.y + i*s + j*c, 255);
	record_map_position(2, current_particle.x + i*c - j*s,
			    current_particle.y + i*s + j*c, 255);
      }
*/
    // draw
    display();
    /*
    // clear position
    for (i = -5; i < 6; i++)
      for (j = -5; j < 6; j++ ) {
	record_map_position(0, current_particle.x + i*c - j*s,
			    current_particle.y + i*s + j*c, 0);
	if (!x_y_protected(current_particle.x + i*c - j*s, current_particle.y + i*s + j*c))
	  record_map_position(2, current_particle.x + i*c - j*s,
			      current_particle.y + i*s + j*c, 0);
      }
*/
    glutMainLoopEvent();

    if (iterations % 100 == 0)
      printf("map size: %i\n", map_get_size(map));

    iterations++;
  }

  return 0;
}

/*
void record_map_position(int index, int x, int y, uint8_t value) {
  if (in_arena(x, y))
    map[index][buffer_index_from_x_y(x, y)] = value;
}
*/
