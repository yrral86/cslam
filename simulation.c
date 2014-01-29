#include "simulation.h"

static particle *particles;
static particle *obstacles;
static particle rescue;
static particle robot;
static int particle_count;
static ClutterColor *particle_color;
static ClutterColor *robot_color;
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

  ClutterInitError e = clutter_init(&argc, &argv);

  ClutterColor stage_color = { 255, 255, 255, 255 };
  particle_color = clutter_color_new(0, 0, 0, 255);
  robot_color = clutter_color_new(255, 0, 0, 255);

  stage = clutter_stage_new();
  clutter_actor_set_size(stage, ARENA_WIDTH, ARENA_HEIGHT);
  clutter_actor_set_background_color(stage, &stage_color);

  clutter_actor_show(stage);

  int draw_loop_id = clutter_threads_add_timeout(200, draw_iteration, NULL);
  int process_loop_id = clutter_threads_add_idle(loop_iteration, NULL);

  clutter_main();
  /*
  while (1) {
    loop_iteration(NULL);
  }
  */

  return EXIT_SUCCESS;
}

void initialize_swarm() {
  // generate MAX_PARTICLES random particles in top half of starting area
  int i;
  for (i = 0; i < MAX_PARTICLES; i++) {
    particles[i].x = rand_limit(START_END);
    particles[i].y = rand_limit(ARENA_HEIGHT/2);
    particles[i].theta = rand_limit(360);
  }

  robot.x = rand_limit(START_END);
  robot.y = rand_limit(ARENA_HEIGHT/2);
  robot.theta = rand_limit(360);

  particle_count = MAX_PARTICLES;
  // save first particle
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
  sensor_scan scan = sensor_read(robot);
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
  /*
  int angle;
  if (robot_get_mode() == ROBOT_INITIAL) {
    angle = 15;
  } else {
    // poll remaining particles for steering direction
    double angle_sum = 0.0;
    for (i = 0; i < particle_count; i++) {
      angle_sum += fuzzy_get_angle(particles[i]);
    }

    angle = angle_sum/particle_count;
  }
  // move robot
  //  motor_move(angle, &robot);

  // if robot has gone out of bounds, reinitialize
  if (robot.x < 0 ||
      robot.x > ARENA_WIDTH ||
      robot.y < 0 ||
      robot.y > ARENA_HEIGHT) {
    if (robot.x > ARENA_WIDTH)
      success_count++;
    else
      failure_count++;
    printf("successes: %i failures: %i percent success: %g\n", success_count,
	   failure_count, (double)success_count/(success_count + failure_count));
    initialize_swarm();
    return;
  }

  // move each particle
  for (i = 0; i < particle_count; i++) {
  motor_move(angle, (particles + i));
  }

  // move rescue particle
  motor_move(angle, &rescue);
  */

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
      sum += pow(scan.distances[j] - s.distances[j], 2);
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
	//sum += abs(scan.distances[j] - s.distances[j]);
      }

      sum = sqrt(sum);
      sum /= SENSOR_DISTANCES;

      if (sum < min) {
	min = sum;
	min_index = i;
      }
    }
  }

  // save it if we have a decent candidate
  if (min < 10) {
    printf("%g\n", min);
    rescue = particles[min_index];
    printf("x: %g; y: %g; theta: %g\n", rescue.x, rescue.y, rescue.theta);
    // leave initializtion mode if we have a lock
    if (robot_get_mode() == ROBOT_INITIAL && min < 5) {
      robot_set_mode(ROBOT_LOCK);
    }

    if (min > 5) {
      // add new particles from rescue
      // unless we have a really good fit already
      for (i = 0; i < 5 && particle_count < MAX_PARTICLES; i++) {
	particle p;
	p.x = rescue.x + rand_limit(ARENA_WIDTH/100) - ARENA_WIDTH/200;
	p.y = rescue.y + rand_limit(ARENA_HEIGHT/100) - ARENA_WIDTH/200;
	p.theta = rescue.theta + rand_limit(36) - 18;
	particles[particle_count] = p;
	particle_count++;
      }
    }
  } else if (min < 15) {
    // otherwise, reset particles from rescue
    for (i = 0; i < MAX_PARTICLES; i++) {
      particles[i].x = rescue.x + rand_limit(ARENA_WIDTH/10) - ARENA_WIDTH/20;
      particles[i].y = rescue.y + rand_limit(ARENA_HEIGHT/10) - ARENA_WIDTH/20;
      particles[i].theta = rescue.theta + rand_limit(36) - 18;
    }
    
    particle_count = MAX_PARTICLES;
    robot_set_mode(ROBOT_INITIAL);
  } else {
    for (i = 0; i < MAX_PARTICLES; i++) {
      particles[i].x = rand_limit(ARENA_WIDTH);
      particles[i].y = rand_limit(ARENA_HEIGHT);
      particles[i].theta = rand_limit(360);
    }

    particle_count = MAX_PARTICLES;
    rescue = particles[0];
    robot_set_mode(ROBOT_INITIAL);
  }

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
    double theta = sensor_distance_index_to_radians(max_index) - p.theta*M_PI/180;
  //  double theta = sensor_distance_index_to_radians(max_index) + p.theta*M_PI/180;
  double dx = sign*max_deviation*cos(theta);
  double dy = sign*max_deviation*sin(theta);
  p.x += dx;
  p.y += dy;

  // add particle
  particles[particle_count] = p;
  particle_count++;

  // fortify particle_count
  if (particle_count < 35) {
    particle old, new;
    int old_particle_count = particle_count;
    // no more than 5 new particles
    // too much variation breaks lock
    while (particle_count < old_particle_count + 5) {
      old = particles[rand_limit(particle_count)];
      new.x = old.x + rand_limit(6) - 3;
      new.y = old.y + rand_limit(6) - 3;
      new.x = old.x + rand_limit(6) - 3;
      new.theta = old.theta + rand_limit(6) - 3;
      particles[particle_count] = new;
      particle_count++;
    }
  }
}

void draw() {
  // clear the stage
  clutter_actor_destroy_all_children(stage);

  // add obstacles
  // TODO

  // add first 15 particles
  particle p;
  int i, dx, dy;
  ClutterActor *this_part;
  for (i = 0; i < particle_count && i < 15; i++) {
  //  for (i = 0; i < particle_count; i++) {
    p = particles[i];
    this_part = clutter_actor_new();
    clutter_actor_set_background_color(this_part, particle_color);
    clutter_actor_set_size(this_part, PARTICLE_SIZE, PARTICLE_SIZE);
    // invert y
    clutter_actor_set_position(this_part, p.x, ARENA_HEIGHT - p.y);
    clutter_actor_set_pivot_point(this_part, 0.5, 0.5);
    clutter_actor_set_rotation_angle(this_part, CLUTTER_Z_AXIS, p.theta);
    clutter_actor_add_child(stage, this_part);
    clutter_actor_show(this_part);
  }

  /*
  // add robot
  this_part = clutter_actor_new();
  clutter_actor_set_background_color(this_part, robot_color);
  clutter_actor_set_size(this_part, PARTICLE_SIZE, PARTICLE_SIZE);
  clutter_actor_set_position(this_part, robot.x, robot.y);
  clutter_actor_set_pivot_point(this_part, 0.5, 0.5);
  clutter_actor_set_rotation_angle(this_part, CLUTTER_Z_AXIS, robot.theta);
  clutter_actor_add_child(stage, this_part);
  clutter_actor_show(this_part);
  */
}

int rand_limit(int limit) {
  int r, d = RAND_MAX / limit;
  limit *= d;
  do {r = rand();} while (r >= limit);
  return r/d;
}
