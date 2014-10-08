#include "map.h"

static int width, height;

map_node* map_expand(map_node *map) {
  int w, h;
  w = width;
  h = height;
  map_node *new = map_new(w * 2, h * 2);
  map_merge(new, map, w, h, 0);
  return new;
}

map_node* map_new(int w, int h) {
  width = w + 1;
  height = h + 1;
  return map_node_new(0, width, 0, height);
}

map_node* map_new_from_observation(int *distances) {
  map_node* map;
  int i, j, d, x, y, max = 0;
  double s, c, theta;


  for (i = 0; i < RAW_SENSOR_DISTANCES_USB; i++)
    if (distances[i] > max) max = distances[i];

  max += 1;

  x = max;
  y = max;

  max *= 2;

  /*
    map = map_new(max, max);
  */

  map = map_node_new(0, max, 0, max);

  // record observations
  //  theta = -SENSOR_RANGE_USB*M_PI/360.0;
  //  dtheta = SENSOR_SPACING_USB*M_PI/180;
  for (i = 0; i < RAW_SENSOR_DISTANCES_USB; i++) {
    //    if (i > 0) theta += dtheta;
    theta = (-SENSOR_RANGE_USB/2.0 + i*SENSOR_SPACING_USB)*M_PI/180;
    c = cos(theta);
    s = sin(theta);
    d = distances[i];
    //    printf("d: %i s: %g c: %g theta: %g\n", d, s, c, theta);
    //    printf("x,y: (%g, %g)\n", x+d*c, max - y + d*s);
    map_set_seen(map, x + d*c, y + d*s);
    for (j = d - 30; j >= 30; j -= 30)
      map_set_unseen(map, x + (d-j)*c, y + (d-j)*s);
  }

  return map;
}

map_node* map_node_new(int x_min, int x_max, int y_min, int y_max) {
  int i;
  map_node *n = malloc(sizeof(map_node));
  // init ranges
  n->x_min = x_min;
  n->x_max = x_max;
  n->y_min = y_min;
  n->y_max = y_max;
  n->out_of_bounds = 0;
  n->new = 1;

  // init landmarks and children
  for (i = 0; i < 4; i++) {
    n->landmarks[i].seen = 0;
    n->landmarks[i].unseen = 0;
    n->children[i] = NULL;
  }

  n->landmark.seen = 0;
  n->landmark.unseen = 0;

  n->root = n;

  return n;
}

map_node* map_dup(map_node *map) {
  int i;
  map_node *new = map_node_new(map->x_min, map->x_max, map->y_min, map->y_max);

  for (i = 0; i < 4; i++) {
    if (map->children[i] == NULL) {
      // leaf
      new->landmarks[i] = map->landmarks[i];
    } else {
      // interior
      new->children[i] = map_dup(map->children[i]);
      new->children[i]->root = new->root;
    }
  }

  return new;
}

void map_node_spawn_child(map_node *node, int index) {
  int x_min, x_max, y_min, y_max;

  map_node_ranges_from_index(node, index, &x_min, &x_max, &y_min, &y_max);

  node->children[index] = map_node_new(x_min, x_max, y_min, y_max);
  node->children[index]->root = node->root;
}

void map_merge(map_node *all, map_node *latest, int dx, int dy, int dt) {
  // copy latest into all
  int i, j, x, y, x_min, x_max, y_min, y_max;
  double r, theta, dtheta;

  dtheta = dt*M_PI/180;

  // for now assume aligned: (dx,dy,dt)=(0,0,0)
  for (i = 0; i < 4; i++)
    if (latest->children[i] == NULL) {
      // leaf
      // find center of region
      map_node_ranges_from_index(latest, i, &x_min, &x_max, &y_min, &y_max);
      x = (x_max + x_min)/2;
      y = (y_max + y_min)/2;

      // project to origin at middle of latest map
      x -= (latest->root->x_min + latest->root->x_max)/2;
      y -= (latest->root->y_min + latest->root->y_max)/2;

      // calculate radius and angle
      r = sqrt(x*x + y*y);
      theta = atan2(y, x);

      // reproject to center of all map using dx, dy, dtheta
      x = dx + r*cos(theta + dtheta);
      y = dy + r*sin(theta + dtheta);

      // plot seen/unseen at (x, y)
      for (j = 0; j < latest->landmarks[i].seen; j++)
	map_set_seen(all, x, y);
      for (j = 0; j < latest->landmarks[i].unseen; j++)
	map_set_unseen(all, x, y);
    } else
      map_merge(all, latest->children[i], dx, dy, dt);
}

int map_node_index_from_x_y(map_node *node, int x, int y) {
  int index, mid_x, mid_y;

  assert(node->new == 0);

  mid_x = (node->x_max + node->x_min)/2;
  mid_y = (node->y_max + node->y_min)/2;

  assert(x >= node->x_min && x <= node->x_max);
  assert(y >= node->y_min && y <= node->y_max);

  // find quadrant
  if (x < mid_x && y >= mid_y)
    //top left
    index = 0;
  else if (x >= mid_x && y >= mid_y)
    // top right
    index = 1;
  else if (x < mid_x && y < mid_y)
    // bottom left
    index = 2;
  else if (x >= mid_x && y < mid_y)
    // bottom right
    index = 3;
  else
    assert(0);

  return index;
}

