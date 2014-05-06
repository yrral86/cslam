#include "random.h"
#include <stdio.h>

//static float fn[128];
//static uint32_t kn[128];
static uint32_t seed;
//static float wn[128];

int rand_limit(int limit) {
  int r, d = RAND_MAX / limit;
  limit *= d;
  do {r = rand();} while (r >= limit);
  return r/d;
}

double rand_normal(int variance) {
  return box_muller(0, (double)variance);
}

void rand_normal_init() {
  seed = utime();
  srand(seed);
}

// do not use for actual time due to platform differences, only relative microseconds
uint64_t utime() {
	uint64_t t = 0;
#ifndef LINUX
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	t |= ft.dwHighDateTime;
	t <<= 32;
	t |= ft.dwLowDateTime;;
#endif
#ifdef LINUX
  struct timeval tv;
  gettimeofday(&tv, NULL);
  t = tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
#endif
  return t;
}

float ranf() {
	return rand()/(double)RAND_MAX;
}
