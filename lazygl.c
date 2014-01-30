#include "lazygl.h"

void initGL(uint8_t *b, int w, int h) {
  buffer = b;
  width = w;
  height = h;

  glClearColor(0.0, 0.0, 0.0, 0.0);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(width*2, height*2);
  glutCreateWindow("sensor");
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelZoom(2.0, 2.0);
  glDrawPixels(width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);

  glutSwapBuffers();
}
