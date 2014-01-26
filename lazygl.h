#ifndef __LAZYGL_H__
#define __LAZYGL_H__

#include <GL/glut.h>
#include <math.h>

void initGL (uint8_t*, int, int);

void display ();

void set_position(int, int, int);

static uint8_t *buffer;
static int width, height;

#endif
