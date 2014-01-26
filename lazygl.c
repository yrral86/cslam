#include "lazygl.h"

void initGL (uint8_t *buff, int w, int h) {
  buffer = buff;
  width = w;
  height = h;

  glClearColor(0.0, 0.0, 0.0, 0.0);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(width, height);

  glutCreateWindow("simulation");
}

void display () {
  glClear(GL_COLOR_BUFFER_BIT);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glDrawPixels(width, height, GL_LUMINANCE,
  	       GL_UNSIGNED_BYTE, buffer);

  glutSwapBuffers();
}

void set_position(int x, int y, int value) {
  buffer[y*width + x] = value;
}
