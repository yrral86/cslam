#include "lazygl.h"

int ARENA_WIDTH = 1000;
int ARENA_HEIGHT = 400;

int main (int argc, char **argv) {
  uint8_t **buffer = malloc(sizeof(uint8_t*)*ARENA_HEIGHT);
  int i, j;
  for (i = 0; i < ARENA_HEIGHT; i++) {
    buffer[i] = malloc(ARENA_WIDTH*sizeof(uint8_t));
    for (j = 0; j < ARENA_WIDTH; j++)
      buffer[i][j] = 255;
  }

  glutInit(&argc, argv);
  initGL(buffer, ARENA_WIDTH, ARENA_HEIGHT);

  int toggle = 1;

  while (1) {
    display();
    if (toggle) {
      for (j = 0; j < ARENA_WIDTH; j++)
	buffer[ARENA_HEIGHT/2][j] = 0;
      toggle = 0;
    } else {
      for (j = 0; j < ARENA_WIDTH; j++)
	buffer[ARENA_HEIGHT/2][j] = 255;
      toggle = 1;
    }
    sleep(1);
  }

  return 0;
}
