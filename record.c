#include "record.h"

#include "lazygl.h"

const static int poll_time = 100000;
static uint8_t map[ARENA_HEIGHT*ARENA_WIDTH];
static double spacing;

int main (int argc, char **argv) {
  int i;
  //  map = malloc(sizeof(uint8_t)*ARENA_WIDTH*ARENA_HEIGHT);

  spacing = 240.0/RAW_SENSOR_DISTANCES;

  sensor_init();

  raw_sensor_scan scan;

  uint64_t last_poll;

  glutInit(&argc, argv);
  initGL(map, ARENA_WIDTH, ARENA_HEIGHT);

  while(1) {
    scan = sensor_read_raw();
    last_poll = utime();

    // record each direction
    for (i = 0; i < RAW_SENSOR_DISTANCES; i++)
      record_distance(i, scan.distances[i]);

    display();

    glutMainLoopEvent();

    // attenuate map
    for (i = 0; i < ARENA_HEIGHT*ARENA_WIDTH; i++)
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
  // forward is now 0 degrees, left +, right -
  int degrees = 120 - i*spacing;
  double theta, dx, dy;
  int x, y;
  theta = degrees*M_PI/180;
  dx = distance*cos(theta);
  dy = distance*sin(theta);
  x = 300 + dx;
  y = ARENA_HEIGHT/2 + dy;
  if (in_bounds(x, y)) {
    map[ARENA_WIDTH*y + x] += 100;
  }
}

uint64_t utime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}
