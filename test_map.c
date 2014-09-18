#include "map.h"
#include "lazygl.h"

int main(int argc, char **argv) {
  int width = 10000;
  int height = 10000;
  int i, x_min, x_max, y_min, y_max, index;
  map_node *map;

  uint8_t *buffer = malloc((width + 1)*(height + 1));
  glutInit(&argc, argv);
  initGL(buffer, buffer, width + 1, height + 1, (width + 1)/16, (height + 1)/16);
  
  map = map_new(width, height);

  map_set_seen(map, 3909, 3110);

  index = map_node_index_from_x_y(map, 3912, 3089);

  //  printf("after first plot\n");
  //  map_debug(map);

  map_set_seen(map, 3912, 3089);

  //  printf("after second plot\n");
  //  map_debug(map);

  map_write_buffer(map, buffer);

  map_deallocate(map);

  display();

  glutMainLoop();

  return 0;
}
