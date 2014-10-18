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
  //glutInitWindowSize(window_width, window_height*2);
  glutInitWindowSize(window_width, window_height);
  glutCreateWindow("sensor");
  glutReshapeFunc(window_resize);
}

void display(map_node *m) {
  int w, h;
  assert(m->buffer != NULL);
  w = m->width/BUFFER_FACTOR;
  h = m->height/BUFFER_FACTOR;
  glClear(GL_COLOR_BUFFER_BIT);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelZoom((double)window_width/w, (double)window_height/h);
  glWindowPos2i(0, 0);
  glDrawPixels(w, h, GL_LUMINANCE, GL_UNSIGNED_BYTE, m->buffer);
  //  glWindowPos2i(0, window_height);
  //  glDrawPixels(buffer_width, buffer_height, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer2);
  glutSwapBuffers();
  glutReshapeWindow(window_width, window_height);
}

void window_resize(int w, int h) {
  window_width = w;
  window_height = h;
}
