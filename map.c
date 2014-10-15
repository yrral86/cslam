#include "map.h"

static int width, height;
/*
map_node* map_expand(map_node *map) {
  int w, h;
  w = width;
  h = height;
  map_node *new = map_new(w * 2, h * 2);
  map_merge(new, map, w, h, 0);
  return new;
}
*/

map_node* map_new(int w, int h) {
  map_node *map;
  width = w + 1;
  height = h + 1;
#ifdef __MAP_TYPE_TREE__
  map = map_node_new(0, width, 0, height);
#endif
#ifdef __MAP_TYPE_HEAP__
  map = malloc(sizeof(map_node));
  // initial max size 1/10 of possible pixels
  map->max_size = MAP_SIZE*MAP_SIZE/10;
  map->current_size = 0;
  map->heap_sorted = 0;
  map->heap = malloc(sizeof(map_pixel)*map->max_size);
  bzero(map->heap, sizeof(map_pixel)*map->max_size);
#endif
  return map;
}

#ifdef __MAP_TYPE_TREE__
map_node* map_new_from_observation(int *distances) {
  int x, y, max = 0;
#endif
#ifdef __MAP_TYPE_HEAP__
map_node* map_new_from_hypothesis(hypothesis h) {
  map_pixel pixel;
#endif
  map_node* map;
  int i, j, d;
  double s, c, theta;

#ifdef __MAP_TYPE_TREE__
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
      map_set_unseen(map, x + j*c, y + j*s);
  }

#endif
#ifdef __MAP_TYPE_HEAP__
  map = map_new(width - 1, height - 1);

  for (i = 0; i < RAW_SENSOR_DISTANCES_USB; i++) {
    // observation theta + pose theta
    theta = (h.obs->list[i].theta + h.theta)*M_PI/180;
    c = cos(theta);
    s = sin(theta);
    d = h.obs->list[i].r;
    pixel.x = h.x + d*c;
    pixel.y = h.y + d*s;
    pixel.l.seen = 1;
    pixel.l.unseen = 0;
    pixel.h = h;
    pixel.obs_index = i;
    pixel.x /= BUFFER_FACTOR;
    pixel.y /= BUFFER_FACTOR;
    map_add_pixel(map, pixel);
    for (j = d - BUFFER_FACTOR; j >= BUFFER_FACTOR; j -= BUFFER_FACTOR) {
      pixel.x = h.x + j*c;
      pixel.y = h.y + j*s;
      pixel.l.seen = 0;
      pixel.l.unseen = 1;
      pixel.h = h;
      pixel.obs_index = i;
      pixel.x /= BUFFER_FACTOR;
      pixel.y /= BUFFER_FACTOR;
      map_add_pixel(map, pixel);
    }
  }

  map = map_sort(map);
#endif

  return map;
}

#ifdef __MAP_TYPE_HEAP__
void map_add_pixel(map_node *map, map_pixel p) {
  if (map->current_size + 1 <= map->max_size) {
    map->heap[map->current_size] = p;
    map->current_size++;
    map_reheapify_up(map);
    map->heap_sorted = 0;
  } else {
    map_double_max_size(map);
    map_add_pixel(map, p);
  }
}

void map_double_max_size(map_node *map) {
  int new_size = 2*map->max_size;
  map_pixel *heap = map->heap;

  // shouldn't modify heap when sorted
  assert(map->heap_sorted == 0);

  map->heap = malloc(sizeof(map_pixel)*new_size);
  bzero(map->heap, sizeof(map_pixel)*new_size);
  memcpy(map->heap, heap, sizeof(map_pixel)*map->max_size);
  map->max_size = new_size;
}

