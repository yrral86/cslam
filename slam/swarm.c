#include "swarm.h"
#include <stdio.h>

static particle particles[PARTICLE_COUNT];
static particle previous_particles[PARTICLE_COUNT];
static particle best_particle;
static int iterations = 0;
static int m, sensor_degrees, long_side, short_side, start;
static double spacing;

/*
// TODO: _ETH
double K[3*RAW_SENSOR_DISTANCES_USB], H[RAW_SENSOR_DISTANCES_USB*3], P[9], PH[3*RAW_SENSOR_DISTANCES_USB], HPH[RAW_SENSOR_DISTANCES_USB*RAW_SENSOR_DISTANCES_USB];
// 1% of measurement, avereage around 40 mm
double R = 40;
// TODO: VRV(T) to scale R based on distances
*/
void swarm_init(int m_in, int degrees_in, int long_side_in, int short_side_in, int start_in) {
  int i, j, k;
  double x, y, theta;
  particle initial_map;
  m = m_in;
  sensor_degrees = degrees_in;
  long_side = long_side_in;
  short_side = short_side_in;
  start = start_in;
  spacing = sensor_degrees/(double)(m);

  buffer_set_arena_size(long_side, short_side);

  initial_map.map = landmark_map_copy(NULL);

  // draw initial border
  for (k = 0; k < long_side; k += BUFFER_FACTOR)
    for (j = 0; j < BORDER_WIDTH; j += BUFFER_FACTOR) {
      landmark_set_seen_value(initial_map.map, buffer_index_from_x_y((double)k, (double)j), 10000);
      landmark_set_seen_value(initial_map.map,
			      buffer_index_from_x_y((double)k, (double)(short_side - 1 - j)), 10000);
    }

  for (k = 0; k < short_side; k += BUFFER_FACTOR)
    for (j = 0; j < BORDER_WIDTH; j += BUFFER_FACTOR) {
      landmark_set_seen_value(initial_map.map, buffer_index_from_x_y((double)j, (double)k), 10000);
      landmark_set_seen_value(initial_map.map,
			      buffer_index_from_x_y((double)(long_side - 1 - j), (double)k), 10000);
    }

  // initialize first round of particles
  x = start/2;
  for (i = 0; i < PARTICLE_COUNT; i++) {
    y = short_side/4;
    if (rand_limit(2))
      y *= 3;
    theta = rand_limit(360) - 180;
    particles[i] = particle_init(x, y, theta);
    particles[i].map = initial_map.map;
    landmark_map_reference(particles[i].map);
  }

  landmark_map_dereference(initial_map.map);
}

void swarm_move(int dx, int dy, int dtheta) {
  int i;
  particle p;

  // add motion (nothing for now, relying on high variance and lots of particles)
  for (i = 0; i < PARTICLE_COUNT; i++) {
    p = particles[i];
    // sample motion distribution
    particles[i] = particle_sample_motion(particles[i], dx, dy, dtheta);
    // dereference old map, particle_sample_motion copied it already
    landmark_map_dereference(p.map);
  }
}

