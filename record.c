#include "record.h"

#include "lazygl.h"

const static int poll_time = 100000;
// 5*5 (map is 5X bigger in both directions than arena) / 10 * 10 (mm -> cm)
// 25*height*width/100
const  static int BUFFER_SIZE = ARENA_HEIGHT*ARENA_WIDTH/4;
static uint8_t *map;
static double spacing;

int main (int argc, char **argv) {
  int i;
  map = malloc(sizeof(int8_t)*BUFFER_SIZE);

  spacing = 240.0/RAW_SENSOR_DISTANCES;

  sensor_init();

  raw_sensor_scan scan;

  uint64_t last_poll;

  glutInit(&argc, argv);
  // pass size of buffer, then window size
  initGL(map, ARENA_WIDTH/2, ARENA_HEIGHT/2, ARENA_WIDTH, ARENA_HEIGHT);

  while(1) {
    scan = sensor_read_raw();
    last_poll = utime();

    // record each direction
    for (i = 0; i < RAW_SENSOR_DISTANCES; i++)
      record_distance(i, scan.distances[i]);

    display();

    glutMainLoopEvent();

    // attenuate map
    for (i = 0; i < BUFFER_SIZE; i++)
      if (map[i] > 50)
	map[i] *= 0.75;
      else
	map[i] *= 0.95;

    int sleep_time = poll_time - (utime() - last_poll);
    if (sleep_time > 0)
      usleep(poll_time - (utime() - last_poll));
  }

  // because malloc, eyeroll
  free(map);

  return 0;
}

void record_distance(int i, double distance) {
  // mm -> cm
  distance /= 10.0;
  // forward is now 0 degrees, left -, right +
  double degrees = -120 + i*spacing;
  double theta, dx, dy;
  int x, y;
  theta = degrees*M_PI/180;
  dx = distance*cos(theta);
  dy = distance*sin(theta);
  // use middle of buffer as origin
  x = ARENA_WIDTH/4 + dx;
  y = ARENA_HEIGHT/4 + dy;

  int index = y*(ARENA_WIDTH -1)/2 + x;
  if (index >= 0 && index < BUFFER_SIZE && x < ARENA_WIDTH/2)
    map[index] = 255;
}

uint64_t utime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}