void map_node_ranges_from_index(map_node *node, int index, int *x_min, int *x_max, int *y_min, int *y_max) {
  int mid_x, mid_y;

  assert(node->new == 0);

  mid_x = (node->x_max + node->x_min)/2;
  mid_y = (node->y_max + node->y_min)/2;

  switch (index) {
  case 0:
    // top left
    *x_min = node->x_min;
    *x_max = mid_x - 1;
    *y_min = mid_y;
    *y_max = node->y_max;
    break;
  case 1:
    // top right
    *x_min = mid_x;
    *x_max = node->x_max;
    *y_min = mid_y;
    *y_max = node->y_max;
    break;
  case 2:
    // bottom left
    *x_min = node->x_min;
    *x_max = mid_x - 1;
    *y_min = node->y_min;
    *y_max = mid_y - 1;
    break;
  case 3:
    // bottom right
    *x_min = mid_x;
    *x_max = node->x_max;
    *y_min = node->y_min;
    *y_max = mid_y - 1;
    break;
  default:
    assert(0);
  }

  if (*x_min < node->x_min || *x_max > node->x_max)
    printf("oh noes: x_min: %i x_max: %i node->x_min: %i node->x_max: %i\n", *x_min, *x_max, node->x_min, node->x_max);
  if (*y_min < node->y_min || *y_max > node->y_max)
    printf("oh noes: y_min: %i y_max: %i node->y_min: %i node->y_max: %i\n", *y_min, *y_max, node->y_min, node->y_max);
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
  for (i = 0; i < 4; i++)
    if (map->children[i] != NULL)
      map_deallocate(map->children[i]);

  free(map);
}

void map_set_seen(map_node *map, int x, int y) {
  int index;

  //  printf("seen: x_min: %i x_max: %i y_min %i y_max: %i x: %i y: %i\n",
  //	 map->x_min, map->x_max, map->y_min, map->y_max, x, y);
  //  fflush(stdout);
  if (x < 0) {
    x = 0;
    map->out_of_bounds++;
  } else if (x > width) {
    x = width;
    map->out_of_bounds++;
  }

  if (y < 0) {
    y = 0;
    map->out_of_bounds++;
  } else if (y > height) {
    y = height;
    map->out_of_bounds++;
  }

  // initialize indices
  if (map->new) {
    //    printf("new map\n");
    map->x = x;
    map->y = y;
    map->new = 0;
  }

  (map->landmark.seen)++;

  index = map_node_index_from_x_y(map, x, y);
  //  printf("index: %i\n", index);

  if (map->children[index] == NULL) {
    if (map->x_max - map->x_min > 30 || map->y_max - map->y_min > 30) {
      // need to spawn child
      map_node_spawn_child(map, index);
      //      printf("spawned child, setting seen\n");
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

  if (x < 0) {
    x = 0;
    map->out_of_bounds++;
  } else if (x > width) {
    x = width;
    map->out_of_bounds++;
  }

  if (y < 0) {
    y = 0;
    map->out_of_bounds++;
  } else if (y > height) {
    y = height;
    map->out_of_bounds++;
  }

  //  printf("unseen: x_min: %i x_max: %i y_min %i y_max: %i x: %i y: %i\n",
  //	 map->x_min, map->x_max, map->y_min, map->y_max, x, y);

  // initialize indices
  if (map->new) {
    map->x = x;
    map->y = y;
    map->new = 0;
  }

  (map->landmark.unseen)++;

  index = map_node_index_from_x_y(map, x, y);

  if (map->children[index] == NULL) {
    // do not drill down for unseen
    /*    if (map->x_max - map->x_min > 30 || map->y_max - map->y_min > 30) {
      // need to spawn child
      map_node_spawn_child(map, index);
      map_set_unseen(map->children[index], x, y);
      } else*/
      // we are at a leaf, record sublandmark
      map->landmarks[index].unseen++;
  } else
    // interior node, traverse
    map_set_unseen(map->children[index], x, y);
}

double map_seen_probability(map_node *map, int x, int y) {
  int index = map_node_index_from_x_y(map, x, y);
  double sum, p;

  if (map->children[index] == NULL) {
    // leaf node
    sum = map->landmarks[index].seen + map->landmarks[index].unseen;
    // 50/50 when no information
    p = 0.5;
    if (sum > 0)
      p = map->landmarks[index].seen/sum;

    // max certainty is 99%
    if (p > 0.99) p = 0.99;

    return p;
  } else
    // interior
    return map_seen_probability(map->children[index], x, y);
}

double map_unseen_probability(map_node *map, int x, int y) {
  return 1 - map_seen_probability(map, x, y);
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
  bzero(buffer, width*height);
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

  // we plotted over the edge
  if (node->out_of_bounds > 0)
    size += 1000*node->out_of_bounds;

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
      info += landmark_get_info(node->landmarks[i]);
    } else
      // interior node
      info += map_get_info(node->children[i]);

  return info;
}

void map_debug(map_node *map) {
  int i;
  printf("x_min: %i x_max: %i y_min: %i y_max: %i x: %i y: %i\n",
	 map->x_min, map->x_max, map->y_min, map->y_max, map->x, map->y);

  for (i = 0; i < 4; i++)
    if (map->children[i] == NULL)
      printf("seen: %i unseen: %i\n", map->landmarks[i].seen, map->landmarks[i].unseen);
    else
      map_debug(map->children[i]);
}
