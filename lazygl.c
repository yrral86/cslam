#include "lazygl.h"

static uint8_t *buffer1, *buffer2;
static int buffer_width, buffer_height, window_width, window_height;

void initGL(uint8_t *b1, uint8_t *b2, int b_w, int b_h, int w_w, int w_h) {
  buffer1 = b1;
  buffer2 = b2;
  buffer_width = b_w;
  buffer_height = b_h;

  window_width = w_w;
  window_height = w_h;

  glClearColor(0.0, 0.0, 0.0, 0.0);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(window_width, window_height*2);
  //glutInitWindowSize(window_width, window_height);
  glutCreateWindow("sensor");
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelZoom((double)window_width/buffer_width, (double)window_height/buffer_height);
  glWindowPos2i(0, 0);
  glDrawPixels(buffer_width, buffer_height, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer1);
  glWindowPos2i(0, window_height);
  glDrawPixels(buffer_width, buffer_height, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer2);  
  glutSwapBuffers();
}
