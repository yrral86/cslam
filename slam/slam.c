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
  int sample_count = 3;

  // connect to sensor in new thread while we
  // initialize everything else
  sensor_thread = sensor_init_thread();

  scans = malloc(sample_count*sizeof(raw_sensor_scan));

  // allocate buffers
  for (i = 0; i < BUFFER_HISTORY; i++)
    map[i] = buffer_allocate();

  glutInit(&argc, argv);
  // pass size of buffer, then window size
  initGL(map[0], map[1], BUFFER_WIDTH, BUFFER_HEIGHT, ARENA_WIDTH/2, ARENA_HEIGHT/2);

  rand_normal_init();

  // wait for sensor
  assert(pthread_join(sensor_thread, NULL) == 0);

  // read initial map
  sensor_thread = sensor_read_raw_n_thread(INITIAL_SCANS);
  // init_map joins thread
  init_map(sensor_thread);

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
    for (i = -5; i < 6; i++)
      for (j = -5; j < 6; j++ ) {
	record_map_position(0, ARENA_WIDTH/2 +current_particle.x + i,
			    ARENA_HEIGHT/2 +current_particle.y + j, 255);
	record_map_position(1, ARENA_WIDTH/2 +current_particle.x + i,
			    ARENA_HEIGHT/2 +current_particle.y + j, 255);
      }

    // draw
    display();

    // clear position
    for (i = -5; i < 6; i++)
      for (j = -5; j < 6; j++ ) {
	record_map_position(0, ARENA_WIDTH/2 +current_particle.x + i,
			    ARENA_HEIGHT/2 +current_particle.y + j, 0);
	record_map_position(1, ARENA_WIDTH/2 +current_particle.x + i,
			    ARENA_HEIGHT/2 +current_particle.y + j, 0);
      }

    glutMainLoopEvent();

    iterations++;

    // copy current map into historical map
    // and attenuate
    for (i = 0; i < BUFFER_SIZE; i++) {
      if (map[1][i] > 0)
	map[1][i] -= 1;
      if (map[0][i] > 150)
	if (map[1][i] < 235)
	  map[1][i] += 50;
	else
	  map[1][i] = 255;
    }
  }

  return 0;
}

void init_map(pthread_t t) {
  int i, j;
  raw_sensor_scan s;

  // wait for sensor
  assert(pthread_join(t, NULL) == 0);  

  for (i = 0; i < INITIAL_SCANS; i++) {
    s = sensor_fetch_index(i);

    for (j = 0; j < RAW_SENSOR_DISTANCES; j++)
      record_distance_init(j, s.distances[j]);
  }
}

void record_distance_init(int angle_index, double distance) {
    // forward is now 0 degrees, left -, right +
  double degrees = -120 + angle_index*SENSOR_SPACING;
  double theta, dx, dy;
  int i, x, y;

  theta = degrees*M_PI/180;
  dx = distance*cos(theta);
  dy = distance*sin(theta);
  // use middle of buffer as origin
  x = BUFFER_WIDTH/2 + dx;
  y = BUFFER_HEIGHT/2 + dy;

  for (i = 0; i < BUFFER_HISTORY; i++)
    record_map_position(i, x, y, 255);
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
  // use middle of buffer as origin
  x = BUFFER_WIDTH/2 + dx;
  y = BUFFER_HEIGHT/2 + dy;

  record_map_position(0, x, y, 255);
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
