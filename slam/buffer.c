#include "buffer.h"

uint8_t* buffer_allocate() {
  uint8_t *buffer = malloc(sizeof(uint8_t)*BUFFER_SIZE);
  bzero(buffer, sizeof(uint8_t)*BUFFER_SIZE);
  return buffer;
}

void buffer_attenuate(uint8_t *buffer, double factor) {
  int i;
  for (i = 0; i < BUFFER_SIZE; i++)
    buffer[i] *= 0.75;
}

int buffer_index_from_x_y(double x, double y) {
  int i_x, i_y;
  i_x = x/BUFFER_FACTOR;
  i_y = y/BUFFER_FACTOR;
  if (i_x < BUFFER_WIDTH && i_y < BUFFER_HEIGHT)
    return i_y*BUFFER_WIDTH + i_x;
  else return 0;
}

int x_from_buffer_index(int index) {
  return (index % BUFFER_WIDTH)*BUFFER_FACTOR;
}

int y_from_buffer_index(int index) {
  return (index / BUFFER_WIDTH)*BUFFER_FACTOR;
}

int index_protected(int index) {
  int x, y;
  x = x_from_buffer_index(index);
  y = y_from_buffer_index(index);
  return x_y_protected(x, y);
}

// returns 1 if the position is "protected"
// protected pixels are the 10 buffer pixels around the border
// returns 0 if the position is not protected
int x_y_protected(int x, int y) {
  int protected = 0;
  if (x < 10*BUFFER_FACTOR || x > ARENA_WIDTH - 10*BUFFER_FACTOR ||
      y < 10*BUFFER_FACTOR || y > ARENA_HEIGHT - 10*BUFFER_FACTOR)
    protected = 1;
  return protected;
}
