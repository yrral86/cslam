#include "slam.h"

#include "lazygl.h"

const static int poll_time = 100000;
const static int INITIAL_POSITION_VARIANCE = 5;
const static int INITIAL_ANGLE_VARIANCE = 15;
// 5*5 (map is 5X bigger in both directions than arena) / 10 * 10 (mm -> cm)
// 25*height*width/100
const static int BUFFER_WIDTH = ARENA_WIDTH/2;
const static int BUFFER_HEIGHT = ARENA_HEIGHT/2;
const  static int BUFFER_SIZE = ARENA_HEIGHT*ARENA_WIDTH/4;
// 2, one for current, one for history
#define BUFFER_HISTORY 2
#define PARTICLE_COUNT 400
static uint8_t* map[BUFFER_HISTORY];
// first particle is reserved for current position
// (relative to buffer center, 0 degrees = + x)
// second particle is reserved for initializing the top 10% indice list
static particle particles[PARTICLE_COUNT];
// keep the indices of the top 10 %
int top_ten[PARTICLE_COUNT/10];

static double spacing;
static uint64_t last_poll;
static raw_sensor_scan *scans;

int main (int argc, char **argv) {
  int i, j, k, l, iterations;
  // sample 3 times (0.3 sec)
  int sample_count = 3;
  scans = malloc(sample_count*sizeof(raw_sensor_scan));

  // allocate buffers
  for (i = 0; i < BUFFER_HISTORY; i++)
    map[i] = malloc(sizeof(uint8_t)*BUFFER_SIZE);

  spacing = 240.0/RAW_SENSOR_DISTANCES;

  sensor_init();

  raw_sensor_scan scan;

  glutInit(&argc, argv);
  // pass size of buffer, then window size
  initGL(map[0], map[1], BUFFER_WIDTH, BUFFER_HEIGHT, BUFFER_WIDTH, BUFFER_HEIGHT);

  init_map();

  rand_normal_init();

  // initialize terrible score
  particles[1] = particle_init(0, 0, 0);
  particle_add_sample(particles + 1, 100000000000);

  iterations = 0;

  while(1) {
    // generate particles
    for (i = 0; i < PARTICLE_COUNT; i++) {
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

    // get samples
    for (j = 0; j < sample_count; j++) {
      scans[j] = sensor_read_raw();
      last_poll = utime();

      if (j + 1 < sample_count) {
	int sleep_time = poll_time - (utime() - last_poll);
	if (sleep_time > 0)
	  usleep(poll_time - (utime() - last_poll));
	else printf("sleepless\n");
      }
    }

    // score particles by how much new information they contain
    uint8_t *buffer;
    particle p;
    int filtered, min_index;
    int last_top_ten, current_top_ten, other_top_ten, x, y, difference;
    double distance, degrees, theta, dx, dy;

    // evaulate each direction for each particle
    for (i = 0; i < PARTICLE_COUNT; i++) {
      filtered = 0;
      p = particles[i];

      for (j = 0; j < RAW_SENSOR_DISTANCES; j++) {
	for (k = 0; k < sample_count; k++) {
	  // mm -> cm
	  distance = scans[k].distances[j]/10.0;
	  // forward is now 0 degrees, left -, right +
	  degrees = -120 + j*spacing;

	  theta = (degrees + p.theta + particles[0].theta)*M_PI/180;
	  dx = distance*cos(theta) + p.x + particles[0].x;
	  dy = distance*sin(theta) + p.y + particles[0].y;
	  // use middle of buffer as origin
	  x = BUFFER_WIDTH/2 + dx;
	  y = BUFFER_HEIGHT/2 + dy;
    
	  if (in_bounds(x, y)) {
	    l = index_from_x_y(x, y);
	    difference = 255 - map[1][l];
	    // allow roughly 18 attenuations from full recognition
	    if (difference > 200)
	      filtered++;
	  } else filtered++;
	}
      }
      particles[i].score = filtered;
    }

    // find best particle
    // finds top 10%, ordered best first
    last_top_ten = PARTICLE_COUNT/10 - 1;

    // initialize top_ten to point to terrible score (particles[1])
    bzero(top_ten, sizeof(int)*PARTICLE_COUNT/10);
    for (i = 0; i <= last_top_ten; i++)
      top_ten[i]++;
    
    for (i = 0; i < PARTICLE_COUNT; i++) {
      p = particles[i];
      current_top_ten = last_top_ten;

      // if we have better than the worst saved, replace it
      if (p.score < particles[top_ten[current_top_ten]].score)
	top_ten[current_top_ten] = i;

      // bubble towards front of list
      while (current_top_ten > 0 && p.score < particles[top_ten[current_top_ten - 1]].score) {
	other_top_ten = top_ten[current_top_ten];
	top_ten[current_top_ten] = top_ten[current_top_ten - 1];
	top_ten[current_top_ten - 1] = other_top_ten;
	current_top_ten--;
      }
    }

    min_index = top_ten[0];

    // draw best particle
    for (i = 0; i < RAW_SENSOR_DISTANCES; i++)
      for (j = 0; j < sample_count; j++)
	record_distance(min_index, i, scans[j].distances[i]);

    // attenuate map
    for (i = 0; i < BUFFER_SIZE; i++)
	map[0][i] *= 0.75;

    // update localization
    // assuming 10% momentum
    p = particles[min_index];
    particles[0].x += 1.1*p.x;
    particles[0].y += 1.1*p.y;
    particles[0].theta += 1.1*p.theta;

    // draw position
    for (i = -5; i < 6; i++)
      for (j = -5; j < 6; j++ ) {
	record_map_position(0, BUFFER_WIDTH/2 + p.x + i,
			    BUFFER_HEIGHT/2 + p.y + j, 255);
	record_map_position(1, BUFFER_WIDTH/2 + p.x + i,
			    BUFFER_HEIGHT/2 + p.y + j, 255);
      }

    // draw
    display();

    // clear position
    for (i = -5; i < 6; i++)
      for (j = -5; j < 6; j++ ) {
	record_map_position(0, BUFFER_WIDTH/2 + p.x + i,
			    BUFFER_HEIGHT/2 + p.y + j, 0);
	record_map_position(1, BUFFER_WIDTH/2 + p.x + i,
			    BUFFER_HEIGHT/2 + p.y + j, 0);
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
  x = BUFFER_WIDTH/2 + dx;
  y = BUFFER_HEIGHT/2 + dy;

  for (i = 0; i < BUFFER_HISTORY; i++)
    record_map_position(i, x, y, 255);
}

// records into current map (map[0])
void record_distance(int particle_index, int angle_index, double distance) {
  // mm -> cm
  distance /= 10.0;
  // forward is now 0 degrees, left -, right +
  double degrees = -120 + angle_index*spacing;
  double theta, dx, dy;
  int x, y;
  theta = (degrees + particles[particle_index].theta + particles[0].theta)*M_PI/180;
  dx = distance*cos(theta) + particles[particle_index].x + particles[0].x;
  dy = distance*sin(theta) + particles[particle_index].y + particles[0].y;
  // use middle of buffer as origin
  x = BUFFER_WIDTH/2 + dx;
  y = BUFFER_HEIGHT/2 + dy;

  record_map_position(0, x, y, 255);
}

uint64_t utime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

void record_map_position(int index, int x, int y, uint8_t value) {
  if (in_bounds(x, y))
    map[index][index_from_x_y(x, y)] = value;
}

int index_from_x_y(int x, int y) {
  return y*(ARENA_WIDTH - 1)/2 + x;
}

int in_bounds(int x, int y) {
  if (x > 0 && x < BUFFER_WIDTH && y > 0 && y < BUFFER_HEIGHT)
    return 1;
  else return 0;
}
