#include "simulation.h"

const int ARENA_WIDTH = 738;
const int ARENA_HEIGHT = 388;
const int START_END = 150;
const int MINE_BEGIN = 444;
const int MAX_PARTICLES = 250;
const int OBSTACLE_COUNT = 6;
const int PARTICLE_SIZE = 10;

//static uint8_t *buffer;
static particle *particles;
static particle *obstacles;
static particle rescue;
static particle robot;
static int particle_count;
static ClutterColor *particle_color;
static ClutterColor *robot_color;
static ClutterActor *stage;

int main (int argc, char **argv) {
  particles = malloc(sizeof(particle)*MAX_PARTICLES);
  obstacles = malloc(sizeof(particle)*OBSTACLE_COUNT);

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

  ClutterInitError e = clutter_init(&argc, &argv);

  ClutterColor stage_color = { 255, 255, 255, 255 };
  particle_color = clutter_color_new(0, 0, 0, 255);
  robot_color = clutter_color_new(255, 0, 0, 255);

  stage = clutter_stage_new();
  clutter_actor_set_size(stage, ARENA_WIDTH, ARENA_HEIGHT);
  clutter_actor_set_background_color(stage, &stage_color);

  clutter_actor_show(stage);

  int loop_id = clutter_threads_add_timeout(100, loop_iteration, NULL);

  clutter_main();

  return EXIT_SUCCESS;
}

static gboolean loop_iteration(gpointer data) {
  simulate();

  draw();

  return TRUE;
}

void simulate() {
  // save first particle
  rescue = particles[0];

  // scan sensors, eliminate "impossible" instances
  int f_distance = sensor_distance_forward(robot);
  /*  int r_distance = sensor_distance_reverse(robot);
  int l_distance = sensor_distance_left(robot);
  int rt_distance = sensor_distance_right(robot);*/
  int selected = 0;
  int i;
  for (i = 0; i < particle_count; i++) {
    if (filter_particle_forward(particles[i], f_distance))
      continue;
    /*    else if (filter_particle_reverse(particles[i], r_distance))
      continue;
    else if (filter_particle_left(particles[i], l_distance))
      continue;
    else if (filter_particle_right(particles[i], rt_distance))
    continue;*/
    else {
      particles[selected] = particles[i];
      selected++;
    }
  }
  particle_count = selected;

  // poll remaining particles for steering direction
  double angle_sum = 0.0;
  for (i = 0; i < particle_count; i++) {
    angle_sum += fuzzy_get_angle(particles[i]);
  }

  int angle = angle_sum/particle_count;
  // move robot in average direction
  motor_move(angle, &robot);

  // move each particle in average direction with random errors (replicate each particle)
  for (i = 0; i < particle_count; i++) {
    motor_move(angle, (particles + i));
  }

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

  // fortify particle_count
  if (particle_count < 50) {
    particle old, new;
    while (particle_count < 100) {
      if (particle_count > 0)
	old = particles[rand_limit(particle_count)];
      else
	old = rescue;
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

  // add particles
  particle p;
  int i, dx, dy;
  ClutterActor *this_part;
  for (i = 0; i < particle_count; i++) {
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
