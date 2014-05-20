#ifndef __LAZYGL_H__
#define __LAZYGL_H__

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <math.h>

void initGL(uint8_t*, uint8_t*, int, int, int, int);
void display();

static uint8_t *buffer1, *buffer2;
static int buffer_width, buffer_height, window_width, window_height;

#endif
