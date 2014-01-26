#include "simulation.h"

#include "lazygl.h"
#include "particle.h"

#include <stdio.h>

const int ARENA_WIDTH = 738;
const int ARENA_HEIGHT = 388;
const int START_END = 150;
const int MINE_BEGIN = 444;
const int MAX_PARTICLES = 100;
const int OBSTACLE_COUNT = 6;
const int PARTICLE_RADIUS = 3;

static uint8_t *buffer;
static particle *particles;
static particle *obstacles;
static int particle_count;

int main (int argc, char **argv) {
  buffer = malloc(sizeof(uint8_t)*ARENA_HEIGHT*ARENA_WIDTH);
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

  glutInit(&argc, argv);
  initGL(buffer, ARENA_WIDTH, ARENA_HEIGHT);

  while (1) {
    simulate();

    draw();

    display();

    glutMainLoopEvent();
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
  // clear buffer
  int x, y;
  for (y = 0; y < ARENA_HEIGHT; y++)
    for (x = 0; x < ARENA_WIDTH; x++)
      set_position(x, y, 255);

  // add obstacles
  // add particles
  particle p;
  int i, dx, dy;
  for (i = 0; i < particle_count; i++) {
    p = particles[i];
    
    for (dx = -1*PARTICLE_RADIUS; dx <= PARTICLE_RADIUS; dx++)
      for (dy = -1*PARTICLE_RADIUS; dy <= PARTICLE_RADIUS; dy++)
	set_position(p.x + dx, p.y + dy, 0);
    set_position(p.x + PARTICLE_RADIUS + 1, p.y, 0);
    set_position(p.x + PARTICLE_RADIUS + 2, p.y, 0);
  }
}

int rand_limit(int limit) {
  int r, d = RAND_MAX / limit;
  limit *= d;
  do {r = rand();} while (r >= limit);
  return r/d;
}
