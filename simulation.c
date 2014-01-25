#include "lazygl.h"

const int ARENA_WIDTH = 738;
const int ARENA_HEIGHT = 388;
const int START_END = 150;
const int MINE_BEGIN = 444;

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

  display();

  /*
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
    }*/
  glutMainLoop();

  return 0;
}
