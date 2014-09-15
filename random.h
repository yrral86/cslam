#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <stdlib.h>
#include <stdint.h>
#ifndef LINUX
#include <Windows.h>
#include <time.h>
#endif
#ifdef LINUX
#include <sys/time.h>
#endif

#include "boxmuller.h"

float ranf();
int rand_limit(int);
double rand_normal(int);
void rand_normal_init();
uint64_t utime();

#endif
