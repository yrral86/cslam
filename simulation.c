#include "simulation.h"

#include "particle.h"

#include <clutter/clutter.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

const int ARENA_WIDTH = 738;
const int ARENA_HEIGHT = 388;
const int START_END = 150;
const int MINE_BEGIN = 444;
const int MAX_PARTICLES = 100;
const int OBSTACLE_COUNT = 6;
const int PARTICLE_SIZE = 10;

//static uint8_t *buffer;
static particle *particles;
static particle *obstacles;
static int particle_count;
static ClutterColor *particle_color;
static ClutterActor *stage;

int main (int argc, char **argv) {
  particles = malloc(sizeof(particle)*MAX_PARTICLES);
  obstacles = malloc(sizeof(particle)*OBSTACLE_COUNT);

  // generate MAX_PARTICLES random particles in top half of starting area
  int i;
  particle p;
  for (i = 0; i < MAX_PARTICLES; i++) {
    particles[i].x = rand_limit(START_END);
    particles[i].y = rand_limit(ARENA_HEIGHT/2);
    particles[i].theta = rand_limit(360);
    p = particles[i];
  }

  particle_count = MAX_PARTICLES;

  clutter_init(&argc, &argv);

  ClutterColor stage_color = { 255, 255, 255, 255 };
  particle_color = clutter_color_new(0, 0, 0, 255);

  stage = clutter_stage_new();
  clutter_actor_set_size(stage, ARENA_WIDTH, ARENA_HEIGHT);
  clutter_actor_set_background_color(stage, &stage_color);

  while (1) {
    simulate();

    draw();

    clutter_actor_show(stage);

    clutter_main();

    //    display();

    //    glutMainLoopEvent();
  }

  return 0;
}

void simulate() {
    // scan particles, eliminate "impossible" instances
    // poll remaining particles for steering direction
    // move robot in average direction
    // move each particle in average direction with random errors (replicate each particle)

}

void draw() {
  // clear the stage
  clutter_actor_destroy_all_children(stage);

  // add obstacles
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
}

int rand_limit(int limit) {
  int r, d = RAND_MAX / limit;
  limit *= d;
  do {r = rand();} while (r >= limit);
  return r/d;
}
