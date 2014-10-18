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

/*
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
*/

int buffer_hypothesis_distance(hypothesis *h, int offset, int divisor) {
  int i, j, index, distance, factor, adj, x_min, x_max, y_min, y_max;
  double c, s, d, x, y, theta;
  double h_theta, h_x, h_y;
  uint8_t *b;
  observation *list = h->obs->list;
  h_theta = h->theta;
  h_x = h->x;
  h_y = h->y;
  b = h->map->buffer;
  x_min = h->map->center_x - h->map->width/2;
  x_max = h->map->center_x + h->map->width/2;
  y_min = h->map->center_y - h->map->height/2;
  y_max = h->map->center_y + h->map->height/2;

  // width of row
  factor = h->map->width/BUFFER_FACTOR;
  // adjust for center of map
  adj = factor*h->map->center_y/BUFFER_FACTOR + h->map->center_x/BUFFER_FACTOR;
  // adjust for min x and min y of map
  adj -= factor*y_min + x_min;

  distance = 0;
  for (i = offset; i < RAW_SENSOR_DISTANCES_USB; i += divisor) {
      // observation theta + pose theta
      theta = (list[i].theta + h_theta)*M_PI/180;
      c = cos(theta);
      s = sin(theta);

    // find distance
    for (j = -5; j < 6; j++) {
      // adjust distance by BUFFER_FACTOR to enable line trace
      d = list[i].r + j*BUFFER_FACTOR;
      x = h_x + d*c;
      y = h_y + d*s;

      if (x >= x_min && x <= x_max &&
	  y >= y_min && y <= y_max) {
	index = factor*y + x - adj;

	//	printf("x: %d y: %d width: %d height: %d\n", x, y, h->map->width, h->map->height);
	//	assert(index < factor*h->map->height/BUFFER_FACTOR);

	// if we are fairly certain about this location being hit
	if (b[index] > 200) {
	  distance += abs(j);
	  // next theta
	  break;
	}
      }// outside of parent map, don't punish
      /* else {
	// out of bounds
	distance += 10;
	// next theta
	break;
	}*/
    }
  }

  if (j == 6)
    // we didn't find an obstacle
    distance += abs(j);

  return distance;
}

void buffer_deallocate(uint8_t *b) {
  free(b);
}
