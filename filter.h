#ifndef __FILTER_H__
#define __FILTER_H__

#include "particle.h"

int filter_particle_forward(particle, int);
int filter_particle_reverse(particle, int);
int filter_particle_left(particle, int);
int filter_particle_right(particle, int);

#endif
