#include "record.h"

#include "lazygl.h"

const static int poll_time = 100000;
// 5*5 (map is 5X bigger in both directions than arena) / 10 * 10 (mm -> cm)
// 25*height*width/100
const  static int BUFFER_SIZE = ARENA_HEIGHT*ARENA_WIDTH/4;
#define BUFFER_COUNT 1000
static uint8_t* map[BUFFER_COUNT];
static particle particles[BUFFER_COUNT];
static double spacing;

int main (int argc, char **argv) {
  // allocate buffers
  int i, j;
  for (i = 0; i < BUFFER_COUNT; i++)
    map[i] = malloc(sizeof(uint8_t)*BUFFER_SIZE);

  uint8_t *best_map = map[0];

  spacing = 240.0/RAW_SENSOR_DISTANCES;

  sensor_init();

  raw_sensor_scan scan;

  uint64_t last_poll;

  glutInit(&argc, argv);
  // pass size of buffer, then window size
  initGL(best_map, ARENA_WIDTH/2, ARENA_HEIGHT/2, ARENA_WIDTH, ARENA_HEIGHT);

  init_map();

  while(1) {
    // reset buffers
    for (i = 1; i < BUFFER_COUNT; i++)
      bzero(map[i], BUFFER_SIZE*sizeof(uint8_t));

    // generate particles
    for (i = 1; i < BUFFER_COUNT; i++) {
      // generate random position and angle varation
      // 0.01 meters in either direction
      particles[i].x = rand_limit(2) - 1;
      particles[i].y = rand_limit(2) - 1;
      // 5 degrees in either direction
      particles[i].theta = rand_limit(10) - 5;
    }

    // sample 5 times (0.5 sec)
    for (j = 0; j < 5; j++) {
      scan = sensor_read_raw();
      last_poll = utime();

      // record each direction
      for (i = 0; i < RAW_SENSOR_DISTANCES; i++)
	record_distance(i, scan.distances[i]);

      int sleep_time = poll_time - (utime() - last_poll);
      if (sleep_time > 0)
	usleep(poll_time - (utime() - last_poll));
      else printf("sleepless\n");
    }

    // evaluate buffers
    uint8_t *buffer;
    particle p;
    int k, sum, min, min_index;
    min = 1000000;
    min_index = 1;
    for (i = 1; i < BUFFER_COUNT; i++) {
      buffer = map[i];
      p = particles[i];

      sum = 0;
      // symmetric difference
      // adjusted to allow for some attenuation on the current best map
      for (k = 0; k < BUFFER_SIZE; k++) {
	if (best_map[k] > 200 && buffer[k] != 255)
	  sum++;
	else if (buffer[k] == 255 && best_map[k] <= 200)
	  sum++;
      }

      p.score = sum;

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
	best_map[i] = 255;

    display();

    glutMainLoopEvent();
  }

  // because malloc, eyeroll
  free(map);

  return 0;
}

void init_map() {
  int i, j, last_poll;
  raw_sensor_scan s;

  particles[0].x = 0;
  particles[0].y = 0;
  particles[0].theta = 0;

  for (i = 0; i < 20; i++) {
    s = sensor_read_raw();
    last_poll = utime();

    for (j = 0; j < RAW_SENSOR_DISTANCES; j++)
      record_distance_init(j, s.distances[j]);
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
    map[0][index] = 255;
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
  for (i = 1; i < BUFFER_COUNT; i++) {
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
