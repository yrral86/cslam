#include "record.h"

#include "lazygl.h"

const static int poll_time = 100000;
const static int INITIAL_POSITION_VARIANCE = 5;
const static int INITIAL_ANGLE_VARIANCE = 15;
// 5*5 (map is 5X bigger in both directions than arena) / 10 * 10 (mm -> cm)
// 25*height*width/100
const  static int BUFFER_SIZE = ARENA_HEIGHT*ARENA_WIDTH/4;
// first 2 buffers are history, so you only have
// BUFFER_COUNT - 2 to work with
#define BUFFER_HISTORY 2
#define BUFFER_COUNT 500
static uint8_t* map[BUFFER_COUNT];
// first particle is reserved for current position
// (relative to buffer center, 0 degrees = + x)
static particle particles[BUFFER_COUNT];
static double spacing;
static uint64_t last_poll;
static raw_sensor_scan *scans;

int main (int argc, char **argv) {
  // sample 3 times (0.3 sec)
  int sample_count = 3;
  scans = malloc(sample_count*sizeof(raw_sensor_scan));

  // allocate buffers
  int i, j, k, l, iterations;
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

  // initialize variance
  particles[0].x = INITIAL_POSITION_VARIANCE;
  particles[0].y = INITIAL_POSITION_VARIANCE;
  particles[0].theta = INITIAL_ANGLE_VARIANCE;

  iterations = 0;

  while(1) {
    for (i = BUFFER_HISTORY; i < BUFFER_COUNT; i++) {
      // reset buffers
      bzero(map[i], BUFFER_SIZE*sizeof(uint8_t));

      // generate particles
      // generate random position and angle varation
      particles[i] = particle_init(rand_normal(INITIAL_POSITION_VARIANCE),
				   rand_normal(INITIAL_POSITION_VARIANCE),
				   rand_normal(INITIAL_ANGLE_VARIANCE));
    }

    // make sure sensor is ready
    int sleep_time = poll_time - (utime() - last_poll);
    if (sleep_time > 0)
      usleep(sleep_time);
    else printf("sleepless\n");

    for (j = 0; j < sample_count; j++) {
      scans[j] = sensor_read_raw();
      last_poll = utime();

      /*
      // record each direction
      for (i = 0; i < RAW_SENSOR_DISTANCES; i++)
	record_distance(i, scan.distances[i]);
      */
      // record 1/20th of directions

      if (j + 1 < sample_count) {
	int sleep_time = poll_time - (utime() - last_poll);
	if (sleep_time > 0)
	  usleep(poll_time - (utime() - last_poll));
	else printf("sleepless\n");
      }
    }

    // evaluate buffers
    // instead of iterating over buffers,
    // lets filter the candidates in a loop, testing 10 new
    // directions each time for each particle and dropping 
    // any particles that are more than 50% new
    uint8_t *buffer;
    particle p;
    /*    int difference, sum, min, min_index;
    min = 1000000;
    min_index = 1;
    for (i = BUFFER_HISTORY; i < BUFFER_COUNT; i++) {
    */
    int filtered, min, min_index;
    int count = BUFFER_COUNT - BUFFER_HISTORY;
    int new_count = 0;
    while (count > 1) {
      // evaulate 10 directions for each particle
      // filter out anything with less than half matching data
      for (i = 0; i < count; i++) {
	filtered = 0;
	buffer = map[BUFFER_HISTORY + i];
	p = particles[BUFFER_HISTORY + i];

	for (j = 0; j < 10; j++) {
	  k = rand_limit(RAW_SENSOR_DISTANCES);
	  for (l = 0; l < sample_count; l++)
	    filtered += record_distance(k, scans[l].distances[k]);
	}

	particle_add_sample(&p, filtered);
      }


      //TODO fix this mess so that it
      // finds top 10% and throws out the rest
      min = 10000000;
      min_index = BUFFER_HISTORY;
      for (i = 0; i < count; i ++) {
	p = particles[BUFFER_HISTORY + i];
	if (p.score < min){
	  min = p.score;
	  min_index= BUFFER_HISTORY + i;
	}
      }

      count = new_count;
      printf("count: %i\n", count);
    }

    /*
	sum = 0;
	// subtract from each historical map
	// this should give us new things to add
	// give more weight to older maps
	for (k = 0; k < BUFFER_SIZE; k++) {
	  difference = buffer[k] - map[1][k];
	// filter out everything in the map
	// that we can't see
	// and make sure difference is significant
	if (difference >= 100)
	  sum += 1;
      }

      p.score = sum;

      // we choose the minimum new to add
      if (sum < min) {
	min = sum;
	min_index = i;
      }
    }
    */

    // attenuate map
    for (i = 0; i < BUFFER_SIZE; i++)
	map[0][i] *= 0.85;

    // update localization
    p = particles[BUFFER_HISTORY];
    particles[0].x += p.x;
    particles[0].y += p.y;
    particles[0].theta += p.theta;

    // draw position
    for (i = -5; i < 6; i++)
      for (j = -5; j < 6; j++ )
	record_map_position(0, ARENA_WIDTH/4 + p.x + i,
			    ARENA_HEIGHT/4 + p.y + j, 255);

    // draw
    display();

    // clear position
    for (i = -5; i < 6; i++)
      for (j = -5; j < 6; j++ )
	record_map_position(0, ARENA_WIDTH/4 + p.x + i,
			    ARENA_HEIGHT/4 + p.y + j, 0);

    // copy best into map
    for (i = 0; i < BUFFER_SIZE; i++)
      if (map[BUFFER_HISTORY][i] == 255)
	map[0][i] = 255;

    glutMainLoopEvent();

    iterations++;

    // copy new map into historical buffer
    // and attenuate
    if (iterations % 20)
      for (i = 0; i < BUFFER_SIZE; i++) {
	map[1][i] *= 0.99;
	if (map[0][i] > 200)
	  if (map[1][i] < 205)
	    map[1][i] += 50;
	  else
	    map[1][i] = 255;
      }
  }

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

  for (i = 0; i < BUFFER_HISTORY; i++)
    record_map_position(i, x, y, 255);
}

int record_distance(int angle_index, double distance) {
  int difference;
  int filter = 0;
  // mm -> cm
  distance /= 10.0;
  // forward is now 0 degrees, left -, right +
  double degrees = -120 + angle_index*spacing;
  double theta, dx, dy;
  int i, j, x, y;
  // generate BUFFER_COUNT - 1 buffers with small variations in postion and angle
  // map[0] is the overall map
  for (i = BUFFER_HISTORY; i < BUFFER_COUNT; i++) {
    theta = (degrees + particles[i].theta + particles[0].theta)*M_PI/180;
    dx = distance*cos(theta) + particles[i].x + particles[0].x;
    dy = distance*sin(theta) + particles[i].y + particles[0].y;
    // use middle of buffer as origin
    x = ARENA_WIDTH/4 + dx;
    y = ARENA_HEIGHT/4 + dy;

    j = index_from_x_y(x, y);
    difference = map[i][j] - map[1][j];
    if (difference < 100)
      record_map_position(i, x, y, 255);
    else
      filter = 1;
  }

  return filter;
}

uint64_t utime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

void record_map_position(int index, int x, int y, uint8_t value) {
  if (x > 0 && x < ARENA_WIDTH/2 && y > 0 && y < ARENA_HEIGHT/2)
    map[index][index_from_x_y(x, y)] = value;
}

int index_from_x_y(int x, int y) {
  return y*(ARENA_WIDTH - 1)/2 + x;
}