void swarm_update(int *distances) {
  int i, j, k, l;
  int swap;
  double posterior, distance, degrees, theta, x, y, s, c, total, min, p, step;
  double xyt[3];
  particle temp;

  min = 10000.0;
  // evaulate each direction for each particle
  for (i = 0; i < PARTICLE_COUNT; i++) {
    posterior = 0.0;

    /*
    // EKF
    //    H = magical_jacobian_magic();
    // TODO: _ETH
    bzero(H, sizeof(double)*RAW_SENSOR_DISTANCES_USB*3);
    xyt[0] = particles[i].x_var;
    xyt[1] = particles[i].y_var;
    xyt[2] = particles[i].theta_var;
    for (j = 0; j < 3; j++)
      for (k = 0; k < 3; k++)
	if (j == k)
	  P[j*3+k] = xyt[j];
	else
	  P[j*3+k] = sqrt(xyt[j])*sqrt(xyt[k]);
    
    // K NUMERATOR
    // TODO: _ETH
    bzero(PH, sizeof(double)*3*RAW_SENSOR_DISTANCES_USB);
    // TODO: _ETH
    for (j = 0; j < RAW_SENSOR_DISTANCES_USB; j++)
      for (k = 0; k < 3; k++)
	for (l = 0; l < 3; l++)
	  // TODO _ETH
	  PH[k*RAW_SENSOR_DISTANCES_USB + j] += P[k*3 + l]*H[k*RAW_SENSOR_DISTANCES_USB + l];

    
    // K DENOMINATOR
    // TODO: _ETH
    bzero(HPH, sizeof(double)*RAW_SENSOR_DISTANCES_USB*RAW_SENSOR_DISTANCES_USB);
    for (j = 0; j < RAW_SENSOR_DISTANCES_USB; j++)
      for (k = 0; k < RAW_SENSOR_DISTANCES_USB; k++)
	for (l = 0; l < 3; l++) {
	  HPH[k*RAW_SENSOR_DISTANCES_USB + j] += H[k*3 + l]*PH[l*RAW_SENSOR_DISTANCES_USB + k]; 
	  if (j == k)
	    HPH[k*RAW_SENSOR_DISTANCES_USB + j] += R;
	}

    // TODO: INVERT K DENOMINATOR

    // K
    // TODO: _ETH
    bzero(K, sizeof(double)*3*RAW_SENSOR_DISTANCES_USB);
    for (j = 0; j < RAW_SENSOR_DISTANCES_USB; j++)
      for (k = 0; k < 3; k++)
	for (l = 0; l < RAW_SENSOR_DISTANCES_USB; l++)
	  K[k*RAW_SENSOR_DISTANCES_USB + j] += PH[k*RAW_SENSOR_DISTANCES_USB + l] *
	    HPH[l*RAW_SENSOR_DISTANCES_USB + j];

    // K(actual - simulated)
    sim = landmark_map_simulate_scan(particles[i]);
    bzero(xyt, sizeof(double)*3);
    for (j = 0; j < 3; j++)
      for (l = 0; l < RAW_SENSOR_DISTANCES_USB; l++)
	// use first scan if there are multiple
	xyt[j] += K[j*RAW_SENSOR_DISTANCES + l]*(scans[0].distances[l]-sim.distances[l]);

    // adjust particle
    particles[i].x += xyt[0];
    particles[i].y += xyt[1];
    particles[i].theta += xyt[2];
    */

    // evaluate the particle's relative probability
    for (j = 0; j < m; j++) {
      distance = distances[j];
      // forward is now 0 degrees, left -, right +
      degrees = -sensor_degrees/2.0 + j*spacing;
      theta = (degrees + particles[i].theta)*M_PI/180;
      s = sin(theta);
      c = cos(theta);

      // check and record unseen every 1000 mm
      for (l = 0; l < distance; l += 1000) {
	x = l*c + particles[i].x;
	y = l*s + particles[i].y;

	// make sure it is in bounds
	if (in_arena(x, y)) {
	  k = buffer_index_from_x_y(x, y);
	  p = landmark_unseen_probability(particles[i].map, k);
	  posterior += -log2(p);
	} else posterior += -log2(0.05);
      }

      // check and record seen
      x = distance*c + particles[i].x;
      y = distance*s + particles[i].y;

      // make sure it is in bounds
      if (in_arena(x, y)) {
	k = buffer_index_from_x_y(x, y);
	p = landmark_seen_probability(particles[i].map, k);
	posterior += -log2(p);
      } else posterior += -log2(0.05);
    }

    particles[i].p += posterior;
    if (particles[i].p < min)
      min = particles[i].p;
  }

  // normalize particle log probabilities, convert to normal probabilities for resampling
  total = 0.0;
  for (i = 0; i < PARTICLE_COUNT; i++) {
    particles[i].p -= min;
    particles[i].p = pow(2, -particles[i].p);
    total += particles[i].p;
  }

  for (i = 0; i < PARTICLE_COUNT; i++) {
    particles[i].p /= total;
  }

  // bubblesort particles by p
  swap = 1;
  i = 0;
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

  /*
  for (i = 0; i < PARTICLE_COUNT; i++) {
    if (particles[i].p > 0)
      printf("i: %i, p: %g\n", i, particles[i].p);
  }
  */

  // save old particles before we resample
  memcpy(previous_particles, particles, sizeof(particle)*PARTICLE_COUNT);
  // resample with replacement
  p = rand()/(double)RAND_MAX;
  step = 1.0/PARTICLE_COUNT;
  for (i = 0; i < PARTICLE_COUNT; i++) {
    p += step;
    if (p > 1.0) p -= 1.0;
    total = 0.0;
    j = 0;
    while (j++ && total < p)
      total += previous_particles[j - 1].p;
    particles[i] = previous_particles[j - 1];
    particles[i].map = landmark_map_copy(particles[i].map);
  }

  // save best, copy the map we are about to dereference
  best_particle = previous_particles[0];
  best_particle.map = landmark_map_copy(best_particle.map);

  // dereference previous particle maps
  for (i = 0; i < PARTICLE_COUNT; i++)
    landmark_map_dereference(previous_particles[i].map);

  // restore log probabilities
  for (i = 0; i < PARTICLE_COUNT; i++)
    particles[i].p = -log2(particles[i].p);

  iterations++;
}

int swarm_get_best_x() {
  return best_particle.x;
}

int swarm_get_best_y() {
  return best_particle.y;
}

int swarm_get_best_theta() {
  return best_particle.theta;
}

void swarm_get_best_buffer(uint8_t *buffer) {
  landmark_write_map(best_particle.map, buffer);
}

int in_arena(int x, int y) {
  if (x >= 0 && x < long_side && y >= 0 && y < short_side)
    return 1;
  else return 0;
}
