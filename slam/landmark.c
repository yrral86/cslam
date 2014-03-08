#include "landmark.h"
#include "buffer.h"

landmark_tree_node* landmark_tree_copy(landmark_tree_node *parent) {
  landmark_tree_node *head;
  if (parent == NULL) {
    head = landmark_build_subtree(0, BUFFER_SIZE - 1);
  } else {
    head = malloc(sizeof(landmark_tree_node));
    memcpy(head->map, parent->map, sizeof(landmark)*BUFFER_SIZE);
  }
  return head;
}

void landmark_tree_node_dereference(landmark_tree_node *node) {
  assert(node != NULL);
  free(node);
}

landmark_tree_node* landmark_build_subtree(int min, int max) {
  assert(min <= max);
  int i;
  landmark_tree_node *node = malloc(sizeof(landmark_tree_node));
  for (i = min; i <= max; i++) {
    node->map[i].x = x_from_buffer_index(i);
    node->map[i].y = y_from_buffer_index(i);
    node->map[i].seen = 0;
    node->map[i].unseen = 0;
  }
  return node;
}

void landmark_set_seen(landmark_tree_node *node, int index) {
  assert(node != NULL);
  node->map[index].seen++;
}

void landmark_set_seen_value(landmark_tree_node *node, int index, int value) {
  assert(node != NULL);
  assert(node->map != NULL);
  node->map[index].seen = value;
}

void landmark_set_unseen(landmark_tree_node *node, int index) {
  assert(node != NULL);
  assert(node->map != NULL);
  node->map[index].unseen++;
}

void landmark_set_unseen_value(landmark_tree_node *node, int index, int value) {
  assert(node != NULL);
  node->map[index].unseen = value;
}

// writes a byte buffer given the head of a landmark tree
void landmark_write_map(landmark_tree_node *head, uint8_t *buffer) {
  bzero(buffer, BUFFER_SIZE*sizeof(uint8_t));
  landmark_write_map_subtree(head, buffer);
}

// writes a subtree to the given byte buffer
void landmark_write_map_subtree(landmark_tree_node *node, uint8_t *buffer) {
  assert(node != NULL);
  int i;
  for (i = 0; i < BUFFER_SIZE; i++)
    buffer[i] = 255*landmark_seen_probability(node, i);
}

double landmark_seen_probability(landmark_tree_node *node, int index) {
  assert(node != NULL);
  landmark l = node->map[index];

  double sum = l.seen + l.unseen;
  // default to 5% probability if we have no data
  double p = 0.05;
  if (sum > 0)
    p = l.seen/sum;
  return p;
}

double landmark_unseen_probability(landmark_tree_node *node, int index) {
  return 1 - landmark_seen_probability(node, index);
}

// returns distance in the direction specified by step in mm, according to the map
int landmark_tree_node_find_distance(landmark_tree_node *node, int step, particle p) {
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

raw_sensor_scan landmark_tree_simulate_scan(particle p) {
  raw_sensor_scan scan;
  int i;

  // TODO: _ETH
  for (i = 0; i < RAW_SENSOR_DISTANCES_USB; i++)
    scan.distances[i] = landmark_tree_node_find_distance(p.map, i, p);

  return scan;
}
