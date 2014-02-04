#include "slam.h"

#include "lazygl.h"

// 2, one for current, one for history
#define BUFFER_HISTORY 2
static uint8_t* map[BUFFER_HISTORY];

static raw_sensor_scan *scans;
static particle current_particle;

int main (int argc, char **argv) {
  int i, j, iterations;
  pthread_t sensor_thread;
  // sample 3 times (0.3 sec)
  int sample_count = 1;

  // connect to sensor in new thread while we
  // initialize everything else
  sensor_thread = sensor_init_thread();

  scans = malloc(sample_count*sizeof(raw_sensor_scan));

  // allocate buffers
  for (i = 0; i < BUFFER_HISTORY; i++)
    map[i] = buffer_allocate();

  glutInit(&argc, argv);
  // pass size of buffer, then window size
  initGL(map[0], map[1], BUFFER_WIDTH, BUFFER_HEIGHT, ARENA_WIDTH/8, ARENA_HEIGHT/8);

  rand_normal_init();

  // wait for sensor
  assert(pthread_join(sensor_thread, NULL) == 0);

  // draw initial border
  for (i = 0; i < ARENA_WIDTH; i++)
    for (j = 0; j < 10*BUFFER_FACTOR; j += BUFFER_FACTOR) {
      record_map_position(1, i, j, 255);
      record_map_position(1, i, ARENA_HEIGHT - 1 - j, 255);
    }

  for (i = 0; i < ARENA_HEIGHT; i++)
    for (j = 0; j < 10*BUFFER_FACTOR; j+= BUFFER_FACTOR) {
      record_map_position(1, j, i, 255);
      record_map_position(1, ARENA_WIDTH - 1 - j, i, 255);
    }

  // start a scan
  sensor_thread = sensor_read_raw_n_thread(sample_count);

  iterations = 0;

  while(1) {
    swarm_init();

    // wait for sensor
    assert(pthread_join(sensor_thread, NULL) == 0);

    // get samples
    for (j = 0; j < sample_count; j++) {
      scans[j] = sensor_fetch_index(j);
    }

    // start a scan
    sensor_thread = sensor_read_raw_n_thread(sample_count);

    // give the swarm the new scans and the historical map
    swarm_filter(scans, map[1], sample_count);

    // update localization
    current_particle = swarm_get_best();

    // draw map from best particle
    for (i = 0; i < RAW_SENSOR_DISTANCES; i++)
      for (j = 0; j < sample_count; j++)
	record_distance(i, scans[j].distances[i]);

    // attenuate current map
    buffer_attenuate(map[0], 0.75);

    // draw position
    double s, c, t;
    t = current_particle.theta*M_PI/180;
    s = sin(t);
    c = cos(t);
    for (i = -50; i < 51; i++)
      for (j = -50; j < 51; j++ ) {
	record_map_position(0, current_particle.x + i*c - j*s,
			    current_particle.y + i*s + j*c, 255);
	record_map_position(1, current_particle.x + i*c - j*s,
			    current_particle.y + i*s + j*c, 255);
      }

    // draw
    display();

    // clear position
    for (i = -50; i < 51; i++)
      for (j = -50; j < 51; j++ ) {
	record_map_position(0, current_particle.x + i*c - j*s,
			    current_particle.y + i*s + j*c, 0);
	if (!x_y_protected(current_particle.x + i*c - j*s, current_particle.y + i*s + j*c))
	  record_map_position(1, current_particle.x + i*c - j*s,
			      current_particle.y + i*s + j*c, 0);
      }

    glutMainLoopEvent();

    iterations++;

    // copy current map into historical map
    // and attenuate

    // update historical map
    for (i = 0; i < BUFFER_SIZE; i++) {
      // if the current buffer has a high value
      if (map[0][i] > 150) {
	// add to historical buffer,
	// taking care not to wrap around
	if (map[1][i] <= 235)
	  map[1][i] += 50;
	else
	  map[1][i] = 255;
      } else
	// if the current buffer does not have a high value,
	// the historical buffer is above zero, and
	// the index is not protected, attenuate
	if (map[1][i] > 0)
	  if (!index_protected(i) && index_is_visible(i, current_particle))
	    map[1][i] -= 1;
    }
  }

  return 0;
}

// records into current map (map[0])
void record_distance(int angle_index, double distance) {
  // forward is now 0 degrees, left -, right +
  double degrees = -120 + angle_index*SENSOR_SPACING;
  double theta, dx, dy;
  int x, y;
  theta = (degrees + current_particle.theta)*M_PI/180;
  dx = distance*cos(theta) + current_particle.x;
  dy = distance*sin(theta) + current_particle.y;

  record_map_position(0, dx, dy, 255);
}

void record_map_position(int index, int x, int y, uint8_t value) {
  if (in_arena(x, y))
    map[index][buffer_index_from_x_y(x, y)] = value;
}

int in_arena(int x, int y) {
  if (x >= 0 && x < ARENA_WIDTH && y >= 0 && y < ARENA_HEIGHT)
    return 1;
  else return 0;
}

