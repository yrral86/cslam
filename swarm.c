#include "swarm.h"
#include <stdio.h>
#include "slamd.h"

static particle particles[INITIAL_PARTICLE_FACTOR*PARTICLE_COUNT];
static particle previous_particles[INITIAL_PARTICLE_FACTOR*PARTICLE_COUNT];
static particle best_particle;
static landmark_map *map;
static int iterations = 0;
static int converged, m, sensor_degrees, long_side, short_side, start;
static double spacing;
// 550 mm
static int sensor_radius;

#ifndef LINUX
__declspec(dllexport) void swarm_init(int m_in, int degrees_in, int long_side_in, int short_side_in, int start_in, int radius) {
  int param_size, return_size;

  m = m_in;
  sensor_degrees = degrees_in;
  long_side = long_side_in;
  short_side = short_side_in;
  start = start_in;
  spacing = sensor_degrees/(double)(m);
  sensor_radius = radius;

  // set up shared memory
  param_sem = CreateSemaphore(NULL, 0, 1, param_sem_name);
  return_sem = CreateSemaphore(NULL, 0, 1, return_sem_name);
  ready_sem = CreateSemaphore(NULL, 0, 1, ready_sem_name);
  assert(param_sem != NULL);
  assert(return_sem != NULL);
  assert(ready_sem != NULL);

  converged = 0;

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
  params[6] = radius;

  ReleaseSemaphore(param_sem, 1, NULL);
  WaitForSingleObject(return_sem, INFINITE);
}

__declspec(dllexport) void swarm_move(int dx, int dy, int dtheta) {
	params[0] = SLAMD_MOVE;
	params[1] = dx;
	params[2] = dy;
	params[3] = dtheta;

	ReleaseSemaphore(param_sem, 1, NULL);
	WaitForSingleObject(return_sem, INFINITE);
}

__declspec(dllexport) void swarm_update(int *distances) {
	params[0] = SLAMD_UPDATE;
	memcpy(params + 1, distances, m*sizeof(int));
	ReleaseSemaphore(param_sem, 1, NULL);
	WaitForSingleObject(return_sem, INFINITE);
}

__declspec(dllexport) void swarm_update_finalize() {
    assert(ready_sem != NULL);
	WaitForSingleObject(ready_sem, INFINITE);
}

__declspec(dllexport) void swarm_map(int *distances) {
	params[0] = SLAMD_MAP;
	memcpy(params + 1, distances, m*sizeof(int));
	ReleaseSemaphore(param_sem, 1, NULL);
	WaitForSingleObject(return_sem, INFINITE);
}

