#include "map.h"
#include "lazygl.h"

int main(int argc, char **argv) {
  int width = 5000;
  int height = 5000;
  int i;
  map_node *map;

  uint8_t *buffer = malloc((width + 1)*(height + 1));
  glutInit(&argc, argv);
  initGL(buffer, buffer, width + 1, height + 1, (width + 1)/16, (height + 1)/16);
  
  map = map_new(width, height);

  for (i = 0; i < 25; i++) {
    map_set_seen(map, 10 + 2500, 10 + 2500);
    map_set_unseen(map, 50 + 2500, 50 + 2500);
    map_set_seen(map, 100 + 2500, 100 + 2500);
    map_set_unseen(map, 150 + 2500, 150 + 2500);
    map_set_seen(map, 200 + 2500, 200 + 2500);
    map_set_unseen(map, 250 + 2500, 250 + 2500);
    map_set_seen(map, 300 + 2500, 300 + 2500);
    map_set_unseen(map, 350 + 2500, 350 + 2500);
    map_set_seen(map, 400 + 2500, 400 + 2500);
  }

  map_write_buffer(map, buffer);

  map_deallocate(map);

  display();

  glutMainLoop();

  return 0;
}
