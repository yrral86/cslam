#include "swarm.h"

// we will store a terrible score at the end
static particle particles[PARTICLE_COUNT+1];
// keep the indices of the top 10 %
int top_ten[PARTICLE_COUNT/10];
static particle current_best;

void swarm_init() {
  int i;
  // generate particles
  for (i = 0; i < PARTICLE_COUNT; i++) {
    // generate random position and angle varation
    particles[i] = particle_init(rand_normal(INITIAL_POSITION_VARIANCE),
				 rand_normal(INITIAL_POSITION_VARIANCE),
				 rand_normal(INITIAL_ANGLE_VARIANCE));
  }

  // terrible score for initializing top ten index array
  particles[PARTICLE_COUNT].score = 10000000;

  current_best.x = 0;
  current_best.y = 0;
  current_best.theta = 0;
}

void swarm_filter(raw_sensor_scan *scans, uint8_t *map, int sample_count) {
  int i, j, k, l;
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
	  distance = scans[k].distances[j];
	  // forward is now 0 degrees, left -, right +
	  degrees = -120 + j*SENSOR_SPACING;

	  theta = (degrees + p.theta + current_best.theta)*M_PI/180;
	  dx = distance*cos(theta) + p.x + current_best.x;
	  dy = distance*sin(theta) + p.y + current_best.y;
	  // use middle of arena as origin
	  x = ARENA_WIDTH/2 + dx;
	  y = ARENA_HEIGHT/2 + dy;
    
	  if (in_arena(x, y)) {
	    l = buffer_index_from_x_y(x, y);
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

  min_index = top_ten[0];

  p = particles[min_index];

  // update current best position
  // with 10% momentum
  current_best.x += 1.1*p.x;
  current_best.y += 1.1*p.y;
  current_best.theta += 1.1*p.theta;    
}

particle swarm_get_best() {
  return current_best;
}
