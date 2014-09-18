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

void map_merge(map_node *all, map_node *latest, int dx, int dy, int dt) {
  // copy latest into all
  int i, j, x, y, x_min, x_max, y_min, y_max;
  double r, theta;

  for (i = 0; i < 4; i++) {
    if (latest->children[i] == NULL) {
      // leaf
      // find center of region
      map_node_ranges_from_index(latest, i, &x_min, &x_max, &y_min, &y_max);
      x = (x_max - x_min)/2;
      y = (y_max - y_min)/2;

      // translate x, y
      r = sqrt(dx*dx+dy*dy);
      theta = dt*M_PI/180;
      x += r*cos(theta);
      y += r*cos(theta);

      // plot seen/unseen at (x, y)
      for (j = 0; j < latest->landmarks[i].seen; j++)
	map_set_seen(all, x, y);
      for (j = 0; j < latest->landmarks[i].unseen; j++)
	map_set_unseen(all, x, y);
    } else
      map_merge(all, latest->children[i], dx, dy, dt);
  }
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
  else
    assert(0);

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

void map_node_split(map_node *map, int index) {
  map_node *tmp;
  int x_min, x_max, y_min, y_max;

  // set ranges
  map_node_ranges_from_index(map, index, &x_min, &x_max, &y_min, &y_max);

  // allocate new node
  tmp = map_node_new(x_min, x_max, y_min, y_max);

  // copy landmark
  tmp->landmark = map->landmarks[index];

  // set child to new node
  map->children[index] = tmp;
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
  if (map->children[index] == NULL) {
    map->landmarks[index].seen++;
    map_landmark_check_split(map, index);
  } else
    // interior node, traverse
    map_set_seen(map->children[index], x, y);
}

void map_set_unseen(map_node *map, int x, int y) {
  int index;

  (map->landmark.unseen)++;

  index = map_node_index_from_x_y(map, x, y);

  // we are at a leaf, record sublandmark
  if (map->children[index] == NULL) {
    map->landmarks[index].unseen++;
    map_landmark_check_split(map, index);
  } else
    // interior node, traverse
    map_set_unseen(map->children[index], x, y);
}

void map_landmark_check_split(map_node *node, int index) {
  int sum;
  double info;
  landmark *tmp;

  // max resolution: 10mm
  if (node->x_max - node->x_min > 10 &&
      node->y_max - node->y_min > 10) {
    tmp = node->landmarks + index;

    // check for split if more than 10 observations and there is disagreement
    sum = tmp->seen + tmp->unseen;
    if (sum > 10 && tmp->seen != 0 && tmp->unseen != 0) {
      info = landmark_get_info(*tmp);

      // split if info is less than 80% certain
      // n/(p(1-p)) = n/(0.8*0.2)
      if (info < sum/0.16)
	map_node_split(node, index);
    }
  }
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

      // use parent if new
      // (same as map_get_info)
      if (sum < 5)
	value = (int)(255 * node->landmark.seen/(double)(node->landmark.seen + node->landmark.unseen));
      else
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

int map_get_size(map_node *node) {
  int size, i;

  size = 0;

  for (i = 0; i < 4; i++)
    if (node->children[i] == NULL)
      size++;
    else
      size += map_get_size(node->children[i]);

  return size;
}

double map_get_info(map_node *node) {
  int i;
  double info;

  info = 0.0;
  for (i = 0; i < 4; i++)
    if (node->children[i] == NULL) {
      // leaf node
      // use parent info if new
      if (node->landmarks[i].seen + node->landmarks[i].unseen < 5)
	info += landmark_get_info(node->landmark);
      else
	info += landmark_get_info(node->landmarks[i]);
    } else
      // interior node
      info += map_get_info(node->children[i]);

  return info;
}