void map_reheapify_up(map_node *map) {
  int child = map->current_size - 1;
  int parent = map_parent_index(child);

  // shouldn't modify heap when sorted
  assert(map->heap_sorted == 0);  

  map_pixel pixel;
  while (child > 0 && map_pixel_need_swap(map->heap[parent], map->heap[child])) {
    if (map->heap[child].x == map->heap[parent].x &&
	map->heap[child].y == map->heap[parent].y) {
      // merge child into parent
      pixel = map->heap[child];
      pixel.l.seen += map->heap[parent].l.seen;
      pixel.l.unseen += map->heap[parent].l.unseen;
      map->heap[parent] = pixel;

      // copy last element into child
      map->current_size--;
      map->heap[child] = map->heap[map->current_size];

      // reheapify_down starting at child
      map_reheapify_down_root(map, child);
    } else {
      pixel = map->heap[parent];
      map->heap[parent] = map->heap[child];
      map->heap[child] = pixel;
      child = parent;
      parent = map_parent_index(child);
    }
  }
}

void map_reheapify_down(map_node *map) {
  map_reheapify_down_root(map, 0);
}

void map_reheapify_down_root(map_node *map, int parent) {
  map_pixel p;

  // shouldn't modify heap when sorted
  assert(map->heap_sorted == 0);

  int child_left = map_left_index(parent);
  int child_right = map_right_index(parent);
  while ((child_left < map->current_size &&
	  map_pixel_need_swap(map->heap[parent], map->heap[child_left])) ||
	 (child_right < map->current_size &&
	  map_pixel_need_swap(map->heap[parent], map->heap[child_right]))) {
    // need to swap left or right for parent
    p = map->heap[parent];
    // if we have two children
    if (child_right < map->current_size &&
	// check order
	map_pixel_need_swap(map->heap[child_left], map->heap[child_right])) {
      // child_right should be parent
      map->heap[parent] = map->heap[child_right];
      map->heap[child_right] = p;
      parent = child_right;
    } else {
      // child_left should be parent
      map->heap[parent] = map->heap[child_left];
      map->heap[child_left] = p;
      parent = child_left;
    }
    child_left = map_left_index(parent);
    child_right = map_right_index(parent);
  }
}

inline int map_pixel_need_swap(map_pixel parent, map_pixel child) {
  if (parent.y > child.y ||
      (parent.y == child.y &&
       parent.x > child.x))
    //    if (parent.h.obs->list[parent.obs_index].r >
    //	child.h.obs->list[child.obs_index].r)
    return 1;
  else
    return 0;
}

inline int map_parent_index(int i) {
  return i/2;
}

inline int map_left_index(int i) {
  return 2*i+1;
}

inline int map_right_index(int i) {
  return 2*i + 2;
}

