#include "landmark.h"
#include "buffer.h"

landmark_map* landmark_map_copy(landmark_map *parent) {
  landmark_map *head;
  if (parent == NULL) {
    head = landmark_map_init(buffer_get_size());
  } else {
    head = parent;
    landmark_map_reference(head);
  }
  assert(head != NULL);
  assert(head->map != NULL);
  return head;
}

void landmark_map_free(landmark_map *node) {
  assert(node != NULL);
  free(node);
}

void landmark_map_reference(landmark_map *node) {
  assert(node != NULL);
  assert(node->references > 0);
  node->references++;
}

void landmark_map_dereference(landmark_map *node) {
  assert(node != NULL);
  assert(node->references > 0);
  node->references--;
  if (node->references < 1)
    landmark_map_free(node);
}

landmark_map* landmark_map_init(int size) {
  int i;
  landmark_map *node = malloc(sizeof(landmark_map));
  node->references = 1;
  node->map = malloc(sizeof(landmark)*size);
  for (i = 0; i < size; i++) {
    node->map[i].x = x_from_buffer_index(i);
    node->map[i].y = y_from_buffer_index(i);
    node->map[i].seen = 0;
    node->map[i].unseen = 0;
  }
  return node;
}

void landmark_set_seen(landmark_map *node, int index) {
  assert(node != NULL);
  node->map[index].seen++;
}

void landmark_set_seen_value(landmark_map *node, int index, int value) {
  assert(node != NULL);
  assert(node->map != NULL);
  node->map[index].seen = value;
}

void landmark_set_unseen(landmark_map *node, int index) {
  assert(node != NULL);
  assert(node->map != NULL);
  node->map[index].unseen++;
}

void landmark_set_unseen_value(landmark_map *node, int index, int value) {
  assert(node != NULL);
  node->map[index].unseen = value;
}

// writes a byte buffer given the head of a landmark tree
void landmark_write_map(landmark_map *head, uint8_t *buffer) {
  memset(buffer, '\0', buffer_get_size()*sizeof(uint8_t));
  landmark_write_map_subtree(head, buffer);
}

// writes a subtree to the given byte buffer
void landmark_write_map_subtree(landmark_map *node, uint8_t *buffer) {
  int i;
  assert(node != NULL);
  for (i = 0; i < buffer_get_size(); i++)
    buffer[i] = 255*landmark_seen_probability(node, i);
}

double landmark_seen_probability(landmark_map *node, int index) {
  landmark l;
  double p, sum;
  assert(node != NULL);
  l = node->map[index];

  sum = l.seen + l.unseen;
  // default to 5% probability if we have no data
  p = 0.05;
  if (sum > 0)
    p = l.seen/sum;

  // max p is 95% to avoid unseen probability of 0%
  if (p > 0.95)
    p = 0.95;

  return p;
}

double landmark_unseen_probability(landmark_map *node, int index) {
  return 1 - landmark_seen_probability(node, index);
}
/*
// returns distance in the direction specified by step in mm, according to the map
int landmark_map_find_distance(landmark_map *node, int step, particle p) {
  int d, done;
  double x, y, c, s, degrees, theta;

  // forward is now 0 degrees, left -, right +
  // TODO: _ETH
  degrees = -120 + step*SENSOR_SPACING_USB;
  theta = (degrees + p.theta)*M_PI/180;
  c = cos(theta);
  s = sin(theta);

  done = 0;
  for (d = 0; !done; d++) {
    x = d*c + p.x;
    y = d*s + p.y;

    // make sure it is in bounds and we can see it
    if (in_arena(x, y) &&
	landmark_seen_probability(node, buffer_index_from_x_y(x, y)) > 0.9)
      done = 1;
  }

  return d - 1;
}

void landmark_map_simulate_scan(particle p, int *distances, int m) {
  int i;
  for (i = 0; i < m; i++)
    distances[i] = landmark_map_find_distance(p.map, i, p);
}
*/
