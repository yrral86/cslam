#include "buffer.h"
#include <assert.h>

static int buffer_size, buffer_width, buffer_height, arena_width, arena_height;

void buffer_set_arena_size(int width_in, int height_in) {
  arena_width = width_in;
  arena_height = height_in;
  buffer_width = arena_width/BUFFER_FACTOR;
  buffer_height = arena_height/BUFFER_FACTOR;
  buffer_size = buffer_width*buffer_height;
}

int buffer_get_size() {
  return buffer_size;
}

int buffer_get_width() {
  return buffer_width;
}

int buffer_get_height() {
  return buffer_height;
}

uint8_t* buffer_allocate() {
  uint8_t *buffer = malloc(sizeof(uint8_t)*buffer_size);
  memset(buffer, '\0', sizeof(uint8_t)*buffer_size);
  return buffer;
}

void buffer_attenuate(uint8_t *buffer, double factor) {
  int i;
  for (i = 0; i < buffer_size; i++)
    buffer[i] *= 0.75;
}

int buffer_index_from_x_y(int x, int y) {
  int i_x, i_y;
  i_x = x/BUFFER_FACTOR;
  i_y = y/BUFFER_FACTOR;
  if (i_x >= 0 && i_x < buffer_width && i_y >= 0 && i_y < buffer_height)
    return i_y*buffer_width + i_x;
  else assert(0);
}

int x_from_buffer_index(int index) {
  return (index % buffer_width)*BUFFER_FACTOR;
}

int y_from_buffer_index(int index) {
  return (index / buffer_width)*BUFFER_FACTOR;
}

int index_protected(int index) {
  int x, y;
  x = x_from_buffer_index(index);
  y = y_from_buffer_index(index);
  return x_y_protected(x, y);
}

// returns 1 if the position is "protected"
// protected pixels are the border pixels
// returns 0 if the position is not protected
int x_y_protected(int x, int y) {
  int protected = 0;
  if (x < BORDER_WIDTH || x > arena_width - BORDER_WIDTH ||
      y < BORDER_WIDTH || y > arena_height - BORDER_WIDTH)
    protected = 1;
  return protected;
}
