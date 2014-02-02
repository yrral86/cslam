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
