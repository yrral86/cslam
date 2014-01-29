#ifndef __PARTICLE_H__
#define __PARTICLE_H__

typedef struct particle {
  double x;
  double y;
  double theta;
  int samples;
  double score;
} particle;

void particle_add_sample(particle*, double);
particle particle_init(int, int, int);

#endif
