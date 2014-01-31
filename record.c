#include "record.h"

#include "lazygl.h"

const static int poll_time = 100000;
// 5*5 (map is 5X bigger in both directions than arena) / 10 * 10 (mm -> cm)
// 25*height*width/100
const  static int BUFFER_SIZE = ARENA_HEIGHT*ARENA_WIDTH/4;
// first 10 buffers are history, so you only have
// BUFFER_COUNT - 2 to work with
#define BUFFER_HISTORY 2
#define BUFFER_COUNT 100
static uint8_t* map[BUFFER_COUNT];
static particle particles[BUFFER_COUNT];
static double spacing;
static uint64_t last_poll;

int main (int argc, char **argv) {
  // allocate buffers
  int i, j, k, iterations;
  for (i = 0; i < BUFFER_COUNT; i++)
    map[i] = malloc(sizeof(uint8_t)*BUFFER_SIZE);

  spacing = 240.0/RAW_SENSOR_DISTANCES;

  sensor_init();

  raw_sensor_scan scan;

  glutInit(&argc, argv);
  // pass size of buffer, then window size
  initGL(map[0], map[1], ARENA_WIDTH/2, ARENA_HEIGHT/2, ARENA_WIDTH/2, ARENA_HEIGHT/2);

  init_map();

  rand_normal_init();

  iterations = 0;
  while(1) {
    for (i = BUFFER_HISTORY; i < BUFFER_COUNT; i++) {
      // reset buffers
      bzero(map[i], BUFFER_SIZE*sizeof(uint8_t));

      // generate particles
      // generate random position and angle varation
      // 0.05 meters in either direction
      particles[i].x = rand_normal(5);
      particles[i].y = rand_normal(5);
      // 5 degrees in either direction
      particles[i].theta = rand_normal(5);
    }

    // make sure sensor is ready
    int sleep_time = poll_time - (utime() - last_poll);
    if (sleep_time > 0)
      usleep(sleep_time);
    else printf("sleepless\n");

    // sample 3 times (0.3 sec)
    int sample_count = 3;
    for (j = 0; j < sample_count; j++) {
      scan = sensor_read_raw();
      last_poll = utime();

      // record each direction
      for (i = 0; i < RAW_SENSOR_DISTANCES; i++)
	record_distance(i, scan.distances[i]);

      if (j + 1 < sample_count) {
	int sleep_time = poll_time - (utime() - last_poll);
	if (sleep_time > 0)
	  usleep(poll_time - (utime() - last_poll));
	else printf("sleepless\n");
      }
    }

    // evaluate buffers
    uint8_t *buffer;
    particle p;
    int difference, sum, min, min_index;
    min = 1000000;
    min_index = 1;
    for (i = BUFFER_HISTORY; i < BUFFER_COUNT; i++) {
      buffer = map[i];
      p = particles[i];

      sum = 0;
      // subtract from each historical map
      // this should give us new things to add
      // give more weight to older maps
      for (k = 0; k < BUFFER_SIZE; k++) {
	difference = buffer[k] - map[1][k];
	// filter out everything in the map
	// that we can't see
	// and make sure difference is significant
	if (difference >= 200)
	  sum += 1;
      }

      p.score = sum;

      // we choose the minimum new to add
      if (sum < min) {
	min = sum;
	min_index = i;
      }
    }

    // attenuate map
    for (i = 0; i < BUFFER_SIZE; i++)
	map[0][i] *= 0.9;

    // update localization
    particles[0].x += particles[min_index].x;
    particles[0].y += particles[min_index].y;
    particles[0].theta += particles[min_index].theta;

    // copy best into map
    for (i = 0; i < BUFFER_SIZE; i++)
      if (map[min_index][i] == 255)
	map[0][i] = 255;

    display();

    glutMainLoopEvent();

    iterations++;
    /*
    // shift history buffers every 10 iterations
    // filtering out anything less than 200
    if (iterations % 10 == 0)
      for (i = BUFFER_HISTORY - 1; i > 0; i--) {
	memcpy(map[i], map[i - 1], sizeof(uint8_t)*BUFFER_SIZE);
	for (j = 0; j < BUFFER_SIZE; j++)
	  if (map[i][j] != 0 && map[i][j] < 200)
	    map[i][j] = 0;
      }
    */

    // copy new map into historical buffer
    if (iterations % 10)
      for (i = 0; i < BUFFER_SIZE; i++)
	if (map[0][i] > 200)
	  map[1][i] = 255;
  }

  // because malloc, eyeroll
  free(map);

  return 0;
}

void init_map() {
  int i, j;
  raw_sensor_scan s;

  particles[0].x = 0;
  particles[0].y = 0;
  particles[0].theta = 0;

  for (i = 0; i < 20; i++) {
    s = sensor_read_raw();
    last_poll = utime();

    for (j = 0; j < RAW_SENSOR_DISTANCES; j++)
      record_distance_init(j, s.distances[j]);

    int sleep_time = poll_time - (utime() - last_poll);
    if (sleep_time > 0)
      usleep(sleep_time);
    else printf("sleepless\n");
  }

  for (i = 0; i < BUFFER_SIZE; i++) {
    map[0][i] *= 0.95;
  }
}

void record_distance_init(int angle_index, double distance) {
  // mm -> cm
  distance /= 10.0;
  // forward is now 0 degrees, left -, right +
  double degrees = -120 + angle_index*spacing;
  double theta, dx, dy;
  int i, x, y;

  theta = degrees*M_PI/180;
  dx = distance*cos(theta);
  dy = distance*sin(theta);
  // use middle of buffer as origin
  x = ARENA_WIDTH/4 + dx;
  y = ARENA_HEIGHT/4 + dy;

  int index = y*(ARENA_WIDTH -1)/2 + x;
  if (index >= 0 && index < BUFFER_SIZE && x < ARENA_WIDTH/2)
    for (i = 0; i < BUFFER_HISTORY; i++)
      map[i][index] = 255;
}

void record_distance(int angle_index, double distance) {
  // mm -> cm
  distance /= 10.0;
  // forward is now 0 degrees, left -, right +
  double degrees = -120 + angle_index*spacing;
  double theta, dx, dy;
  int i, x, y;
  // generate BUFFER_COUNT - 1 buffers with small variations in postion and angle
  // map[0] is the overall map
  for (i = BUFFER_HISTORY; i < BUFFER_COUNT; i++) {
    theta = (degrees + particles[i].theta + particles[0].theta)*M_PI/180;
    dx = distance*cos(theta) + particles[i].x + particles[0].x;
    dy = distance*sin(theta) + particles[i].y + particles[0].y;
    // use middle of buffer as origin
    x = ARENA_WIDTH/4 + dx;
    y = ARENA_HEIGHT/4 + dy;

    int index = y*(ARENA_WIDTH -1)/2 + x;
    if (index >= 0 && index < BUFFER_SIZE && x < ARENA_WIDTH/2)
      map[i][index] = 255;
  }
}

uint64_t utime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}
