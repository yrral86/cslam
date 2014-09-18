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
  n->new = 1;

  // init landmarks and children
  for (i = 0; i < 9; i++) {
    n->landmarks[i].seen = 0;
    n->landmarks[i].unseen = 0;
    n->children[i] = NULL;
  }

  n->landmark.seen = 0;
  n->landmark.unseen = 0;

  return n;
}

void map_node_spawn_child(map_node *node, int index) {
  int x_min, x_max, y_min, y_max;

  map_node_ranges_from_index(node, index, &x_min, &x_max, &y_min, &y_max);

  node->children[index] = map_node_new(x_min, x_max, y_min, y_max);
}

void map_merge(map_node *all, map_node *latest, int dx, int dy, int dt) {
  // copy latest into all
  int i, j, x, y, x_min, x_max, y_min, y_max;
  double r, theta;
  /*
  for (i = 0; i < 9; i++) {
    if (latest->children[i] == NULL) {
      // leaf
      // find center of region
      //      map_node_ranges_from_index(latest, i, &x_min, &x_max, &y_min, &y_max);
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
  */
}

int map_node_index_from_x_y(map_node *node, int x, int y) {
  int index, left, right, top, bottom;

  // left = x - 1/6 of width
  left = node->x - (node->x_max - node->x_min)/6;
  // right = x + 1/6 of width
  right = node->x + (node->x_max - node->x_min)/6;
  // bottom = y - 1/6 of height
  bottom = node->y - (node->y_max - node->y_min)/6;
  // top = y + 1/6 of height
  top = node->y + (node->y_max - node->y_min)/6;

  // find quadrant
  if (x < left && y >= top)
    //top left
    index = 0;
  else if (x >= left && x < right && y >= top)
    //top mid
    index = 1;
  else if (x >= right && y >= top)
    // top right
    index = 2;
  else if (x < left && y >= bottom && y < top)
    //mid left
    index = 3;
  else if (x >= left && x < right && y >= bottom && y < top)
    //mid mid
    index = 4;
  else if (x >= right && y >= bottom && y < top)
    //mid right
    index = 5;
  else if (x < left && y < bottom)
    // bottom left
    index = 6;
  else if (x >= left && x < right && y < bottom)
    //bottom mid
    index = 7;
  else if (x >= right && y < bottom)
    // bottom right
    index = 8;
  else
    assert(0);

  return index;
}

void map_node_ranges_from_index(map_node *node, int index, int *x_min, int *x_max, int *y_min, int *y_max) {
  int left, right, bottom, top;

  // initialize indices to center (we are plotting an cell with no data)
  if (node->new) {
    node->x = (node->x_min + node->x_max)/2;
    node->y = (node->y_min + node->y_max)/2;
  }

  // left = x - 1/6 of width
  left = node->x - (node->x_max - node->x_min)/6;
  // right = x + 1/6 of width
  right = node->x + (node->x_max - node->x_min)/6;
  // bottom = y - 1/6 of height
  bottom = node->y - (node->y_max - node->y_min)/6;
  // top = y + 1/6 of height
  top = node->y + (node->y_max - node->y_min)/6;

  switch (index) {
  case 0:
    // top left
    *x_min = node->x_min;
    *x_max = left - 1;
    *y_min = top;
    *y_max = node->y_max;
    break;
  case 1:
    // top mid
    *x_min = left;
    *x_max = right - 1;
    *y_min = top;
    *y_max = node->y_max;
    break;
  case 2:
    // top right
    *x_min = right;
    *x_max = node->x_max;
    *y_min = top;
    *y_max = node->y_max;
    break;
  case 3:
    // mid left
    *x_min = node->x_min;
    *x_max = left - 1;
    *y_min = bottom;
    *y_max = top - 1;
    break;
  case 4:
    // mid mid
    *x_min = left;
    *x_max = right - 1;
    *y_min = bottom;
    *y_max = top - 1;
    break;
  case 5:
    // mid right
    *x_min = right;
    *x_max = node->x_max;
    *y_min = bottom;
    *y_max = top - 1;
    break;
  case 6:
    // bottom left
    *x_min = node->x_min;
    *x_max = left - 1;
    *y_min = node->y_min;
    *y_max = bottom - 1;
    break;
  case 7:
    // bottom mid
    *x_min = left;
    *x_max = right - 1;
    *y_min = node->y_min;
    *y_max = bottom - 1;
    break;
  case 8:
    // bottom right
    *x_min = right;
    *x_max = node->x_max;
    *y_min = node->y_min;
    *y_max = bottom - 1;
    break;
  default:
    assert(0);
  }
}

/*
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
*/

void map_deallocate(map_node *map) {
  int i;

  // deallocate children
  for (i = 0; i < 9; i++)
    if (map->children[i] != NULL)
      map_deallocate(map->children[i]);

  free(map);
}

void map_set_seen(map_node *map, int x, int y) {
  int index;

  // initialize indices
  if (map->new) {
    map->x = x;
    map->y = y;
    map->new = 0;
  }

  (map->landmark.seen)++;

  index = map_node_index_from_x_y(map, x, y);

  if (map->children[index] == NULL) {
    if (map->x_max - map->x_min > 30 || map->y_max - map->y_min > 30) {
      // need to spawn child
      map_node_spawn_child(map, index);
      map_set_seen(map->children[index], x, y);
    } else
      // we are at a leaf, record sublandmark
      map->landmarks[index].seen++;
  } else
    // interior node, traverse
    map_set_seen(map->children[index], x, y);
}

void map_set_unseen(map_node *map, int x, int y) {
  int index;

  // initialize indices
  if (map->new) {
    map->x = x;
    map->y = y;
    map->new = 0;
  }

  (map->landmark.unseen)++;

  index = map_node_index_from_x_y(map, x, y);

  if (map->children[index] == NULL) {
    if (map->x_max - map->x_min > 30 || map->y_max - map->y_min > 30) {
      // need to spawn child
      map_node_spawn_child(map, index);
      map_set_unseen(map->children[index], x, y);
    } else
      // we are at a leaf, record sublandmark
      map->landmarks[index].unseen++;
  } else
    // interior node, traverse
    map_set_unseen(map->children[index], x, y);
}

/*
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
*/

void map_write_buffer(map_node *map, uint8_t *buffer) {
  // clear buffer
  bzero(buffer, (map->x_max - map->x_min + 1)*(map->y_max - map->y_min + 1));
  // write nodes
  map_node_write_buffer(map, buffer);
}

void map_node_write_buffer(map_node *node, uint8_t *buffer) {
  int x_min, x_max, y_min, y_max, sum, value, i, x, y;

  for (i = 0; i < 9; i++) {
    if (node->children[i] == NULL) {
      // leaf node

      // determine value
      sum = node->landmarks[i].seen + node->landmarks[i].unseen;

      // use 0 if no data
      if (sum < 1)
	value = 0;
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

  for (i = 0; i < 9; i++)
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
  for (i = 0; i < 9; i++)
    if (node->children[i] == NULL) {
      // leaf node
      info += landmark_get_info(node->landmarks[i]);
    } else
      // interior node
      info += map_get_info(node->children[i]);

  return info;
}
