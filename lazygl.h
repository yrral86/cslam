#ifndef __LAZYGL_H__
#define __LAZYGL_H__

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <math.h>
#include "map.h"

void initGL(uint8_t*, uint8_t*, int, int, int, int);
void display(map_node*);
void window_resize(int, int);

#endif