map_pixel map_pop_pixel(map_node *map) {
  // can't pop from empty heap
  assert(map->current_size > 0);
  map_pixel p;
  map->current_size--;
  if (map->heap_sorted == 1)
    p = map->heap[map->index++];
  else {
    p = map->heap[0];
    map->heap[0] = map->heap[map->current_size];
    map_reheapify_down(map);
  }
  return p;
}
/*
double map_merge_variance(map_node *m1, map_node *m2) {
  double variance = 0.0;
  int one_done = 0, two_done = 0, merge = 0;
  map_pixel one, two, merged;
  m1 = map_dup(m1);
  m2 = map_dup(m2);

  if (m1->current_size > 0)
    one = map_pop_pixel(m1);
  else {
    one_done = 1;
    one.x = MAP_SIZE+10;
    one.y = MAP_SIZE+10;
    one.l.seen = 0;
    one.l.unseen = 0;
  }

  if (m2->current_size > 0)
    two = map_pop_pixel(m2);
  else {
    two_done = 1;
    two.x = MAP_SIZE+10;
    two.y = MAP_SIZE+10;
    two.l.seen = 0;
    two.l.unseen = 0;
  }

  merged.x = 0;
  merged.y = 0;
  merged.l.seen = 0;
  merged.l.unseen = 0;
  while (one_done == 0 || two_done == 0) {
    printf("m1->current_size: %i\nm2->current_size: %i\n", m1->current_size, m2->current_size);
    printf("one = (%d,%d,%d,%d) two = (%d,%d,%d,%d) merged = (%d,%d,%d,%d)\n",
	   one.x, one.y, one.l.seen, one.l.unseen,
	   two.x, two.y, two.l.seen, two.l.unseen,
	   merged.x, merged.y, merged.l.seen, merged.l.unseen);
    if (one_done == 0 && one.x == merged.x &&
	one.y == merged.y) {
      merged.l.seen += one.l.seen;
      merged.l.unseen += one.l.unseen;
      if (m1->current_size > 0)
	one = map_pop_pixel(m1);
      else {
	one_done = 1;
	one.x = MAP_SIZE+10;
	one.y = MAP_SIZE+10;
	one.l.seen = 0;
	one.l.unseen = 0;
      }
      merge = 1;
    }

    if (two_done == 0 && two.x == merged.x &&
	two.y == merged.y) {
      merged.l.seen += two.l.seen;
      merged.l.unseen += two.l.unseen;
      if (m2->current_size > 0)
	two = map_pop_pixel(m2);
      else {
	two_done = 1;
	two.x = MAP_SIZE+10;
	two.y = MAP_SIZE+10;
	two.l.seen = 0;
	two.l.unseen = 0;
      }
      merge = 1;
    }

    if (merge == 0) {
      // nothing matched, record variance
      if (merged.l.seen + merged.l.unseen > 0)
	variance += merged.l.seen*merged.l.unseen/(double)(merged.l.seen+merged.l.unseen);

      // set next merged target to smallest of one/two
      if (one_done || map_pixel_need_swap(one, two)) {
	// two should be merged next
	merged = two;
	if (m2->current_size > 0)
	  two = map_pop_pixel(m2);
	else {
	  two_done = 1;
	  two.x = MAP_SIZE+10;
	  two.y = MAP_SIZE+10;
	  two.l.seen = 0;
	  two.l.unseen = 0;
	}
      } else if (one_done == 0) {
	// one should be merged next
	merged = one;
	if (m1->current_size > 0)
	  one = map_pop_pixel(m1);
	else {
	  one_done = 1;
	  one.x = MAP_SIZE+10;
	  one.y = MAP_SIZE+10;
	  one.l.seen = 0;
	  one.l.unseen = 0;
	}
      }
    } else
      merge = 0;
  }

  if (merged.l.seen + merged.l.unseen > 0)
    variance += merged.l.seen*merged.l.unseen/(double)(merged.l.seen+merged.l.unseen);

  map_deallocate(m1);
  map_deallocate(m2);

  return variance;
}
*/
map_node* map_sort(map_node *map) {
  map_node *new = map_new(width -1, height - 1);
  while (map->current_size > 0)
    map_add_pixel(new, map_pop_pixel(map));

  new->heap_sorted = 1;
  new->index = 0;

  map_deallocate(map);

  return new;
}

map_node* map_merge(map_node *m1, hypothesis h) {
  map_node *new;
  map_node *m2 = map_new_from_hypothesis(h);
  map_pixel next;

  if (m1->current_size == 0) {
    map_deallocate(m1);
    return m2;
  }

  assert(m1->heap_sorted == 1);

  new = map_new(width - 1, height - 1);

  do {
    // find next target
    if (m1->current_size > 0 && m2->current_size == 0) {
      // m2 empty, use m1
      next = m1->heap[m1->index++];
      m1->current_size--;
    } else if (m1->current_size == 0 && m2->current_size > 0) {
      // m1 empty, use m2
      next = m2->heap[m2->index++];
      m2->current_size--;
    } else if (map_pixel_need_swap(m1->heap[m1->index], m2->heap[m2->index])) {
      // m2 lower or equal
      next = m2->heap[m2->index++];
      m2->current_size--;
    } else {
      // m1 lower
      next = m1->heap[m1->index++];
      m1->current_size--;
    }

    // check for m1 merge targets
    while (m1->current_size > 0 &&
	   m1->heap[m1->index].x == next.x &&
	   m1->heap[m1->index].y == next.y) {
      next.l.seen += m1->heap[m1->index].l.seen;
      next.l.unseen += m1->heap[m1->index].l.unseen;
      m1->index++;
      m1->current_size--;
    }

    // check for m2 merge targets
    while (m2->current_size > 0 &&
	   m2->heap[m2->index].x == next.x &&
	   m2->heap[m2->index].y == next.y) {
      next.l.seen += m2->heap[m2->index].l.seen;
      next.l.unseen += m2->heap[m2->index].l.unseen;
      // TODO: make list of hypotheses
      //    pixel.h = h;
      //    pixel.obs_index = i;
      m2->index++;
      m2->current_size--;
    }

    map_add_pixel(new, next);
  } while (m1->current_size > 0 || m2->current_size > 0);

  map_deallocate(m1);
  map_deallocate(m2);

  new->heap_sorted = 1;
  new->index = 0;

  return new;
}

