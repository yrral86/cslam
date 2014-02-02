#include "random.h"

static float fn[128];
static uint32_t kn[128];
static uint32_t seed;
float wn[128];

int rand_limit(int limit) {
  int r, d = RAND_MAX / limit;
  limit *= d;
  do {r = rand();} while (r >= limit);
  return r/d;
}

double rand_normal(int variance) {
  return variance*r4_nor( &seed, kn, fn, wn );
}

void rand_normal_init() {
  r4_nor_setup(kn, fn, wn);
  seed = utime();
}