// returns 1 if standard deviations are low
// 0 otherwise 
__declspec(dllexport) int swarm_converged() {
	params[0] = SLAMD_CONVERGED;
	ReleaseSemaphore(param_sem, 1, NULL);
	WaitForSingleObject(return_sem, INFINITE);
	return *return_value;
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

#endif

#ifndef LINUX
void swarm_init_internal(int m_in, int degrees_in, int long_side_in, int short_side_in, int start_in, int radius) {
#endif
#ifdef LINUX
void swarm_init(int m_in, int degrees_in, int long_side_in, int short_side_in, int start_in, int radius) {
#endif
  int i, j, k;//, s;
  int x, y, theta;
  particle initial_map;
  //  char *str, *tok;
  //  FILE *map_file;
  double t;
  m = m_in;
  sensor_degrees = degrees_in;
  long_side = long_side_in;
  short_side = short_side_in;
  start = start_in;
  spacing = sensor_degrees/(double)(m);
  sensor_radius = radius;

  rand_normal_init();

  buffer_set_arena_size(long_side, short_side);

  initial_map.map = landmark_map_copy(NULL);
  map = landmark_map_copy(NULL);

  // draw initial border
  for (k = 0; k < long_side; k += BUFFER_FACTOR)
    for (j = 0; j < BORDER_WIDTH; j += BUFFER_FACTOR) {
      landmark_set_seen_value(initial_map.map, buffer_index_from_x_y(k, j), 10000);
      landmark_set_seen_value(initial_map.map,
			      buffer_index_from_x_y(k, short_side - 1 - j), 10000);
    }

  for (k = 0; k < short_side; k += BUFFER_FACTOR)
    for (j = 0; j < BORDER_WIDTH; j += BUFFER_FACTOR) {
            landmark_set_seen_value(initial_map.map, buffer_index_from_x_y(j, k), 10000);
            landmark_set_seen_value(initial_map.map,
      			      buffer_index_from_x_y(long_side - 1 - j, k), 10000);
    }
  /*
  // load map
  map_file = fopen("slamd_map.csv", "r");
  assert(map_file != NULL);

  s = buffer_get_size();
  // 22 = 10 characters for 32 unsigned bits * 2 + 2 commas
  str = malloc(sizeof(char)*s*22);

  fgets(str, sizeof(char)*s*22, map_file);

  fclose(map_file);

  i = 0;
  tok = strtok(str, ",");
  do {
    if (i % 2 == 0) {
      sscanf(tok, "%u", &(map->map[i/2].seen));
      //      initial_map.map->map[i/2].seen = map->map[i/2].seen;
    } else {
      sscanf(tok, "%u", &(map->map[i/2].unseen));
      //      initial_map.map->map[i/2].unseen = map->map[i/2].unseen;
    }
    i++;
  } while ((tok = strtok(NULL, ",")) != NULL);

  free(str);
  */
  // initialize first round of particles
  x = start/2;
  for (i = 0; i < INITIAL_PARTICLE_FACTOR*PARTICLE_COUNT; i++) {
    y = short_side/4;
    //    if (rand_limit(2))
    y *= 3;
    theta = rand_limit(360) - 180;
    //theta = 180;
    t = theta*M_PI/180;
    particles[i] = particle_init(x + sensor_radius*cos(t), y + sensor_radius*sin(t), theta);
    particles[i].map = initial_map.map;
    landmark_map_reference(particles[i].map);
  }

  best_particle = particles[0];

  landmark_map_dereference(initial_map.map);

#ifndef LINUX
  return_sem = CreateSemaphore(NULL, 0, 1, return_sem_name);
  ready_sem = CreateSemaphore(NULL, 0, 1, ready_sem_name);
  assert(return_sem != NULL);
  assert(ready_sem != NULL);
#endif
}

#ifndef LINUX
void swarm_move_internal(int dx, int dy, int dtheta) {
#endif
#ifdef LINUX
void swarm_move(int dx, int dy, int dtheta) {
#endif
  int i, tries, p_count;
  double t_old, t_new;
  particle p;
  t_old = best_particle.theta*M_PI/180;
  t_new = (best_particle.theta + dtheta)*M_PI/180;

  // adjust dx/dy for dtheta using radius and old theta
  dx += sensor_radius*(cos(t_new) - cos(t_old));
  dy += sensor_radius*(sin(t_new) - sin(t_old));

  if (iterations < 1)
    p_count = INITIAL_PARTICLE_FACTOR*PARTICLE_COUNT;
  else
    p_count = PARTICLE_COUNT;
	
  // reset convergence if we have a non 0 move
  if (abs(dx) > 0 || abs(dy) > 0 || abs(dtheta) > 0)
	converged = 0;

  // add motion
  for (i = 0; i < p_count; i++) {
    p = particles[i];
    tries = 0;
    do {
      // ignore kinematics 20% of the time
      if ((rand() / (double)RAND_MAX) < 0.8) {
        // sample motion distribution
        particles[i] = particle_sample_motion(p, dx, dy, dtheta);
      } else {
        // sample normal distribution
        particles[i] = particle_sample_normal(p);
      }
      // dereference old map, particle_sample_* copied it already
      landmark_map_dereference(p.map);
      tries++;
    } while (!in_arena(particles[i].x, particles[i].y) && tries < 5);
    // if we failed repeatedly, reuse previous pose
    if (tries >= 5)
      particles[i] = p;
  }
}

#ifndef LINUX
void swarm_update_internal(int *distances) {
#endif
#ifdef LINUX
void swarm_update(int *distances) {
#endif
  int i, j, k, l, cull, cut, cuts, cull_index, best_index, p_count;
  int particle_trig_initialized = 0;
  int swap, x, y, count;
  double mean[3], stddev[3];
  double distance, theta, degrees, s, c, total, min, p, step, p95, p5;
  double cos_d, sin_d, factor, theta_d;
  //  double xyt[3];
  particle temp;
  const double BIG = 10000000;

#ifndef LINUX
  ReleaseSemaphore(return_sem, 1, NULL);
#endif

  if (iterations < 1)
    p_count = INITIAL_PARTICLE_FACTOR*PARTICLE_COUNT;
  else
    p_count = PARTICLE_COUNT;

  p95 = -log(0.95);
  p5 = -log(0.05);
  factor = M_PI/180;
  cuts = m/CULLING_FACTOR + 1;
  cull_index = 0;
  particle_trig_initialized = 0;
  for (cull = 0; cull < CULLING_FACTOR; cull++) {
  // evaulate each direction for each particle
    for (cut = 0; cut < cuts; cut++) {
      j = cull + CULLING_FACTOR*cut;
      if (j >= m) {
	continue;
      }
      distance = distances[j];
      // skip any distances that are more than 8 meters in case we shoot over the walls
      if (distance < 8000) {
	degrees = -sensor_degrees/2.0 + j*spacing;
	theta_d = degrees*factor;
	sin_d = sin(theta_d);
	cos_d = cos(theta_d);

	// evaluate the particle's relative probability
	// skip culled particles
	min = BIG;
	best_index = 0;
	for (i = cull_index; i < p_count; i++) {
	  if (particle_trig_initialized == 0) {
	    theta_d = particles[i].theta * factor;
	    particles[i].sin = sin(theta_d);
	    particles[i].cos = cos(theta_d);
	  }
	  // forward is now 0 degrees, left -, right +
	  //theta = (degrees - particles[i].theta)*M_PI/180;
	  // sin(A - B) = sin(A)*cos(B) - sin(B)*cos(A)
	  s = sin_d * particles[i].cos - particles[i].sin * cos_d;
	  // cos(A -B) = cos(A)*cos(B) + sin(A)*sin(B)
	  c = cos_d * particles[i].cos + sin_d * particles[i].sin;

	  // check and record unseen every 200 mm, within 1 meter of obstacle
	  for (l = distance - 200; l > distance - 1000; l -= 200) {
	    x = l*c + particles[i].x;
	    y = l*s + short_side - particles[i].y;

	    // make sure it is in bounds
	    if (in_arena(x, y)) {
	      k = buffer_index_from_x_y(x, y);
	      p = landmark_unseen_probability(particles[i].map, k);
	      if (p == 0.05) {
		particles[i].p += p5;
	      } else {
		particles[i].p += p95;
	      }
	    } else particles[i].p += p5;
	  }

	  // check and record seen
	  x = distance*c + particles[i].x;
	  y = distance*s + short_side - particles[i].y;

	  // make sure it is in bounds
	  if (in_arena(x, y)) {
	    k = buffer_index_from_x_y(x, y);
	    p = landmark_seen_probability(particles[i].map, k);
	    if (p == 0.05) {
	      particles[i].p += p5;
	    } else {
	      particles[i].p += p95;
	    }
	  } else particles[i].p += p5;

	  if (particles[i].p < min) {
	    min = particles[i].p;
	    best_index = i;
	  }
	}
	particle_trig_initialized = 1;
      }
    }


    if (cull != CULLING_FACTOR - 1) {
      // bubblesort particles by p
      swap = 1;
      i = 0;
      do {
	swap = 0;
	for (j = 0; j < p_count - i - 1; j++)
	  // if the left particle is smaller probability, bubble it right
	  if (particles[j].p < particles[j + 1].p) {
	    // maintain best_index
	    if (best_index == j)
	      best_index = j + 1;
	    else if (best_index == j + 1)
	      best_index = j;
	    temp = particles[j];
	    particles[j] = particles[j + 1];
	    particles[j + 1] = temp;
	    swap = 1;
	  }
	i++;
      } while (swap);

      // cull bottom
      cull_index = CULLING_PERCENT*p_count*(cull + 1.0)/CULLING_FACTOR;
      for (i = 0; i < cull_index; i++) {
	particles[i].p = BIG;
      }
      //    printf("cull: %d, p_count: %d, cull_index: %d\n", cull, p_count, cull_index);
    }

    /*
    // cull particles more than 10x the best
    double cutoff = 10*min;
    for (i = 0; i < p_count; i++) {
      if (particles[i].p > cutoff) {
	particles[i].p = BIG;
      }
      }*/
  }

  // clear old best, save new best, copy the map we are about to dereference
  if (iterations > 0)
    landmark_map_dereference(best_particle.map);
  best_particle = particles[best_index];
  best_particle.map = landmark_map_copy(best_particle.map);

  // normalize particle log probabilities, convert to normal probabilities for resampling
  total = 0.0;
  for (i = 0; i < p_count; i++) {
    particles[i].p -= min;
    particles[i].p = pow(M_E, -particles[i].p);
    total += particles[i].p;
  }

  for (i = 0; i < p_count; i++) {
    particles[i].p /= total;
  }

  // bubblesort particles by p
  swap = 1;
  i = 0;
  do {
    swap = 0;
    for (j = 0; j < p_count - i - 1; j++)
      // if the left particle is larger probability, bubble it right
      if (particles[j].p > particles[j + 1].p) {
	temp = particles[j];
	particles[j] = particles[j + 1];
	particles[j + 1] = temp;
	swap = 1;
      }
    i++;
  } while (swap);

  // calculate standard deviation of top 90%
  i = p_count - 1;
  total = 0.0;
  mean[0] = 0.0;
  mean[1] = 0.0;
  mean[2] = 0.0;
  count = 0;
  while (total < 0.99) {
    mean[0] += particles[i].x;
    mean[1] += particles[i].y;
    mean[2] += particles[i].theta;
    count++;
    total += particles[i].p;
    i--;
  }

  for (i = 0; i < 3; i++)
    mean[i] /= count;

  stddev[0] = 0.0;
  stddev[1] = 0.0;
  stddev[2] = 0.0;
  total = 0.0;
  i = p_count - 1;
  while (total < 0.99) {
    stddev[0] += pow(mean[0] - particles[i].x, 2);
    stddev[1] += pow(mean[1] - particles[i].y, 2);
    stddev[2] += pow(mean[2] - particles[i].theta, 2);
    total += particles[i].p;
    i--;
  }

  for (i = 0; i < 3; i++) {
    if (count > 1)
      stddev[i] /= count - 1;
    else
      stddev[i] /= count;
    stddev[i] = sqrt(stddev[i]);
  }

  // make sure 99% of particles are within 50mm and 1 degree
  if (sqrt(pow(stddev[0], 2) + pow(stddev[1], 2)) < 50 && stddev[2] < 2)
    converged = 1;
  else
    converged = 0;

  if (!converged || count > 1)
    printf("count: %d, max prob: %g, standard deviations: %g, %g, %g, converged: %d\n", count, particles[p_count - 1].p, stddev[0], stddev[1], stddev[2], converged);

  // save old particles before we resample
  memcpy(previous_particles, particles, sizeof(particle)*p_count);
  // resample with replacement
  p = rand()/(double)RAND_MAX;
  step = 1.0/p_count;
  for (i = 0; i < p_count; i++) {
    p += step;
    if (p > 1.0) p -= 1.0;
    total = 0.0;
    j = p_count - 1;
    while (j-- && total < p)
      total += previous_particles[j + 1].p;
    particles[i] = previous_particles[j + 1];
    particles[i].map = landmark_map_copy(particles[i].map);
  }

  // dereference previous particle maps
  for (i = 0; i < p_count; i++)
    landmark_map_dereference(previous_particles[i].map);

  // restore log probabilities
  for (i = 0; i < p_count; i++)
    particles[i].p = -log(particles[i].p);

  iterations++;

#ifndef LINUX
  ReleaseSemaphore(ready_sem, 1, NULL);
#endif
}

#ifndef LINUX
void swarm_map_internal(int *distances) {
#endif
#ifdef LINUX
void swarm_map(int *distances) {
#endif
  int j, l, x, y, k;
  double distance, degrees, theta, s, c;

  for (j = 0; j < m; j++) {
    distance = distances[j];
    // forward is now 0 degrees, left -, right +
    degrees = -sensor_degrees/2.0 + j*spacing;
    theta = (degrees - best_particle.theta)*M_PI/180;
    s = sin(theta);
    c = cos(theta);

    // check and record unseen every 10 mm
    for (l = 0; l < distance; l += 10) {
      x = l*c + best_particle.x;
      y = l*s + short_side - best_particle.y;

      // make sure it is in bounds
      if (in_arena(x, y)) {
	k = buffer_index_from_x_y(x, y);
	landmark_set_unseen(map, k);
      }
    }
    
    // check and record seen
    x = distance*c + best_particle.x;
    y = distance*s + short_side - best_particle.y;

    // make sure it is in bounds
    if (in_arena(x, y)) {
      k = buffer_index_from_x_y(x, y);
      landmark_set_seen(map, k);
    }
  }
}

#ifndef LINUX
int swarm_converged_internal() {
#endif
#ifdef LINUX
int swarm_converged() {
#endif
  return converged;
}

#ifndef LINUX
int swarm_get_best_x_internal() {
#endif
#ifdef LINUX
int swarm_get_best_x() {
#endif
  return best_particle.x - sensor_radius*cos(best_particle.theta*M_PI/180);
}

#ifndef LINUX
int swarm_get_best_y_internal() {
#endif
#ifdef LINUX
int swarm_get_best_y() {
#endif
  return short_side - (best_particle.y - sensor_radius*sin(best_particle.theta*M_PI/180));
}

#ifndef LINUX
int swarm_get_best_theta_internal() {
#endif
#ifdef LINUX
int swarm_get_best_theta() {
#endif
  int t =-1*(best_particle.theta + 180) % 360;
  if (t <= -180) t += 360;
  else if (t > 180) t -= 360;
  return t;
}

int swarm_get_x(int i) {
  return particles[i].x - sensor_radius*cos(particles[i].theta*M_PI/180) - BORDER_WIDTH;
}

int swarm_get_y(int i) {
  return short_side - (particles[i].y - sensor_radius*sin(particles[i].theta*M_PI/180)) - BORDER_WIDTH;
}

int swarm_get_theta(int i) {
  return -1*(particles[i].theta + 180) % 360;
}

void swarm_get_best_buffer(uint8_t *buffer) {
  landmark_write_map(best_particle.map, buffer);
}

void swarm_get_map_buffer(uint8_t *buffer) {
  landmark_write_map(map, buffer);
}

landmark_map swarm_get_map() {
  return *map;
}

void swarm_get_all_particles(particle **p) {
  *p = particles;
}

int in_arena(int x, int y) {
  if (x >= 0 && x < long_side && y >= 0 && y < short_side)
    return 1;
  else return 0;
}
