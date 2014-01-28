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

  srand(time(NULL));

  initialize_swarm();
  /*
  ClutterInitError e = clutter_init(&argc, &argv);

  ClutterColor stage_color = { 255, 255, 255, 255 };
  particle_color = clutter_color_new(0, 0, 0, 255);
  robot_color = clutter_color_new(255, 0, 0, 255);

  stage = clutter_stage_new();
  clutter_actor_set_size(stage, ARENA_WIDTH, ARENA_HEIGHT);
  clutter_actor_set_background_color(stage, &stage_color);

  clutter_actor_show(stage);

  int loop_id = clutter_threads_add_timeout(20, loop_iteration, NULL);

  clutter_main();

  */
  while (1) {
    loop_iteration(NULL);
  }


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

  //skip draw for now
  //  draw();

  return TRUE;
}

void simulate() {
  // scan sensors, eliminate "impossible" instances
  // get normalized and raw sensor values
  sensor_scan scan_normalized = sensor_distance(robot);
  particle r_clone = robot;
  r_clone.theta = 0;
  sensor_scan scan = sensor_distance(r_clone);
  int selected = 0;
  int i;
  for (i = 0; i < particle_count; i++) {
    particle clone = particles[i];
    clone.theta = 0;
    if (!(filter_particle(particles[i], scan_normalized) ||
	  filter_particle(clone, scan))) {
      particles[selected] = particles[i];
      selected++;
    }
  }
  particle_count = selected;

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
  // move robot in appropriate direction
  motor_move(angle, &robot);

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

  // move each particle in average direction
  for (i = 0; i < particle_count; i++) {
    motor_move(angle, (particles + i));
  }

  // move rescue particle
  motor_move(angle, &rescue);

  // eliminate any particles that are out of bounds
  selected = 0;
  for (i = 0; i < particle_count; i++) {
    if (particles[i].x < 0 ||
	particles[i].y < 0 ||
	particles[i].x > ARENA_WIDTH ||
	particles[i].y > ARENA_HEIGHT)
      continue;
    else {
      particles[selected] = particles[i];
      selected++;
    }
  }
  particle_count = selected;

  // ensure we have at least one particle
  if (particle_count == 0) {
    particles[0] = rescue;
    particle_count = 1;
  }

  // find best particle
  int raw_min_index = 0;
  int norm_min_index = 0;
  double raw_min = 1000;
  double norm_min = 1000;
  double norm_sum;
  double raw_sum;
  particle p, clone;
  sensor_scan s, s_n;
  int j;
  for (i = 0; i < particle_count; i++) {
    norm_sum = 0.0;
    raw_sum = 0.0;
    p = particles[i];
    clone = p;
    clone.theta = 0;
    s_n = sensor_distance(p);
    s = sensor_distance(clone);
    for (j = 0; j < SENSOR_DISTANCES; j++) {
      raw_sum += abs(scan.distances[j] - s.distances[j]);
      norm_sum += abs(scan_normalized.distances[j] - s_n.distances[j]);
    }
    raw_sum /= SENSOR_DISTANCES;
    norm_sum /= SENSOR_DISTANCES;

    if (raw_sum < raw_min) {
      raw_min = raw_sum;
      raw_min_index = i;
    }

    if (norm_sum < norm_min) {
      norm_min = norm_sum;
      norm_min_index = i;
    }
  }

  // save it if we have a "good" lock
  if (norm_min < 10) {
    rescue = particles[norm_min_index];
    // leave initializtion mode
    if (robot_get_mode() == ROBOT_INITIAL && norm_min < 1) {
      // check if we have orientation lock as well
      if (raw_min < 1)
	robot_set_mode(ROBOT_PO_LOCK);
      else
	robot_set_mode(ROBOT_P_LOCK);
    } else if (robot_get_mode() == ROBOT_P_LOCK && raw_min < 1) {
      robot_set_mode(ROBOT_PO_LOCK);
    }
  } else {
    // otherwise, reset particles
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
  // variation from the normalized sensor scan
  p = rescue;
  sensor_scan p_scan = sensor_distance(p);

  // find direction of max deviation
  int max_deviation = 0.0;
  int max_index = 0;
  int sign = 1;
  for (i = 0; i < SENSOR_DISTANCES; i++) {
    int deviation = p_scan.distances[i] - scan_normalized.distances[i];
    if (abs(deviation) > max_deviation) {
      max_deviation = abs(deviation);
      max_index = i;
      sign = deviation/abs(deviation);
    }
  }

  // move particle
  double theta = sensor_distance_index_to_radians(max_index);
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
    p = particles[i];
    this_part = clutter_actor_new();
    clutter_actor_set_background_color(this_part, particle_color);
    clutter_actor_set_size(this_part, PARTICLE_SIZE, PARTICLE_SIZE);
    clutter_actor_set_position(this_part, p.x, p.y);
    clutter_actor_set_pivot_point(this_part, 0.5, 0.5);
    clutter_actor_set_rotation_angle(this_part, CLUTTER_Z_AXIS, p.theta);
    clutter_actor_add_child(stage, this_part);
    clutter_actor_show(this_part);
  }

  // add robot
  this_part = clutter_actor_new();
  clutter_actor_set_background_color(this_part, robot_color);
  clutter_actor_set_size(this_part, PARTICLE_SIZE, PARTICLE_SIZE);
  clutter_actor_set_position(this_part, robot.x, robot.y);
  clutter_actor_set_pivot_point(this_part, 0.5, 0.5);
  clutter_actor_set_rotation_angle(this_part, CLUTTER_Z_AXIS, robot.theta);
  clutter_actor_add_child(stage, this_part);
  clutter_actor_show(this_part);
}

int rand_limit(int limit) {
  int r, d = RAND_MAX / limit;
  limit *= d;
  do {r = rand();} while (r >= limit);
  return r/d;
}
