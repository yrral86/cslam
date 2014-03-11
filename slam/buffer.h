#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
//#include <strings.h>
#define bzero(b, len) (memset((b), '\0', (len)), (void) 0)

#include "const.h"
#include "arena.h"
#include "particle.h"
//#include "sensor.h"

void buffer_set_arena_size(int, int);
int buffer_get_size();
int buffer_get_width();
int buffer_get_height();
uint8_t* buffer_allocate();
void buffer_attenuate(uint8_t*, double);
int buffer_index_from_x_y(double, double);
int x_from_from_buffer_index(int);
int y_from_buffer_index(int);
int index_protected(int);
int x_y_protected(int, int);

#endif
