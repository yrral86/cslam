#include "simulation.h"

//static particle *particles;
static particle *obstacles;/*
static particle rescue;
static int particle_count;*/
static ClutterColor *particle_color;
static ClutterActor *stage;
static int success_count, failure_count;

int main (int argc, char **argv) {
  //  particles = malloc(sizeof(particle)*MAX_PARTICLES);
  obstacles = malloc(sizeof(particle)*OBSTACLE_COUNT);

  success_count = 0;
  failure_count = 0;

  sensor_init();

  srand(time(NULL));

  robot_set_mode(ROBOT_INITIAL);

  swarm_init();

  ClutterInitError e = clutter_init(&argc, &argv);

  ClutterColor stage_color = { 255, 255, 255, 255 };
  particle_color = clutter_color_new(0, 0, 0, 255);

  stage = clutter_stage_new();
  clutter_actor_set_size(stage, ARENA_WIDTH, ARENA_HEIGHT);
  clutter_actor_set_background_color(stage, &stage_color);

  clutter_actor_show(stage);

  int draw_loop_id = clutter_threads_add_timeout(200, draw_iteration, NULL);
  int process_loop_id = clutter_threads_add_idle(loop_iteration, NULL);

  clutter_main();

  draw();

  return EXIT_SUCCESS;
}

static gboolean loop_iteration(gpointer data) {
  simulate();
  return TRUE;
}

static gboolean draw_iteration(gpointer data) {
  draw();
  return TRUE;
}

