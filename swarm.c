#include "swarm.h"

static swarm_member *queue;
static int swarm_size = 0;

void swarm_init() {
  queue = malloc(sizeof(swarm_member)*MAX_PARTICLES);
  swarm_reset();
}

void swarm_reset() {
  /*
  // grab sensor scan for initial scoring
  sensor_scan scan;
  sensor_scan average_scan;
  int i, j;
  for (i = 0; i < 10; i++) {
    scan = sensor_read();
    for (j = 0; j < SENSOR_DISTANCES; j++)
      average_scan.distances[j] = (average_scan.distances[j]*i + scan.distances[j])/(i + 1);
  }
  */
  swarm_size = 0;
  while (swarm_size < MAX_PARTICLES) {
    particle p = particle_init(400 + rand_limit(100), 400 + rand_limit(100),
			       -120 + rand_limit(60));
    //    if (!filter_particle(p, average_scan)) {
    //      printf("adding a paricle\n");
      swarm_add_particle(p);
      //    }
  }
}

void swarm_evaluate(sensor_scan scan) {
  swarm_member queue_copy[MAX_PARTICLES];
  memcpy(queue_copy, queue, sizeof(swarm_member)*swarm_size);
  int i, j;
  double sum;
  sensor_scan s;
  for (i = 0; i < swarm_size; i++) {
    sum = 0.0;
    s = sensor_distance(queue_copy[i].p);
    for (j = 0; j < SENSOR_DISTANCES; j++) {
      sum += pow(scan.distances[j] - s.distances[j], 2.0);
    }
    sum = sqrt(sum);
    swarm_particle_update_score(queue_copy[i].p, sum);
  }
}

int swarm_get_size() {
  return swarm_size;
}

void swarm_add_particle(particle p) {
  queue[swarm_size].p = p;
  swarm_size++;

  // bubble up starting at last element (which we just added)
  swarm_bubble_up(swarm_size - 1);
}

void swarm_bubble_up(int start) {
  int current = start;
  int parent = swarm_parent(current);
  swarm_member m;
  while (current > 0 && queue[current].p.score < queue[parent].p.score) {
    m = queue[current];
    queue[current] = queue[parent];
    queue[parent] = m;
    current = parent;
    parent = swarm_parent(current);
  }
}

void swarm_bubble_down(int start) {
  int current = start;
  int left = swarm_left_child(current);
  int right = swarm_right_child(current);
  int swap;
  swarm_member m;
  while (current < swarm_size &&
	 (queue[current].p.score > queue[left].p.score ||
	  queue[current].p.score > queue[right].p.score)) {
    if (queue[current].p.score > queue[left].p.score)
      swap = left;
    else swap = right;

    m = queue[current];
    queue[current] = queue[swap];
    queue[swap] = m;
    current = swap;
    left = swarm_left_child(current);
    right = swarm_right_child(current);
  }
}

static int swarm_parent(int index) {
  return (index - 1)/2;
}

static int swarm_left_child(int index) {
  return 2*index + 1;
}

static int swarm_right_child(int index) {
  return 2*(index + 1);
}

void swarm_move_particle(particle p, double x, double y, double theta) {
  int index = swarm_find_particle(p);
  if (index != -1) {
    queue[index].p.x = x;
    queue[index].p.y = y;
    queue[index].p.theta = theta;
  }
}

int swarm_find_particle(particle p) {
  int current = 0;
  int found = 0;
  while (current < swarm_size && !found) {
    if (p.score <= queue[current].p.score) {
      // match or left branch
      if (p.score == queue[current].p.score &&
	  p.x == queue[current].p.x &&
	  p.y == queue[current].p.y &&
	  p.theta == queue[current].p.theta &&
	  p.samples == queue[current].p.samples &&
	  p.score == queue[current].p.score) {
	// match
	found = 1;
      } else {
	// left
	current = swarm_left_child(current);
      }
    } else {
      // right branch
	current = swarm_right_child(current);
    }
  }

  if (found)
    return current;
  else return -1;
}

void swarm_particle_update_score(particle p, double score) {
  int index = swarm_find_particle(p);
  if (index != -1) {
    particle_add_sample(&(queue[index].p), score);
    if (score > p.score)
      // new value is larger, we may need to bubble down
      swarm_bubble_down(index);
    else
      // new value is smaller, we mad need to bubble up
      swarm_bubble_up(index);
  }
}

void swarm_all_particles(particle all_particles[MAX_PARTICLES]) {
  int i;
  for (i = 0; i < swarm_size; i++)
    all_particles[i] = queue[i].p;
}

void swarm_top_particles(particle top_particles[SWARM_TOP_COUNT]) {
  swarm_member queue_copy[swarm_size];
  memcpy(queue_copy, queue, sizeof(swarm_member)*swarm_size);

  int i;
  for (i = 0; i < SWARM_TOP_COUNT; i++) {
    top_particles[i] = swarm_pop_particle();
  }

  memcpy(queue, queue_copy, sizeof(swarm_member)*swarm_size);
}

particle swarm_pop_particle() {
  swarm_member m = queue[0];
  queue[0] = queue[swarm_size - 1];
  swarm_size--;
  swarm_bubble_down(0);
  return m.p;
}

void swarm_filter_particles(sensor_scan scan) {
  int i;
  for (i = 0; i < swarm_size; i++)
    if (filter_particle(queue[i].p, scan))
      swarm_delete_particle(queue[i].p);
}

void swarm_delete_particle(particle p) {
  int index = swarm_find_particle(p);
  if (index != -1) {
    swarm_member last = queue[swarm_size - 1];
    queue[index] = last;
    swarm_size--;
    if (last.p.score < p.score)
      swarm_bubble_up(index);
    else
      swarm_bubble_down(index);
  }
}

particle swarm_get_random_particle() {
  return queue[rand_limit(swarm_size)].p;
}
