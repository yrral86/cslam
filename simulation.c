#include "simulation.h"

static particle *particles;
static particle *obstacles;
static particle rescue;
static int particle_count;
static ClutterColor *particle_color;
static ClutterActor *stage;
static int success_count, failure_count;

int main (int argc, char **argv) {
  particles = malloc(sizeof(particle)*MAX_PARTICLES);
  obstacles = malloc(sizeof(particle)*OBSTACLE_COUNT);

  success_count = 0;
  failure_count = 0;

  sensor_init();

  srand(time(NULL));

  initialize_swarm();
  /*
  ClutterInitError e = clutter_init(&argc, &argv);

  ClutterColor stage_color = { 255, 255, 255, 255 };
  particle_color = clutter_color_new(0, 0, 0, 255);

  stage = clutter_stage_new();
  clutter_actor_set_size(stage, ARENA_WIDTH, ARENA_HEIGHT);
  clutter_actor_set_background_color(stage, &stage_color);

  clutter_actor_show(stage);

  int draw_loop_id = clutter_threads_add_timeout(10000, draw_iteration, NULL);
  //  int process_loop_id = clutter_threads_add_idle(loop_iteration, NULL);

  clutter_main();
*/

  draw();

  return EXIT_SUCCESS;
}

void initialize_swarm() {
  // generate MAX_PARTICLES random particles all over map
  int i;
  for (i = 0; i < MAX_PARTICLES; i++) {
    particles[i] = particle_init(rand_limit(ARENA_WIDTH), rand_limit(ARENA_HEIGHT),
				 rand_limit(360));
  }

  particle_count = MAX_PARTICLES;

    // save first particle
  // todo: maintain a queue of top particles
  rescue = particles[0];

  robot_set_mode(ROBOT_INITIAL);
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
  int selected = 0;
  int i;
  for (i = 0; i < particle_count; i++) {
    if (!filter_particle(particles[i], scan)) {
      particles[selected] = particles[i];
      selected++;
    }
  }
  particle_count = selected;

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

  // eliminate any particles that are out of bounds
  selected = 0;
  for (i = 0; i < particle_count; i++) {
    if (in_bounds(particles[i].x, particles[i].y)) {
      particles[selected] = particles[i];
      selected++;
    }
  }
  particle_count = selected;

  printf("particle count: %i\n", particle_count);

  // ensure we have at least one particle,
  // if we don't, rescue and skip the search
  int min_index = 0;
  double min = 1000000;
  double sum;
  particle p;
  sensor_scan s;
  int j;
  if (particle_count == 0) {
    particles[0] = rescue;
    particle_count = 1;
    sum = 0.0;
    p = particles[0];
    s = sensor_distance(p);
    for (j = 0; j < SENSOR_DISTANCES; j++) {
      //      sum += pow(scan.distances[j] - s.distances[j], 2.0);
      sum += abs(scan.distances[j] - s.distances[j]);
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
	//	sum += pow(scan.distances[j] - s.distances[j], 2.0);
	sum += abs(scan.distances[j] - s.distances[j]);
      }

      sum = sqrt(sum);
      sum /= SENSOR_DISTANCES;

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
  if (best.score < 10) {
    rescue = best;
    printf("x: %g; y: %g; theta: %g\n", rescue.x, rescue.y, rescue.theta);
    // leave initializtion mode if we have a lock
    if (robot_get_mode() == ROBOT_INITIAL && best.score < 5) {
      robot_set_mode(ROBOT_LOCK);
    }

    if (best.score > 5) {
      // add new particles from rescue
      // unless we have a really good fit already
      for (i = 0; i < 5 && particle_count < MAX_PARTICLES; i++) {
	particle p = particle_init(rescue.x + rand_limit(ARENA_WIDTH/50) - ARENA_WIDTH/100,
				   rescue.y + rand_limit(ARENA_HEIGHT/50) - ARENA_WIDTH/100,
				   rescue.theta + rand_limit(36) - 18);
	particles[particle_count] = p;
	particle_count++;
      }
    }
  } else if (best.score < 20) {
    // otherwise, save best and reset particles from rescue
    rescue = best;
    for (i = 0; i < MAX_PARTICLES; i++) {
      particles[i] = particle_init(rescue.x + rand_limit(ARENA_WIDTH/10) - ARENA_WIDTH/20,
				   rescue.y + rand_limit(ARENA_HEIGHT/10) - ARENA_WIDTH/20,
				   rescue.theta + rand_limit(36) - 18);
    }
    
    particle_count = MAX_PARTICLES;
    robot_set_mode(ROBOT_INITIAL);
  } else
    initialize_swarm();

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

  // fortify particle_count from rescue
  if (particle_count < MAX_PARTICLES/2) {
    particle old, new;
    int old_particle_count = particle_count;
    // no more than max/10 new particles
    // too much variation breaks lock
    int max_new = MAX_PARTICLES/10;
    if (particle_count < MAX_PARTICLES/10)
      max_new = MAX_PARTICLES - particle_count;
    while (particle_count < old_particle_count + max_new) {
      old = particles[rand_limit(particle_count)];
      // +/- 0.5 meter
      new.x = old.x + rand_limit(1000) - 500;
      new.y = old.y + rand_limit(1000) - 500;
      new.x = old.x + rand_limit(1000) - 500;
      // +/- 30
      new.theta = old.theta + rand_limit(60) - 30;
      particles[particle_count] = new;
      particle_count++;
    }
  }
}

void draw() {
  // clear the stage
  //  clutter_actor_destroy_all_children(stage);

  // add obstacles
  // TODO

  int i, j, factor;
  factor = 1;
  for (i = 0; i < ARENA_WIDTH/factor; i++) {
    for (j = 0; j < ARENA_HEIGHT/factor; j++) {
      if (in_bounds(i, j)) {
	// do a jig 
/*
	this_part = clutter_actor_new();
	clutter_actor_set_background_color(this_part, particle_color);
	clutter_actor_set_size(this_part, factor, factor);
	clutter_actor_set_position(this_part, i*factor, j*factor);
	clutter_actor_add_child(stage, this_part);
	clutter_actor_show(this_part);*/
      } else
	printf("(%i, %i) is out of bounds\n", i*factor, j*factor);
    }

	  /*
	  // add first 15 particles
	  ClutterActor *this_part;
	  int i;
	  particle p;
	  for (i = 0; i < particle_count && i < 15; i++) {

	  p = particles[i];
	  this_part = clutter_actor_new();
	  clutter_actor_set_background_color(this_part, particle_color);
	  clutter_actor_set_size(this_part, PARTICLE_SIZE, PARTICLE_SIZE);
	  // invert y
	  clutter_actor_set_position(this_part, p.x, ARENA_HEIGHT - p.y);
	  clutter_actor_set_pivot_point(this_part, 0.5, 0.5);
	  // negative theta
	  clutter_actor_set_rotation_angle(this_part, CLUTTER_Z_AXIS, -p.theta);
	  clutter_actor_add_child(stage, this_part);
	  clutter_actor_show(this_part);
	  */
  }
  printf("draw done\n");

  printf("350,350: %i\n", in_bounds(350,350));
}

int rand_limit(int limit) {
  int r, d = RAND_MAX / limit;
  limit *= d;
  do {r = rand();} while (r >= limit);
  return r/d;
}
