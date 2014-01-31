#ifndef __LAZYGL_H__
#define __LAZYGL_H__

#include <GL/glut.h>
#include <math.h>

void initGL(uint8_t*, int, int, int, int);
void display();

static uint8_t *buffer;
static int buffer_width, buffer_height, window_width, window_height;

#endif
