#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "const.h"
#include "particle.h"
#include "hypothesis.h"

/*
int buffer_get_width();
int buffer_get_height();
uint8_t* buffer_allocate();
void buffer_attenuate(uint8_t*, double);
int buffer_index_from_x_y(int, int);
int x_from_buffer_index(int);
int y_from_buffer_index(int);
int index_protected(int);
int x_y_protected(int, int);

*/

void buffer_set_arena_size(int, int);
int buffer_get_size();

int buffer_hypothesis_distance(hypothesis*, int, int);
void buffer_deallocate(uint8_t*);

#endif
