#include "swarm.h"

static particle particles[PARTICLE_COUNT];
static particle previous_particles[PARTICLE_COUNT];
static particle best_particle;
static int iterations = 0;
// TODO: _ETH
static double K[RAW_SENSOR_DISTANCES_USB*3], H[RAW_SENSOR_DISTANCES_USB*3], P[9], PH[3*RAW_SENSOR_DISTANCES_USB], HPH[RAW_SENSOR_DISTANCES_USB*RAW_SENSOR_DISTANCES_USB];
// 1% of measurement, avereage around 40 mm
static double R = 40;
// TODO: VRV(T) to scale R based on distances

void swarm_init() {
  int i, j, k, x, y, theta;
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

      // draw initial border
      for (k = 0; k < ARENA_WIDTH; k++)
	for (j = 0; j < BORDER_WIDTH*BUFFER_FACTOR; j += BUFFER_FACTOR) {
	  landmark_set_seen_value(particles[i].map, buffer_index_from_x_y(k, j), 10000);
	  landmark_set_seen_value(particles[i].map,
				  buffer_index_from_x_y(k, ARENA_HEIGHT - 1 - j), 10000);
	}

      for (k = 0; k < ARENA_HEIGHT; k++)
	for (j = 0; j < BORDER_WIDTH*BUFFER_FACTOR; j+= BUFFER_FACTOR) {
	  landmark_set_seen_value(particles[i].map, buffer_index_from_x_y(j, k), 10000);
	  landmark_set_seen_value(particles[i].map,
				  buffer_index_from_x_y(ARENA_WIDTH - 1 - j, k), 10000);
	}

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
  int i, j, k, l, m;
  int swap;
  double posterior, distance, degrees, theta, x, y, total;

  total = 0.0;
  // evaulate each direction for each particle
  for (i = 0; i < PARTICLE_COUNT; i++) {
    posterior = 1.0;

    // EKF
    //    H = magical_jacobian_magic();
    // TODO: _ETH
    bzero(H,sizeof(double)*RAW_SENSOR_DISTANCES_USB*3);
    double var[3];
    var[0] = particles[i].x_var;
    var[1] = particles[i].y_var;
    var[2] = particles[i].theta_var;
    for (j = 0; j < 3; j++)
      for (k = 0; k < 3; k++)
	if (j == k)
	  P[j*3+k] = var[j];
	else
	  P[j*3+k] = sqrt(var[j])*sqrt(var[k]);
    
    // K
    // TODO: _ETH
    bzero(K, sizeof(double)*RAW_SENSOR_DISTANCES_USB*3);
    // TODO: _ETH
    for (j = 0; j < RAW_SENSOR_DISTANCES_USB; j++)
      for (k = 0; k < 3; k++)
	for (l = 0; l < 3; l++)
	  // TODO _ETH
	  PH[k*3 + j] += P[k*3 +l]*H[j*RAW_SENSOR_DISTANCES_USB + l];

    // TODO: calculate denominator and invert



    // TODO: this will be _ETH since
    for (j = 0; j < RAW_SENSOR_DISTANCES_USB; j++) {
      for (k = 0; k < sample_count; k++) {
	  distance = scans[k].distances[j];
	  // forward is now 0 degrees, left -, right +
	  // TODO: same as above (_ETH)
	  degrees = -120 + j*SENSOR_SPACING_USB;
	  theta = (degrees + particles[i].theta)*M_PI/180;

	  // check and record unseen every 10 mm
	  for (l = 0; l < distance; l += 10) {
	    x = l*cos(theta) + particles[i].x;
	    y = l*sin(theta) + particles[i].y;

	    // make sure it is in bounds
	    if (in_arena(x, y)) {
	      m = buffer_index_from_x_y(x, y);
	      posterior *= landmark_unseen_probability(particles[i].map, m);
	      // only update map once a second
	      if (iterations % 10 == 0)
		landmark_set_unseen(particles[i].map, m);
	    } else posterior *= 0.05;
	  }

	  // check and record seen
	  x = distance*cos(theta) + particles[i].x;
	  y = distance*sin(theta) + particles[i].y;

	  // make sure it is in bounds
	  if (in_arena(x, y)) {
	    m = buffer_index_from_x_y(x, y);
	    posterior *= landmark_seen_probability(particles[i].map, m);
	    // only update map once a second
	    if (iterations % 10 == 0)
	      landmark_set_seen(particles[i].map, m);
	  } else posterior *= 0.05;
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
  particle temp;
  do {
    swap = 0;
    for (j = 0; j < PARTICLE_COUNT - i - 1; j++)
      // if the left particle is smaller probability, bubble it right
      if (particles[j].p < particles[j + 1].p) {
	temp = particles[j];
	particles[j] = particles[j + 1];
	particles[j + 1] = temp;
	swap = 1;
      }
    i++;
  } while (swap);

  // save old particles before we resample
  memcpy(previous_particles, particles, sizeof(particle)*PARTICLE_COUNT);
  // resample with replacement
  for (i = 0; i < PARTICLE_COUNT; i++) {
    double p = rand()/(double)RAND_MAX;
    total = 0.0;
    j = 0;
    while (j++ && total < p)
      total += previous_particles[j - 1].p;
    particles[i] = previous_particles[j - 1];
    particles[i].map = landmark_tree_copy(particles[i].map);
  }

  // save best, copy the map we are about to dereference
  best_particle = previous_particles[0];
  best_particle.map = landmark_tree_copy(best_particle.map);

  // dereference previous particle maps
  for (i = 0; i < PARTICLE_COUNT; i++) {
    landmark_tree_node_dereference(previous_particles[i].map);
  }

  iterations++;
}

particle swarm_get_best() {
  return best_particle;
}
