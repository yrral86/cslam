#include "slam.h"

#include "lazygl.h"

// 3, one for current, one for history, one for display
#define BUFFER_HISTORY 3
static uint8_t* map[BUFFER_HISTORY];

static raw_sensor_scan *scans;
static particle current_particle;
//static int arena_width = 14940;
//static int arena_height = 6400;
//static int arena_width = 3740;
//static int arena_height = 1600;
static int arena_width = 10000;
static int arena_height = 10000;

int main (int argc, char **argv) {
  int i, j, iterations;
  pthread_t sensor_thread;
  // sample 3 times (0.3 sec)
  int sample_count = 1;

  // connect to sensor in new thread while we
  // initialize everything else
  sensor_thread = sensor_init_thread();

  scans = malloc(sample_count*sizeof(raw_sensor_scan));

  // swarm_init will set up buffer sizing
  //  swarm_init(RAW_SENSOR_DISTANCES_USB, SENSOR_RANGE_USB, 7380, 3880, 1500);
  swarm_init(RAW_SENSOR_DISTANCES_USB, SENSOR_RANGE_USB, arena_width, arena_height, 2000, 0);

  // allocate buffers
  for (i = 0; i < BUFFER_HISTORY; i++)
    map[i] = buffer_allocate();

  glutInit(&argc, argv);
  // pass size of buffer, then window size
  initGL(map[0], map[2], buffer_get_width(), buffer_get_height(), 2*buffer_get_width(), 2*buffer_get_height());

  // wait for sensor
  assert(pthread_join(sensor_thread, NULL) == 0);

  // start a scan
  sensor_thread = sensor_read_raw_n_thread(sample_count);

  iterations = 0;

  while(1) {
    do {
      swarm_move(0, 0, 360);

      // wait for sensor
      assert(pthread_join(sensor_thread, NULL) == 0);

      // get samples
      for (j = 0; j < sample_count; j++) {
	scans[j] = sensor_fetch_index(j);
      }

      // start a scan
      sensor_thread = sensor_read_raw_n_thread(sample_count);

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

    // draw
    display();

    // clear position
    for (i = -5; i < 6; i++)
      for (j = -5; j < 6; j++ ) {
	record_map_position(0, current_particle.x + i*c - j*s,
			    current_particle.y + i*s + j*c, 0);
	if (!x_y_protected(current_particle.x + i*c - j*s, current_particle.y + i*s + j*c))
	  record_map_position(2, current_particle.x + i*c - j*s,
			      current_particle.y + i*s + j*c, 0);
      }

    // update historical map
    // and display map
    // bzero(map[2], buffer_get_size()*sizeof(uint8_t));
    
    /*
    for (i = 0; i < BUFFER_SIZE; i++) {
      // j is our current value minus a threshold (200)
      j = map[0][i] - 200;
      // if the current buffer has a high value
      if (j > 0) {
	// add to historical buffer,
	// taking care not to wrap around
	if (map[1][i] + j <= 255)
	  map[1][i] += j;
	else
	  map[1][i] = 255;
      } else
	// if the current buffer does not have a high value,
	// the historical buffer is above zero,
	// the index is not protected, and the index is in the
	// scanner's visible arc, attenuate
	if (map[1][i] > 0 && !index_protected(i) && index_is_visible(i, current_particle, scans[0]))
	  map[1][i] *= 0.95;
      // only display a pixel if we are paying attention
      // to it in the particle filter (swarm.c:swarm_filter)
      if (map[1][i] > 200)
	map[2][i] = 255;
    }
    */
    // attenuate current map
    //    buffer_attenuate(map[0], 0.75);

    glutMainLoopEvent();

    iterations++;
  }

  return 0;
}

void record_map_position(int index, int x, int y, uint8_t value) {
  if (in_arena(x, y))
    map[index][buffer_index_from_x_y(x, y)] = value;
}