double map_variance(map_node *map) {
  int i;
  double variance = 0.0;

  for (i = 0; i < map->current_size; i++) {
    if (map->heap[i].l.seen + map->heap[i].l.unseen > 0)
      variance += map->heap[i].l.seen*map->heap[i].l.unseen/(double)(map->heap[i].l.seen + map->heap[i].l.unseen);
  }

  return variance;
}

#endif

#ifdef __MAP_TYPE_TREE__
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
#endif

map_node* map_dup(map_node *map) {
  map_node *new;
#ifdef __MAP_TYPE_TREE__
  int i;

  new = map_node_new(map->x_min, map->x_max, map->y_min, map->y_max);

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
#endif
#ifdef __MAP_TYPE_HEAP__
  new = map_new(width - 1, height - 1);
  // make it big enough
  while (new->max_size < map->max_size)
    map_double_max_size(new);
  // copy
  new->heap_sorted = map->heap_sorted;
  new->current_size += map->index;
  new->index = 0;
  memcpy(new->heap, map->heap, sizeof(map_pixel)*map->current_size);
  new->current_size = map->current_size;
#endif

  return new;
}

#ifdef __MAP_TYPE_TREE__
void map_node_spawn_child(map_node *node, int index) {
  int x_min, x_max, y_min, y_max;

  map_node_ranges_from_index(node, index, &x_min, &x_max, &y_min, &y_max);

  node->children[index] = map_node_new(x_min, x_max, y_min, y_max);
  node->children[index]->root = node->root;
}
#endif

#ifdef __MAP_TYPE_TREE__
void map_merge(map_node *all, map_node *latest, int dx, int dy, int dt) {
  // copy latest into all
  int i, j, x, y, x_min, x_max, y_min, y_max;
  double r, theta, dtheta;

  dtheta = dt*M_PI/180;

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
      map_increase_seen(all, x, y, latest->landmarks[i].seen);
      map_increase_unseen(all, x, y, latest->landmarks[i].unseen);	
    } else
      map_merge(all, latest->children[i], dx, dy, dt);
}

void map_merge_aligned(map_node *all, map_node *latest) {
    // copy latest into all
  int i, j, x, y, x_min, x_max, y_min, y_max;
  double r, theta;

  for (i = 0; i < 4; i++)
    if (latest->children[i] == NULL) {
      // leaf
      // find center of region
      map_node_ranges_from_index(latest, i, &x_min, &x_max, &y_min, &y_max);
      x = (x_max + x_min)/2;
      y = (y_max + y_min)/2;

      map_increase_seen(all, x, y, latest->landmarks[i].seen);
      map_increase_unseen(all, x, y, latest->landmarks[i].unseen);
    } else
      map_merge_aligned(all, latest->children[i]);
}

