#include "swarm.h"
#include <stdio.h>
#include "slamd.h"

static particle particles[PARTICLE_COUNT];
static particle previous_particles[PARTICLE_COUNT];
static particle best_particle;
//static landmark_map *map;
static uint8_t *map;
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

#ifdef LINUX
void swarm_set_initial_hypothesis(hypothesis *h) {
  best_particle.h = h;
}
#endif

#ifndef LINUX
void swarm_init_internal(int m_in, int degrees_in, int long_side_in, int short_side_in, int start_in, int radius) {
#endif
#ifdef LINUX
void swarm_init(int m_in, int degrees_in, int long_side_in, int short_side_in, int start_in, int radius) {
#endif
  int i;//, j, k, s;
  int x, y, theta;
  //  particle initial_map;
  //  char *str, *tok;
  //  FILE *map_file;
  //  double t;
  m = m_in;
  sensor_degrees = degrees_in;
  long_side = long_side_in;
  short_side = short_side_in;
  start = start_in;
  spacing = sensor_degrees/(double)(m);
  sensor_radius = radius;

  rand_normal_init();

  buffer_set_arena_size(long_side, short_side);

  /*  initial_map.map = landmark_map_copy(NULL);
  map = landmark_map_copy(NULL);
  */

  // draw initial border
  /*  for (k = 0; k < long_side; k += BUFFER_FACTOR)
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
  */


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
  //x = start/2;
  x = long_side/2;
  y = short_side/2;
  theta = 0;
  //  t = theta*M_PI/180;
  for (i = 0; i < PARTICLE_COUNT; i++) {
    //y = short_side/4;

    //    if (rand_limit(2))
    //y *= 3;
    //    theta = rand_limit(360) - 180;
    //    particles[i] = particle_init(x + sensor_radius*cos(t), y + sensor_radius*sin(t), theta);
    particles[i] = particle_init(x, y, theta);
    particles[i].h = best_particle.h;
    //    printf("referencing best_particle.h from init\n");
    hypothesis_reference(best_particle.h);

    //    particles[i].map = initial_map.map;
    //    landmark_map_reference(particles[i].map);
  }

  //  printf("dereferencing best_particle.h from init\n");
  hypothesis_dereference(best_particle.h);

  best_particle = particles[0];

  //  landmark_map_dereference(initial_map.map);

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
  //  double t_old, t_new;
  particle p;
  hypothesis *h;
  /*  t_old = best_particle.theta*M_PI/180;
  t_new = (best_particle.theta + dtheta)*M_PI/180;

  // adjust dx/dy for dtheta using radius and old theta
  dx += sensor_radius*(cos(t_new) - cos(t_old));
  dy += sensor_radius*(sin(t_new) - sin(t_old));
  */
  p_count = PARTICLE_COUNT;

  // add motion
  for (i = 0; i < p_count; i++) {
    p = particles[i];
    //    printf("swarm move: before h = %p, h->parent = %p\n", p.h, p.h->parent);
    tries = 0;
    do {
      // ignore kinematics 20% of the time
      //      if ((rand() / (double)RAND_MAX) < 0.8) {
        // sample motion distribution
      //        particles[i] = particle_sample_motion(p, dx, dy, dtheta);
      //      } else {
        // sample normal distribution
      particles[i] = particle_sample_normal(p, iterations);
	//      }
      // dereference old map, particle_sample_* copied it already
      //      landmark_map_dereference(p.map);
      tries++;
    } while (!in_arena(particles[i].x, particles[i].y) && tries < 5);
    // if we failed repeatedly, reuse previous pose
    // (plus stay put 20% of time)
    if (tries >= 5  || rand_limit(100) < 20)
      particles[i] = p;
    else {
      // we succeeded in sampling, change hypothesis
      p = particles[i];
      if (abs(p.h->x - p.x) > 0 ||
	  abs(p.h->y - p.y) > 0 ||
	  abs(p.h->theta - p.theta) > 0) {
	// generate new hypothesis
	h = p.h;
	//	printf("hypothesis new in swarm move\n");
	p.h = hypothesis_new(h, p.x, p.y, p.theta);
	// dereference, but p.h has a new refrence to it
	//	printf("dereferencing parent hypothesis, we should have just referenced it as the parent\n");
	hypothesis_dereference(h);

	// put back in particles
	particles[i].h = p.h;
      }
    }

    //    printf("swarm move: after h = %p, h->parent = %p\n", particles[i].h, particles[i].h->parent);

    /*
    printf("move: (%d, %d, %d)\n", particles[i].x,
	   particles[i].y,
	   particles[i].theta);

    printf("move parent: (%d, %d, %d)\n", p.x,
	   p.y,
	   p.theta);*/
  }
}

void swarm_reset_convergence() {
  converged = 0;
  iterations = 0;
}

#ifndef LINUX
void swarm_update_internal(int *distances) {
#endif
#ifdef LINUX
void swarm_update(observations *obs) {
  hypothesis *temp_h;
  map_node *temp_map, *parent_map;
  int offset;
#endif
  int i, last_j, j, best_index, p_count, count;
  //  int swap, x, y, count;
  double mean[3], stddev[3];
  //  double posterior, distance, theta, degrees, s, c, total, max, p, step;
  double total, max, p, step;
  //  double xyt[3];
  particle temp;
  //  double weight;

#ifndef LINUX
  ReleaseSemaphore(return_sem, 1, NULL);
#endif

  p_count = PARTICLE_COUNT;
  int culled[p_count];
  int culled_count = 0;
  for (i = 0; i < p_count; i++)
    culled[i] = 0;

  if (obs->hypotheses == NULL)
    obs->hypotheses = malloc(sizeof(hypothesis*)*p_count);

  //  min = 10000.0;
  max = 0.0;
  best_index = 0;
  // use the same observations for all particles
  //  offset = rand_limit(20);
  for (offset = 0; offset < 20; offset++) {
    // evaulate each direction for each particle
    for (i = 0; i < p_count; i++) {
      if (culled[i] == 0) {
	temp_h = particles[i].h;
	particles[i].h->obs = obs;
	particles[i].p *= 1.0/(buffer_hypothesis_distance(temp_h, offset, 20)*temp_h->map->current_size*temp_h->map->current_size);

	if (particles[i].p > max) {
	  max = particles[i].p;
	  best_index = i;
	}
      }
    }

    // cull terrible particles
    swarm_normalize();
    for (i = 0; i < p_count; i++)
      // stop if we've culled more than 90% of particles
      if (culled[i] == 0 && culled_count < 0.9*p_count)
	if (particles[i].p < 1.0/p_count) {
	  culled[i] = 1;
	  particles[i].p = 0;
	  culled_count++;
	}// else
    //	  printf("score: %g\n", particles[i].p);
  }

  // clear old best, save new best, copy the map we are about to dereference
  //  if (iterations > 0)
  //    landmark_map_dereference(best_particle.map);
  best_particle = particles[best_index];
  //  best_particle.map = landmark_map_copy(best_particle.map);
  //  best_particle.map = landmark_map_copy(best_particle.map);

  //  swarm_map_current(distances);

  // normalize particles
  swarm_normalize();

  printf("culled %d of %d particles\n", culled_count, p_count);

  swarm_sort(0, p_count - culled_count - 1);

  // calculate standard deviation of top 99%
  i = 0;
  total = 0.0;
  mean[0] = 0.0;
  mean[1] = 0.0;
  mean[2] = 0.0;
  count = 0;
  while (total < 0.99 && i < p_count - culled_count) {
    mean[0] += particles[i].x;
    mean[1] += particles[i].y;
    mean[2] += particles[i].theta;
    count++;
    total += particles[i].p;
    i++;
  }

  for (i = 0; i < 3; i++)
    mean[i] /= count;

  stddev[0] = 0.0;
  stddev[1] = 0.0;
  stddev[2] = 0.0;
  total = 0.0;
  i = 0;
  while (total < 0.99 && i < p_count - culled_count) {
    stddev[0] += pow(mean[0] - particles[i].x, 2);
    stddev[1] += pow(mean[1] - particles[i].y, 2);
    stddev[2] += pow(mean[2] - particles[i].theta, 2);
    total += particles[i].p;
    i++;
  }

  for (i = 0; i < 3; i++) {
    if (count > 1)
      stddev[i] /= count - 1;
    else
      stddev[i] /= count;
    if (i == 2)
      stddev[i] = sqrt(stddev[i]);
  }

  // make sure 99% of particles are within 75mm and 5 degrees
  if (stddev[0] + stddev[1] < INITIAL_POSITION_VARIANCE*INITIAL_POSITION_VARIANCE &&
      stddev[2] < INITIAL_ANGLE_VARIANCE)
    converged = 1;
  else
    converged = 0;

  if (!converged || count > 1)
    printf("count: %d, max prob: %g, standard deviations: %g, %g, %g, converged: %d\n", count, particles[0].p, stddev[0], stddev[1], stddev[2], converged);

  // save old particles before we resample
  memcpy(previous_particles, particles, sizeof(particle)*p_count);
  // resample with replacement
  p = rand()/(double)RAND_MAX;
  step = 1.0/p_count;
  last_j = -1;
  for (i = 0; i < p_count; i++) {
    p += step;
    if (p > 1.0) p -= 1.0;
    total = 0.0;
    j = 0;
    while (j++ && total < p)
      total += previous_particles[j - 1].p;
    j--;
    //    printf("resampling: j = %d\n", j);
    temp = previous_particles[j];
    if (last_j < j) {
      if (last_j != -1)
	// restore hypothesis for dereferencing below
	previous_particles[last_j].h = temp_h;
      // save next hypothesis
      temp_h = temp.h;
      last_j = j;
    }
    if (temp.resampled == 0) {
      // first resample
      // get map from mask and hypothesis
      //      printf("resample get mask\n");
      temp_map = map_get_shifted_mask(temp.x, temp.y);
      // no dereferences, copies map from parent in masked region
      //      printf("resample intersection\n");
      parent_map = map_intersection(temp_map, temp.h->map);
      // dereferences mask
      //      printf("resample get map from mask and hypothesis\n");
      temp_map = map_from_mask_and_hypothesis(temp_map, temp.h);

/*      printf("dereference map copied from parent\n");
      // dereference parent map
      map_dereference(temp.h->map);
*/
      // merge map with map copied from parent
      // this will dereference parent map and temp_map
      //      printf("map merge parent map and map from hypotheis\n");
      temp.h->map = map_merge(parent_map, temp_map);

      // write buffer
      map_write_buffer(temp.h->map);

      // reference map from particle we will sample,
      // original map will be restored
      previous_particles[j].h->map = temp.h->map;
      previous_particles[j].resampled = 1;
      hypothesis_reference(particles[j].h);
    } else {
      // we've already resampled, just increase the reference count
      //      printf("referencing hypothesis in resample\n");
      hypothesis_reference(particles[j].h);
    }
    particles[i] = previous_particles[j];

    // add hypothesis to observations
    obs->hypotheses[i] = particles[i].h;

    //    printf("resample: (%g, %g, %g), i = %d, j = %d\n", particles[i].x,
    //    	   particles[i].y, particles[i].theta, i , j);
  }

  previous_particles[last_j].h = temp_h;

  // dereference previous hypotheses
  //  printf("dereference previous hypotheses\n");
  for (i = 0; i < p_count; i++)
    hypothesis_dereference(previous_particles[i].h);

  // restore log probabilities
  //  for (i = 0; i < p_count; i++)
  //    particles[i].p = -log(particles[i].p);

  iterations++;

#ifndef LINUX
  ReleaseSemaphore(ready_sem, 1, NULL);
#endif
}

#ifndef LINUX
void swarm_map_internal(int *distances) {
  /*#endif
#ifdef LINUX
void swarm_map(int *distances) {
#endif*/
  int j, l, x, y, k;
  double distance, degrees, theta, s, c;

  for (j = 0; j < m; j++) {
    distance = distances[j];
    // forward is now 0 degrees, left -, right +
    degrees = -sensor_degrees/2.0 + j*spacing;
    theta = (degrees - best_particle.theta)*M_PI/180;
    s = sin(theta);
    c = cos(theta);

    // check and record unseen every BUFFER_FACTOR mm
    for (l = distance - BUFFER_FACTOR; l > 0; l -= BUFFER_FACTOR) {
      x = l*c + best_particle.x;
      y = l*s + short_side - best_particle.y;

      // make sure it is in bounds
      if (in_arena(x, y)) {
	k = buffer_index_from_x_y(x, y);
	//	landmark_set_unseen(map, k);
      }
    }
    
    // check and record seen
    x = distance*c + best_particle.x;
    y = distance*s + short_side - best_particle.y;

    // make sure it is in bounds
    if (in_arena(x, y)) {
      k = buffer_index_from_x_y(x, y);
      //      landmark_set_seen(map, k);
    }
  }
}

//#ifdef LINUX
void swarm_map_reset_current() {
  // blank map
  //  landmark_map_dereference(best_particle.map);
  //  best_particle.map = landmark_map_copy(NULL);
}
 
void swarm_map_current(int *distances) {
  int j, l, x, y, k;
  double distance, degrees, theta, s, c;

  for (j = 0; j < m; j++) {
    distance = distances[j];
    // forward is now 0 degrees, left -, right +
    degrees = -sensor_degrees/2.0 + j*spacing;
    theta = (degrees - best_particle.theta)*M_PI/180;
    s = sin(theta);
    c = cos(theta);

    
    // check and record unseen every BUFFER_FACTOR mm
    for (l = distance - BUFFER_FACTOR; l > 0;  l -= BUFFER_FACTOR) {
      x = l*c + best_particle.x;
      y = l*s + short_side - best_particle.y;

      // make sure it is in bounds
      if (in_arena(x, y)) {
	k = buffer_index_from_x_y(x, y);
	//	landmark_set_unseen(best_particle.map, k);
      }
    }
    
    // check and record seen
    x = distance*c + best_particle.x;
    y = distance*s + short_side - best_particle.y;

    // make sure it is in bounds
    if (in_arena(x, y)) {
      k = buffer_index_from_x_y(x, y);
      //      landmark_set_seen(best_particle.map, k);
    }
  }
}
#endif

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
double swarm_get_best_x() {
#endif
  return best_particle.x - sensor_radius*cos(best_particle.theta*M_PI/180);
}

#ifndef LINUX
int swarm_get_best_y_internal() {
#endif
#ifdef LINUX
double swarm_get_best_y() {
#endif
  return best_particle.y - sensor_radius*sin(best_particle.theta*M_PI/180);
}

#ifndef LINUX
int swarm_get_best_theta_internal() {
#endif
#ifdef LINUX
double swarm_get_best_theta() {
#endif
  double r = best_particle.theta - (int)best_particle.theta;
  double t = (int)best_particle.theta % 360;
  if (t <= -180) t += 360;
  else if (t > 180) t -= 360;
  return t + r;
}

#ifndef LINUX
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
  //  landmark_write_map(best_particle.map, buffer);
}

void swarm_get_map_buffer(uint8_t *buffer) {
  //  landmark_write_map(map, buffer);
}

map_node* swarm_get_map() {
  return map;
}

void swarm_get_all_particles(particle **p) {
  *p = particles;
}
#endif

int in_arena(int x, int y) {
  if (x >= 0 && x < long_side && y >= 0 && y < short_side)
    return 1;
  else return 0;
}

void swarm_set_map(uint8_t *new_map) {
  map = new_map;
}

void swarm_normalize() {
  int i;
  double total = 0.0;
  //  printf("swarm_normalize:\n");
  for (i = 0; i < PARTICLE_COUNT; i++) {
  //    printf("%d: %g\n", i, particles[i].p);
    total += particles[i].p;
  }

  // if scores are all 0, avoid a NaN
  if (total == 0) total = 1;

  for (i = 0; i < PARTICLE_COUNT; i++) {
    particles[i].p /= total;
  }
}

void swarm_sort(int left, int right) {
  int pivot;
  if (left < right) {
    pivot = swarm_partition(left, right);
    swarm_sort(left, pivot - 1);
    swarm_sort(pivot + 1, right);
  }
}

int swarm_partition(int left, int right) {
  int i, pivot_index = (right + left)/2;
  particle p, pivot = particles[pivot_index];
  int index = left;

  p = particles[right];
  particles[right] = pivot;
  particles[pivot_index] = p;

  for (i = left; i <= right; i++)
    if (particles[i].p > pivot.p) {
      p = particles[i];
      particles[i] = particles[index];
      particles[index] = p;
      index++;
    }

  p = particles[index];
  particles[index] = particles[right];
  particles[right] = p;

  return index;
}