void simulate() {
  // scan sensors, eliminate "impossible" instances
  sensor_scan scan = sensor_read();

  swarm_filter_particles(scan);

  // working on sensor, disable movement

  int angle = 0;
  /*if (robot_get_mode() == ROBOT_INITIAL) {
    angle = 0;
  } else {
    // poll remaining particles for steering direction
    double angle_sum = 0.0;
    for (i = 0; i < particle_count; i++) {
      angle_sum += fuzzy_get_angle(particles[i]);
    }

    angle = angle_sum/particle_count;
  }
  // move robot
  //  motor_move(angle);

   */


  /*
  // move duplicates of 1/2 of the particles to allow
  // synchronization via forward motion
  int orig_count = particle_count;
  for (i = 0; i < orig_count && particle_count < MAX_PARTICLES; i++) {
    if (rand_limit(2) == 0) {
      // do not init, we want to inherit score
      particle p = particles[i];
      // TODO: rename simulate_motor_move
      // create multiple particles with stochastic variation
      motor_move(angle, &p);
      particles[particle_count] = p;
      particle_count++;
    }
  }

  */

  printf("particle count: %i\n", swarm_get_size());

  int size = swarm_get_size();
  if (size == 0)
    swarm_reset();
  else
    swarm_evaluate(scan);

  // ensure we have at least one particle,
  // if we don't, rescue and skip the search
  /*
  int min_index = 0;
  double min = 1000000;
  double sum;
  particle p;
  sensor_scan s;
  int j;
  if (get_swarm_size() == 0) {
    sum = 0.0;
    p = particles[0];
    s = sensor_distance(p);
    for (j = 0; j < SENSOR_DISTANCES; j++) {
      sum += pow(scan.distances[j] - s.distances[j], 2.0);
      //sum += abs(scan.distances[j] - s.distances[j]);
    }

    sum = sqrt(sum);
    sum /= SENSOR_DISTANCES;
    min = sum;
  } else {
    // find best particle
    min_index = 0;
    min = 1000000;
    for (i = 0; i < particle_count; i++) {
      sum = 0.0;
      p = particles[i];
      s = sensor_distance(p);
      for (j = 0; j < SENSOR_DISTANCES; j++) {
	sum += pow(scan.distances[j] - s.distances[j], 2.0);
	//	sum += abs(scan.distances[j] - s.distances[j]);
      }

      sum = sqrt(sum);
      //      sum /= SENSOR_DISTANCES;

      particle_add_sample(particles + i, sum);

      if (particles[i].score < min) {
	min = sum;
	min_index = i;
      }
    }
  }

  // save it if we have a decent candidate
  particle best = particles[min_index];
  printf("%g\n", min);
  if (best.score < 600) {
    rescue = best;
    printf("x: %g; y: %g; theta: %g\n", rescue.x, rescue.y, rescue.theta);
    // leave initializtion mode if we have a lock
    if (robot_get_mode() == ROBOT_INITIAL && best.score < 100) {
      robot_set_mode(ROBOT_LOCK);
    }

    if (best.score > 150) {
      // add new particles from rescue
      // within 0.05 meter and 10 degrees
      // unless we have a really good fit already
      for (i = 0; i < 5 && particle_count < MAX_PARTICLES; i++) {
	particle p = particle_init(rescue.x + rand_limit(100) - 50,
				   rescue.y + rand_limit(100) - 50,
				   rescue.theta + rand_limit(20) - 10);
	particles[particle_count] = p;
	particle_count++;
      }
    }
  } else if (best.score < 1000) {
    // otherwise, save best and reset particles from rescue
    rescue = best;
    for (i = 0; i < MAX_PARTICLES; i++) {
      // within a half meter meter and 30 degrees
      particles[i] = particle_init(rescue.x + rand_limit(1000) - 500 ,
				   rescue.y + rand_limit(1000) - 500,
				   rescue.theta + rand_limit(60) - 30);
    }
    
    particle_count = MAX_PARTICLES;
    robot_set_mode(ROBOT_INITIAL);
  } else
    swarm_reset();

  // add a new particle based on the rescue particle,
  // but moved in the direction of the maximum distance
  // variation from the sensor scan
  p = rescue;
  sensor_scan p_scan = sensor_distance(p);

  // find direction of max deviation
  int max_deviation = 0.0;
  int max_index = 0;
  int sign = 1;
  for (i = 0; i < SENSOR_DISTANCES; i++) {
    int deviation = p_scan.distances[i] - scan.distances[i];
    if (abs(deviation) > max_deviation) {
      max_deviation = abs(deviation);
      max_index = i;
      sign = deviation/abs(deviation);
    }
  }

  // move particle
  //    double theta = sensor_distance_index_to_radians(max_index);
  //    double theta = sensor_distance_index_to_radians(max_index) - p.theta*M_PI/180;
  double theta = sensor_distance_index_to_radians(max_index) + p.theta*M_PI/180;
  double dx = sign*max_deviation*cos(theta);
  double dy = sign*max_deviation*sin(theta);
  p.x += dx;
  p.y += dy;

  // add particle
  particles[particle_count] = p;
  particle_count++;
  */

  // fortify particle_count
  // half from random and half from "swarm top"
  size = swarm_get_size();
  if (size < MAX_PARTICLES/2) {
    particle old, new;
    particle swarm_top[SWARM_TOP_COUNT];
    int old_particle_count = size;
    // no more than max/10 new particles
    // too much variation breaks lock
    int max_new = MAX_PARTICLES/10;
    swarm_top_particles(swarm_top);
    if (size < MAX_PARTICLES/10)
      max_new = MAX_PARTICLES - size;
    while (swarm_get_size() < old_particle_count + max_new) {
      if (rand_limit(2) == 0)
	old = swarm_get_random_particle();
      else
	old = swarm_top[rand_limit(SWARM_TOP_COUNT)];
      // +/- 0.5 meter
      new.x = old.x + rand_limit(1000) - 500;
      new.y = old.y + rand_limit(1000) - 500;
      new.x = old.x + rand_limit(1000) - 500;
      // +/- 30 degree
      new.theta = old.theta + rand_limit(60) - 30;
      // inherit score

      // add particle to swarm
      swarm_add_particle(new);
    }
  }
}

void draw() {
  // clear the stage
  printf("yo2\n");
  clutter_actor_destroy_all_children(stage);
  printf("yo2\n");
  // add obstacles
  // TODO

  // add first 15 particles
  ClutterActor *this_part;
  int i;
  particle p;
  printf("yo2\n");
  int size = swarm_get_size();
  particle particles[MAX_PARTICLES];
  printf("yo2\n");
  swarm_all_particles(particles);
  printf("yo2\n");
  for (i = 0; i < size && i < 15; i++) {
  printf("yo2\n");
    p = particles[i];
  printf("yo2\n");
    this_part = clutter_actor_new();
  printf("yo2\n");
    clutter_actor_set_background_color(this_part, particle_color);
    clutter_actor_set_size(this_part, PARTICLE_SIZE, PARTICLE_SIZE);
  printf("yo2\n");
    // invert y
    clutter_actor_set_position(this_part, p.x, ARENA_HEIGHT - p.y);
    clutter_actor_set_pivot_point(this_part, 0.5, 0.5);
    // negative theta
  printf("yo2\n");
    clutter_actor_set_rotation_angle(this_part, CLUTTER_Z_AXIS, -p.theta);
    clutter_actor_add_child(stage, this_part);
    clutter_actor_show(this_part);
  }
}

int rand_limit(int limit) {
  int r, d = RAND_MAX / limit;
  limit *= d;
  do {r = rand();} while (r >= limit);
  return r/d;
}