int map_node_index_from_x_y(map_node *node, int x, int y) {
  int index, mid_x, mid_y;

  mid_x = (node->x_max + node->x_min)/2;
  mid_y = (node->y_max + node->y_min)/2;

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
#endif

void map_deallocate(map_node *map) {
#ifdef __MAP_TYPE_TREE__
  int i;

  // deallocate children
  for (i = 0; i < 4; i++)
    if (map->children[i] != NULL)
      map_deallocate(map->children[i]);
#endif
#ifdef __MAP_TYPE_HEAP__
  free(map->heap);
#endif
  free(map);
}

#ifdef __MAP_TYPE_TREE__
void map_set_seen(map_node *map, int x, int y) {
  map_increase_seen(map, x, y, 1);
}

void map_increase_seen(map_node *map, int x, int y, int increase) {
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

  map->landmark.seen += increase;

  index = map_node_index_from_x_y(map, x, y);
  //  printf("index: %i\n", index);

  if (map->children[index] == NULL) {
    if (map->x_max - map->x_min > 30 || map->y_max - map->y_min > 30) {
      // need to spawn child
      map_node_spawn_child(map, index);
      //      printf("spawned child, setting seen\n");
      map_increase_seen(map->children[index], x, y, increase);
    } else
      // we are at a leaf, record sublandmark
      map->landmarks[index].seen += increase;
  } else
    // interior node, traverse
    map_increase_seen(map->children[index], x, y, increase);
}

void map_set_unseen(map_node *map, int x, int y) {
  map_increase_unseen(map, x, y, 1);
}

void map_increase_unseen(map_node *map, int x, int y, int increase) {
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

  map->landmark.unseen += increase;

  index = map_node_index_from_x_y(map, x, y);

  if (map->children[index] == NULL) {
    // do not drill down for unseen
    /*    if (map->x_max - map->x_min > 30 || map->y_max - map->y_min > 30) {
      // need to spawn child
      map_node_spawn_child(map, index);
      map_set_unseen(map->children[index], x, y);
      } else*/
      // we are at a leaf, record sublandmark
    map->landmarks[index].unseen += increase;
  } else
    // interior node, traverse
    map_increase_unseen(map->children[index], x, y, increase);
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
#endif

void map_write_buffer(map_node *map, uint8_t *buffer) {
  // clear buffer
//  bzero(buffer, width*height);
  // set buffer to 127
  memset(buffer, 127, width*height);
#ifdef __MAP_TYPE_TREE__
  // write nodes
  map_node_write_buffer(map, buffer);
#endif
#ifdef __MAP_TYPE_HEAP__
  map_pixel p;
  int i, j, sum, value;
  map = map_dup(map);
  while(map->current_size > 0) {
    p = map_pop_pixel(map);

    sum = p.l.seen + p.l.unseen;
    if (sum >= 1) {
      value = (int)(255 * p.l.seen/(double)sum);

      // write pixels with value
      for (i = p.y*BUFFER_FACTOR; i < BUFFER_FACTOR*(p.y + 1); i++)
	for (j = p.x*BUFFER_FACTOR; j < BUFFER_FACTOR*(p.x + 1); j++)
	  buffer[width*i + j] = value;
    }
  }
  map_deallocate(map);
#endif
}

#ifdef __MAP_TYPE_TREE__
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
#endif

void map_debug(map_node *map) {
  int i;
#ifdef __MAP_TYPE_HEAP__
  map_pixel p;
  for (i = 0; i < map->current_size; i++) {
    p = map->heap[i];
    printf("(%d,%d,%d,%d,%d,%d)\n", p.x, p.y, p.l.seen, p.l.unseen,
	     p.h.obs->list[p.obs_index].r, p.h.obs->list[p.obs_index].theta);
  }
#endif
#ifdef __MAP_TYPE_TREE__
  printf("x_min: %i x_max: %i y_min: %i y_max: %i x: %i y: %i\n",
	 map->x_min, map->x_max, map->y_min, map->y_max, map->x, map->y);

  for (i = 0; i < 4; i++)
    if (map->children[i] == NULL)
      printf("seen: %i unseen: %i\n", map->landmarks[i].seen, map->landmarks[i].unseen);
    else
      map_debug(map->children[i]);
#endif
}

