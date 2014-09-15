#include "map.h"
#include "lazygl.h"

int main(int argc, char **argv) {
  int width = 5000;
  int height = 5000;
  map_node *map;

  uint8_t *buffer = malloc((width + 1)*(height + 1));
  glutInit(&argc, argv);
  initGL(buffer, buffer, width + 1, height + 1, (width + 1)/16, (height + 1)/16);
  
  map = map_new(width, height);
  map_node_split(map);
  map_node_split(map->children[1]);
  map_set_seen(map, 10, 10);
  map_set_seen(map, 10 + 2500, 10 + 2500);
  map_write_buffer(map, buffer);

  map_deallocate(map);

  display();

  glutMainLoop();

  return 0;
}
