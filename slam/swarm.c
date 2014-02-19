#include "swarm.h"

static particle particles[PARTICLE_COUNT];
static particle previous_particles[PARTICLE_COUNT];
static int iterations = 0;

void swarm_init() {
  int i, j, x, y, theta;
  particle p;

  // initialize first round of particles
  if (iterations == 0) {
    for (i = 0; i < PARTICLE_COUNT; i++) {
      x = START_END/2;
      if (rand_limit(2))
	y = ARENA_HEIGHT/4;
      else
	y = 3*ARENA_HEIGHT/4;
      theta = rand_limit(360) - 180;
      particles[i] = particle_init(x, y, theta);
    }
  }

  // add motion (nothing for now, relying on high variance and lots of particles)
  for (i = 0; i < PARTICLE_COUNT; i++) {
    p = particles[i];
    // sample motion distribution
    particles[i] = particle_sample_motion(particles[i], 0, 0, 0);
    // dereference old map, particle_sample_motion copied it already
    landmark_tree_node_dereference(p.map);
  }
}

void swarm_filter(raw_sensor_scan *scans, uint8_t *map, int sample_count) {
  int i, j, k, l;
  particle p, other_top;
  int swap;
  double posterior, distance, degrees, theta, x, y, total;

  total = 0.0;
  // evaulate each direction for each particle
  for (i = 0; i < PARTICLE_COUNT; i++) {
    posterior = 1.0;
    p = particles[i];

    // TODO: this will be _ETH since
    // that will be our localization sensor,
    // just working on new usb driver for the moment
    for (j = 0; j < RAW_SENSOR_DISTANCES_USB; j++) {
      for (k = 0; k < sample_count; k++) {
	  distance = scans[k].distances[j];
	  // forward is now 0 degrees, left -, right +
	  // TODO: same as above (_ETH)
	  degrees = -120 + j*SENSOR_SPACING_USB;

	  theta = (degrees + p.theta)*M_PI/180;
	  x = distance*cos(theta) + p.x;
	  y = distance*sin(theta) + p.y;

	  // make sure it is in bounds
	  if (in_arena(x, y)) {
	    l = buffer_index_from_x_y(x, y);
	    posterior *= landmark_seen_probability(p.map, l);
	  } else posterior *= 0.1;
      }
    }
    particles[i].p *= posterior;
    total += particles[i].p;
  }

  // normalize particle probabilities
  for (i = 0; i < PARTICLE_COUNT; i++)
    particles[i].p /= total;

  // bubblesort particles by p
  swap = 1;
  i = 0;
  do {
    swap = 0;
    for (j = 0; j < PARTICLE_COUNT - i; j++)
      // if the left particle is smaller probability, bubble it right
      if (particles[i].p < particles[j].p) {
	p = particles[i];
	particles[i] = particles[j];
	particles[j] = p;
	swap = 1;
      }
    i++;
  } while (swap);

  // resample with replacement
  memcpy(previous_particles, particles, sizeof(particle)*PARTICLE_COUNT);
  for (i = 0; i < PARTICLE_COUNT; i++) {
    double p = rand()/(double)RAND_MAX;
    total = 0.0;
    j = 0;
    while (total < p)
      total += previous_particles[j++].p;
    particles[i] = previous_particles[j - 1];
    landmark_tree_node_reference(particles[i].map);
  }

  // dereference previous particle maps
  for (i = 0; i < PARTICLE_COUNT; i++) {
    landmark_tree_node_dereference(previous_particles[i].map);
  }

  iterations++;
}

particle swarm_get_best() {
  return previous_particles[0];
}
