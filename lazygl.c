#include "lazygl.h"

void initGL(uint8_t *b, int b_w, int b_h, int w_w, int w_h) {
  buffer = b;
  buffer_width = b_w;
  buffer_height = b_h;

  window_width = w_w;
  window_height = w_h;

  glClearColor(0.0, 0.0, 0.0, 0.0);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(window_width, window_height);
  glutCreateWindow("sensor");
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelZoom((double)window_width/buffer_width, (double)window_height/buffer_height);
  glDrawPixels(buffer_width, buffer_height, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);

  glutSwapBuffers();
}
