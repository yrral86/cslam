#include "lazygl.h"

void initGL (uint8_t **buff, int w, int h) {
  buffer = buff;
  width = w;
  height = h;

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(width, height);

  glutCreateWindow("simulation");
}

void display () {
  glClearColor(0.0, 0.0, 0.0, 0.0);

  glClear(GL_COLOR_BUFFER_BIT);

  glDrawPixels(width, height, GL_LUMINANCE,
  	       GL_UNSIGNED_BYTE, buffer);

  glutSwapBuffers();
}
