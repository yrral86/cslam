#include "swarm.h"

// we will store a terrible score at the end
static particle particles[PARTICLE_COUNT+1];
// keep the top 1 %
static particle top[PARTICLE_COUNT/100];
static int iterations = 0;

void swarm_init() {
  int i, j, x, y, theta;
  particle p;

  // initialize
  if (iterations == 0) {
    for (i = 0; i < PARTICLE_COUNT/100; i++) {
      x = START_END/2;
      if (rand_limit(2))
	y = ARENA_HEIGHT/4;
      else
	y = 3*ARENA_HEIGHT/4;
      theta = rand_limit(360) - 180;
      top[i] = particle_init(x, y, theta);
    }
    // terrible score for initializing top 1% array
    particles[PARTICLE_COUNT].score = 10000000;
  }

  // copy top 1%
  memcpy(particles, top, (PARTICLE_COUNT/100)*sizeof(uint8_t));

  // generate random particles based on top 1%
  for (i = PARTICLE_COUNT/100; i < PARTICLE_COUNT - PARTICLE_COUNT/100; i++) {
    // chose a random top 1% particle
    p = top[rand_limit(PARTICLE_COUNT/100)];
    // add some variation
    x = p.x + rand_normal(INITIAL_POSITION_VARIANCE);
    y = p.y + rand_normal(INITIAL_POSITION_VARIANCE);
    theta = p.theta + rand_normal(INITIAL_ANGLE_VARIANCE);

    // save the particle
    particles[i] = particle_init(x, y, theta);
  }

  // finish off with 1% random particles
  for (i = PARTICLE_COUNT - PARTICLE_COUNT/100; i < PARTICLE_COUNT; i++) {
    // anywhere
    x = rand_limit(ARENA_WIDTH);
    y = rand_limit(ARENA_HEIGHT);

    // any angle
    theta = rand_limit(360) - 180;

    // save it
    particles[i] = particle_init(x, y, theta);
  }
}

void swarm_filter(raw_sensor_scan *scans, uint8_t *map, int sample_count) {
  int i, j, k, l;
  particle p, other_top;
  int filtered;
  int last_top, current_top, difference;
  double distance, degrees, theta, x, y;

    // evaulate each direction for each particle
  for (i = 0; i < PARTICLE_COUNT; i++) {
    filtered = 0;
    p = particles[i];

    for (j = 0; j < RAW_SENSOR_DISTANCES; j++) {
      for (k = 0; k < sample_count; k++) {
	  distance = scans[k].distances[j];
	  // forward is now 0 degrees, left -, right +
	  degrees = -120 + j*SENSOR_SPACING;

	  theta = (degrees + p.theta)*M_PI/180;
	  x = distance*cos(theta) + p.x;
	  y = distance*sin(theta) + p.y;

	  // make sure it is in bounds
	  if (in_arena(x, y)) {
	    l = buffer_index_from_x_y(x, y);
	    difference = 255 - map[l];
	    // punish the particle if the data doesn't agree
	    // with the historical map
	    if (difference > 200)
	      filtered++;
	  } else filtered += 2;
      }
    }
    particles[i].score = filtered;
  }

  // find best particle
  // finds top 1%, ordered best first
  last_top = PARTICLE_COUNT/100 - 1;

  // initialize top to terrible score (particles[PARTICLE_COUNT])
  for (i = 0; i <= last_top; i++)
    top[i] = particles[PARTICLE_COUNT];
    
  for (i = 0; i < PARTICLE_COUNT; i++) {
    p = particles[i];
    current_top = last_top;

    // if we have better than the worst saved, replace it
    if (p.score < top[current_top].score)
      top[current_top] = p;

    // bubble towards front of list
    while (current_top > 0 && p.score < top[current_top - 1].score) {
      other_top = top[current_top];
      top[current_top] = top[current_top - 1];
      top[current_top - 1] = other_top;
      current_top--;
    }
  }

  iterations++;
}

particle swarm_get_best() {
  return top[0];
}
