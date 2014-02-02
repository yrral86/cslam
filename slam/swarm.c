#include "swarm.h"

// we will store a terrible score at the end
static particle particles[PARTICLE_COUNT+1];
// keep the indices of the top 10 %
int top_ten[PARTICLE_COUNT/10];
static particle current_best;
static int initialized = 0;

void swarm_init() {
  int i, x, y, theta;
  // generate particles
  for (i = 0; i < PARTICLE_COUNT; i++) {
    x = rand_normal(INITIAL_POSITION_VARIANCE);
    y = rand_normal(INITIAL_POSITION_VARIANCE);
    if (!initialized) {
      // generate random position and angle varation around 2 known
      // starting positions
      x += START_END/2;
      theta = rand_normal(180);

      if (rand_limit(2))
	y += ARENA_HEIGHT/4;
      else
	y += 3*ARENA_HEIGHT/4;
    } else
      theta = rand_normal(INITIAL_ANGLE_VARIANCE);

    particles[i] = particle_init(x, y, theta);
  }


  if (!initialized) {
    // terrible score for initializing top ten index array
    particles[PARTICLE_COUNT].score = 10000000;

    current_best.x = 0;
    current_best.y = 0;
    current_best.theta = 0;

    initialized = 1;
  }
}

void swarm_filter(raw_sensor_scan *scans, uint8_t *map, int sample_count) {
  int i, j, k, l;
  particle p;
  int filtered;
  int last_top_ten, current_top_ten, other_top_ten, difference;
  double distance, degrees, theta, dx, dy;

    // evaulate each direction for each particle
  for (i = 0; i < PARTICLE_COUNT; i++) {
    filtered = 0;
    p = particles[i];

    for (j = 0; j < RAW_SENSOR_DISTANCES; j++) {
      for (k = 0; k < sample_count; k++) {
	  distance = scans[k].distances[j];
	  // forward is now 0 degrees, left -, right +
	  degrees = -120 + j*SENSOR_SPACING;

	  theta = (degrees + p.theta + current_best.theta)*M_PI/180;
	  dx = distance*cos(theta) + p.x + current_best.x;
	  dy = distance*sin(theta) + p.y + current_best.y;

	  if (in_arena(dx, dy)) {
	    l = buffer_index_from_x_y(dx, dy);
	    difference = 255 - map[l];
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

  // initialize top_ten to point to terrible score (particles[PARTICLE_COUNT])
  for (i = 0; i <= last_top_ten; i++)
    top_ten[i] = PARTICLE_COUNT;
    
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

  p = particles[top_ten[0]];

  // update current best position
  current_best.x += p.x;
  current_best.y += p.y;
  current_best.theta += p.theta;
}

particle swarm_get_best() {
  return current_best;
}
