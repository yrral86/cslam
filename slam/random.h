#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <stdlib.h>
#include <stdint.h>

#include "ziggurat.h"

int rand_limit(int);
double rand_normal(int);
void rand_normal_init();

#endif
