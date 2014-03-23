#include "swarm.h"
#include <stdio.h>
#include "slamd.h"

static particle particles[PARTICLE_COUNT];
static particle previous_particles[PARTICLE_COUNT];
static particle best_particle;
static int iterations = 0;
static int m, sensor_degrees, long_side, short_side, start;
static double spacing;

__declspec(dllexport) void swarm_init(int m_in, int degrees_in, int long_side_in, int short_side_in, int start_in) {
  int param_size, return_size;
  LPWSTR command = L"Slamd.exe";
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  m = m_in;
  sensor_degrees = degrees_in;
  long_side = long_side_in;
  short_side = short_side_in;
  start = start_in;
  spacing = sensor_degrees/(double)(m);

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  // set up shared memory
  param_sem = CreateSemaphore(NULL, 0, 1, param_sem_name);
  return_sem = CreateSemaphore(NULL, 0, 1, return_sem_name);
  assert(param_sem != NULL);
  assert(return_sem != NULL);

  // parameter space size is (sensor readings + 1) * sizeof(int)
  // first int specifies which function, remainder are params
  param_size = (m_in + 1)*sizeof(int);
  param_handle = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, param_size, param_shm_name);
  assert(param_handle != NULL);

  // return space size is sizeof(int)
  return_size = sizeof(int);
  return_handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, return_size, return_shm_name);
  assert(return_handle != NULL);

  params = (int*)MapViewOfFile(param_handle, FILE_MAP_WRITE, 0, 0, param_size);
  assert(params != NULL);

  return_value = (int*)MapViewOfFile(return_handle, FILE_MAP_WRITE, 0, 0, return_size);
  assert(return_value != NULL);

  params[0] = SLAMD_INIT;
  params[1] = m_in;
  params[2] = degrees_in;
  params[3] = long_side_in;
  params[4] = short_side_in;
  params[5] = start_in;  

  //CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

  ReleaseSemaphore(param_sem, 1, NULL);
}

__declspec(dllexport) void swarm_move(int dx, int dy, int dtheta) {
	FILE *debug_file = fopen("debug_swarm_move.txt", "w");
	params[0] = SLAMD_MOVE;
	params[1] = dx;
	params[2] = dy;
	params[3] = dtheta;
	fprintf(debug_file, "before shm: %i, %i, %i\n", params[1], params[2], params[3]);
	fclose(debug_file);
	ReleaseSemaphore(param_sem, 1, NULL);
	WaitForSingleObject(return_sem, INFINITE);
}

__declspec(dllexport) void swarm_update(int *distances) {
	params[0] = SLAMD_UPDATE;
	memcpy(params + 1, distances, m*sizeof(int));
	ReleaseSemaphore(param_sem, 1, NULL);
	WaitForSingleObject(return_sem, INFINITE);
}

__declspec(dllexport) int swarm_get_best_x() {
	params[0] = SLAMD_X;
	ReleaseSemaphore(param_sem, 1, NULL);
	WaitForSingleObject(return_sem, INFINITE);
	return *return_value;
}

__declspec(dllexport) int swarm_get_best_y() {
	params[0] = SLAMD_Y;
	ReleaseSemaphore(param_sem, 1, NULL);
	WaitForSingleObject(return_sem, INFINITE);
	return *return_value;
}

__declspec(dllexport) int swarm_get_best_theta() {
	params[0] = SLAMD_THETA;
	ReleaseSemaphore(param_sem, 1, NULL);
	WaitForSingleObject(return_sem, INFINITE);
	return *return_value;
}

/*
// TODO: _ETH
double K[3*RAW_SENSOR_DISTANCES_USB], H[RAW_SENSOR_DISTANCES_USB*3], P[9], PH[3*RAW_SENSOR_DISTANCES_USB], HPH[RAW_SENSOR_DISTANCES_USB*RAW_SENSOR_DISTANCES_USB];
// 1% of measurement, avereage around 40 mm
double R = 40;
// TODO: VRV(T) to scale R based on distances
*/
void swarm_init_internal(int m_in, int degrees_in, int long_side_in, int short_side_in, int start_in) {
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

void swarm_move_internal(int dx, int dy, int dtheta) {
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

void swarm_update_internal(int *distances) {
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
	  posterior += -log(p);
	} else posterior += -log(0.05);
      }

      // check and record seen
      x = distance*c + particles[i].x;
      y = distance*s + particles[i].y;

      // make sure it is in bounds
      if (in_arena(x, y)) {
	k = buffer_index_from_x_y(x, y);
	p = landmark_seen_probability(particles[i].map, k);
	posterior += -log(p);
      } else posterior += -log(0.05);
    }

    particles[i].p += posterior;
    if (particles[i].p < min)
      min = particles[i].p;
  }

  // normalize particle log probabilities, convert to normal probabilities for resampling
  total = 0.0;
  for (i = 0; i < PARTICLE_COUNT; i++) {
    particles[i].p -= min;
    particles[i].p = pow(M_E, -particles[i].p);
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

  // clear old best, save new best, copy the map we are about to dereference
  if (iterations > 0)
    landmark_map_dereference(best_particle.map);
  best_particle = previous_particles[0];
  best_particle.map = landmark_map_copy(best_particle.map);

  // dereference previous particle maps
  for (i = 0; i < PARTICLE_COUNT; i++)
    landmark_map_dereference(previous_particles[i].map);

  // restore log probabilities
  for (i = 0; i < PARTICLE_COUNT; i++)
    particles[i].p = -log(particles[i].p);

  iterations++;
}

int swarm_get_best_x_internal() {
  return best_particle.x;
}

int swarm_get_best_y_internal() {
  return best_particle.y;
}

int swarm_get_best_theta_internal() {
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
