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
  index *= BUFFER_FACTOR*BUFFER_FACTOR;
  return index % ARENA_WIDTH;
}

int y_from_buffer_index(int index) {
  index *= BUFFER_FACTOR*BUFFER_FACTOR;
  return index / ARENA_WIDTH;
}

// returns 1 if the index is "protected"
// protected pixels are the 5 buffer pixels around the border
// returns 0 if the index is not protected
int index_protected(int index) {
  int x, y;
  int protected = 0;
  x = x_from_buffer_index(index);
  y = y_from_buffer_index(index);
  if (x < 5*BUFFER_FACTOR || x > ARENA_WIDTH - 5*BUFFER_FACTOR ||
      y < 5*BUFFER_FACTOR || y > ARENA_HEIGHT - 5*BUFFER_FACTOR)
    protected = 1;
  return protected;
}
