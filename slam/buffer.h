#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdlib.h>
#include <stdint.h>
#include <strings.h>

#include "arena.h"
#include "particle.h"
#include "sensor.h"

#define BUFFER_FACTOR 10
#define BUFFER_WIDTH (ARENA_WIDTH/BUFFER_FACTOR)
#define BUFFER_HEIGHT (ARENA_HEIGHT/BUFFER_FACTOR)
#define BUFFER_SIZE (BUFFER_WIDTH*BUFFER_HEIGHT)

uint8_t* buffer_allocate();
void buffer_attenuate(uint8_t*, double);
int buffer_index_from_x_y(double, double);
int x_from_from_buffer_index(int);
int y_from_buffer_index(int);
int index_protected(int);
int x_y_protected(int, int);
int index_is_visible(int, particle);

#endif
