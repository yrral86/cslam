#include "swarm.h"

static particle particles[PARTICLE_COUNT];
static particle previous_particles[PARTICLE_COUNT];
static particle best_particle;
static int iterations = 0;
static int m, degrees, long_side, short_side, start;
static double spacing;

/*
// TODO: _ETH
double K[3*RAW_SENSOR_DISTANCES_USB], H[RAW_SENSOR_DISTANCES_USB*3], P[9], PH[3*RAW_SENSOR_DISTANCES_USB], HPH[RAW_SENSOR_DISTANCES_USB*RAW_SENSOR_DISTANCES_USB];
// 1% of measurement, avereage around 40 mm
double R = 40;
// TODO: VRV(T) to scale R based on distances
*/
void swarm_init(int m_in, int degrees_in, int long_side_in, int short_side_in, int start_in) {
  int i, j, k, x, y, theta;
  m = m_in;
  degrees = degrees_in;
  long_side = long_side_in;
  short_side = short_side_in;
  start = start_in;
  spacing = degrees/(double)(m);

  // initialize first round of particles
  for (i = 0; i < PARTICLE_COUNT; i++) {
    x = START_END/2;
    y = short_side/4;
    if (rand_limit(2))
      y *= 3;
    theta = rand_limit(360) - 180;
    particles[i] = particle_init(x, y, theta);

    particles[i].map;

    // draw initial border
    for (k = 0; k < long_side; k++)
      for (j = 0; j < BORDER_WIDTH*BUFFER_FACTOR; j += BUFFER_FACTOR) {
	landmark_set_seen_value(particles[i].map, buffer_index_from_x_y(k, j), 10000);
	landmark_set_seen_value(particles[i].map,
				buffer_index_from_x_y(k, short_side - 1 - j), 10000);
      }

    for (k = 0; k < short_side; k++)
      for (j = 0; j < BORDER_WIDTH*BUFFER_FACTOR; j+= BUFFER_FACTOR) {
	landmark_set_seen_value(particles[i].map, buffer_index_from_x_y(j, k), 10000);
	landmark_set_seen_value(particles[i].map,
				buffer_index_from_x_y(long_side - 1 - j, k), 10000);
      }
  }
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
    landmark_map_free(p.map);
  }
}

void swarm_update(int *distances) {
  int i, j, k, l, m;
  int swap;
  double posterior, distance, degrees, theta, x, y, s, c, total;
  double xyt[3];

  total = 0.0;
  // evaulate each direction for each particle
  for (i = 0; i < PARTICLE_COUNT; i++) {
    posterior = 1.0;

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
    // TODO: _ETH
    for (j = 0; j < m; j++) {
      distance = distances[j];
      // forward is now 0 degrees, left -, right +
      // TODO: same as above (_ETH)
      degrees = -120 + j*spacing;
      theta = (degrees + particles[i].theta)*M_PI/180;
      s = sin(theta);
      c = cos(theta);

      // check and record unseen every 10 mm
      for (l = 0; l < distance; l += 10) {
	x = l*c + particles[i].x;
	y = l*s + particles[i].y;

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
    particles[i].map = landmark_map_copy(particles[i].map);
  }

  // save best, copy the map we are about to dereference
  best_particle = previous_particles[0];
  best_particle.map = landmark_map_copy(best_particle.map);

  // dereference previous particle maps
  for (i = 0; i < PARTICLE_COUNT; i++) {
    landmark_map_free(previous_particles[i].map);
  }

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
