#include "map.h"

static int width, height;

map_node* map_new(int w, int h) {
  width = w + 1;
  height = h + 1;
  return map_node_new(0, w, 0, h);
}

map_node* map_node_new(int x_min, int x_max, int y_min, int y_max) {
  int i;
  map_node *n = malloc(sizeof(map_node));
  // init ranges
  n->x_min = x_min;
  n->x_max = x_max;
  n->y_min = y_min;
  n->y_max = y_max;

  // init landmarks and children
  for (i = 0; i < 4; i++) {
    n->landmarks[i].seen = 0;
    n->landmarks[i].unseen = 0;
    n->children[i] = NULL;
  }

  n->landmark.seen = 0;
  n->landmark.unseen = 0;

  return n;
}

int map_node_index_from_x_y(map_node *node, int x, int y) {
  int index;

  // find quadrant
  if (x <= (node->x_min + node->x_max)/2 && y > (node->y_min + node->y_max)/2)
    //top left
    index = 0;
  else if (x > (node->x_min + node->x_max)/2 && y > (node->y_min + node->y_max)/2)
    // top right
    index = 1;
  else if (x <= (node->x_min + node->x_max)/2 && y <= (node->y_min + node->y_max)/2)
    // bottom left
    index = 2;
  else if (x > (node->x_min + node->x_max)/2 && y <= (node->y_min + node->y_max)/2)
    // bottom right
    index = 3;

  return index;
}

void map_node_ranges_from_index(map_node *node, int index, int *x_min, int *x_max, int *y_min, int *y_max) {
  switch (index) {
  case 0:
    // top left
    *x_min = node->x_min;
    *x_max = (node->x_min + node->x_max)/2;
    *y_min = (node->y_min + node->y_max)/2 + 1;
    *y_max = node->y_max;
    break;
  case 1:
    // top right
    *x_min = (node->x_min + node->x_max)/2 + 1;
    *x_max = node->x_max;
    *y_min = (node->y_min + node->y_max)/2 + 1;
    *y_max = node->y_max;
    break;
  case 2:
    // bottom left
    *x_min = node->x_min;
    *x_max = (node->x_min + node->x_max)/2;
    *y_min = node->y_min;
    *y_max = (node->y_min + node->y_max)/2;
    break;
  case 3:
    // bottom right
    *x_min = (node->x_min + node->x_max)/2 + 1;
    *x_max = node->x_max; 
    *y_min = node->y_min;
    *y_max = (node->y_min + node->y_max)/2;
    break;
  }
}

void map_node_split(map_node *map) {
  map_node *tmp;
  int x_min, x_max, y_min, y_max, i;

  for (i = 0; i < 4; i++) {
    // set ranges
    map_node_ranges_from_index(map, i, &x_min, &x_max, &y_min, &y_max);

    // allocate new node
    tmp = map_node_new(x_min, x_max, y_min, y_max);

    // copy landmark
    tmp->landmark = map->landmarks[i];

    // set child to new node
    map->children[i] = tmp;
  }
}

void map_deallocate(map_node *map) {
  int i;

  // deallocate children
  for (i = 0; i < 4; i++)
    if (map->children[i] != NULL)
      map_deallocate(map->children[i]);

  free(map);
}

void map_set_seen(map_node *map, int x, int y) {
  int index;

  (map->landmark.seen)++;

  index = map_node_index_from_x_y(map, x, y);

  // we are at a leaf, record sublandmark
  if (map->children[index] == NULL)
    (map->landmarks[index].seen)++;
  else
    // interior node, traverse
    map_set_seen(map->children[index], x, y);
}

void map_set_unseen(map_node *map, int x, int y) {
  int index;

  (map->landmark.unseen)++;

  index = map_node_index_from_x_y(map, x, y);

  // we are at a leaf, record sublandmark
  if (map->children[index] == NULL)
    (map->landmarks[index].unseen)++;
  else
    // interior node, traverse
    map_set_unseen(map->children[index], x, y);
}

void map_write_buffer(map_node *map, uint8_t *buffer) {
  // clear buffer
  bzero(buffer, (map->x_max - map->x_min + 1)*(map->y_max - map->y_min + 1));
  // write nodes
  map_node_write_buffer(map, buffer);
}

void map_node_write_buffer(map_node *node, uint8_t *buffer) {
  int x_min, x_max, y_min, y_max, sum, value, i, x, y;

  for (i = 0; i < 4; i++) {
    if (node->children[i] == NULL) {
      // leaf node

      // determine value
      sum = node->landmarks[i].seen + node->landmarks[i].unseen;
      // don't divide by zero
      if (sum == 0) sum = 1;
      value = (int)(255 * node->landmarks[i].seen/(double)sum);

      // write block with value
      if (value > 0) {
	// set ranges
	map_node_ranges_from_index(node, i, &x_min, &x_max, &y_min, &y_max);
	for (y = y_min; y <= y_max; y++)
	  for (x = x_min; x <= x_max; x++)
	    buffer[width*y + x] = value;
      }
    } else {
      // interior node
      map_node_write_buffer(node->children[i], buffer);
    }
  }
}
