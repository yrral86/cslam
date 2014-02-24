#include "landmark.h"
#include "buffer.h"

landmark_tree_node* landmark_tree_copy(landmark_tree_node *parent) {
  landmark_tree_node *head;
  if (parent == NULL) {
    head = landmark_build_subtree(0, BUFFER_SIZE - 1);
  } else {
    head = malloc(sizeof(landmark_tree_node));
    head->references = 1;
    head->index = parent->index;
    if (parent->left != NULL && parent->right != NULL) {
      landmark_tree_node_copy_left(parent, head);
      landmark_tree_node_copy_right(parent, head);
    } else {
      assert(parent->left == NULL);
      assert(parent->right == NULL);
      head->landmark = parent->landmark;
      head->left = NULL;
      head->right = NULL;
    }
  }
  return head;
}

void landmark_tree_node_copy_left(landmark_tree_node *from, landmark_tree_node *to) {
  assert(from != NULL);
  assert(to != NULL);
  assert(from->left != NULL);
  to->left = from->left;
  landmark_tree_node_reference(to->left);
}

void landmark_tree_node_copy_right(landmark_tree_node *from, landmark_tree_node *to) {
  assert(from != NULL);
  assert(to != NULL);
  assert(from->right != NULL);
  to->right = from->right;
  landmark_tree_node_reference(to->right);
}

void landmark_tree_node_delete(landmark_tree_node *node) {
  assert(node != NULL);
  assert(node->references == 0);
  if (node->left != NULL && node->right != NULL) {
    landmark_tree_node_dereference(node->left);
    landmark_tree_node_dereference(node->right);
  } else {
    assert(node->left == NULL);
    assert(node->right == NULL);
    // TODO: if we switch to landmark pointer
    // to save memory on interior nodes
    // free it here
  }
  free(node);
}

void landmark_tree_node_dereference(landmark_tree_node *node) {
  assert(node != NULL);
  assert(node->references > 0);
  node->references--;
  if (node->references == 0)
    landmark_tree_node_delete(node);
}

void landmark_tree_node_reference(landmark_tree_node *node) {
  assert(node != NULL);
  node->references++;
}

landmark_tree_node* landmark_build_subtree(int min, int max) {
  assert(min <= max);
  landmark_tree_node *node = malloc(sizeof(landmark_tree_node));
  node->references = 1;
  if (min < max) {
    node->index = (max + min)/2;
    node->left = landmark_build_subtree(min, node->index);
    node->right = landmark_build_subtree(node->index + 1, max);
  } else {
    node->index = min;
    node->left = NULL;
    node->right = NULL;
    node->landmark.x = x_from_buffer_index(node->index);
    node->landmark.y = y_from_buffer_index(node->index);
    node->landmark.seen = 0;
    node->landmark.unseen = 0;
  }
  return node;
}

void landmark_set_seen(landmark_tree_node *node, int index) {
  assert(node != NULL);
  landmark_tree_node* leaf = landmark_tree_find_leaf(node, index);
  landmark_set_seen_value(node, index, leaf->landmark.seen + 1);
}

void landmark_set_seen_value(landmark_tree_node *node, int index, int value) {
  assert(node != NULL);
  if (node->left == NULL && node->right == NULL && node->index == index)
    node->landmark.seen = value;
  else if (index <= node->index) {
    assert(node->left != NULL);
    // copy on write
    if (node->left->references > 1) {
      landmark_tree_node *old_left = node->left;
      node->left = landmark_tree_copy(old_left);
      landmark_tree_node_dereference(old_left);
    }
    landmark_set_seen_value(node->left, index, value);
  } else {
    assert(node->right != NULL);
    // copy on write
    if (node->right->references > 1) {
      landmark_tree_node *old_right = node->right;
      node->right = landmark_tree_copy(old_right);
      landmark_tree_node_dereference(old_right);
    }
    landmark_set_seen_value(node->right, index, value);
  }
}

void landmark_set_unseen(landmark_tree_node *node, int index) {
  assert(node != NULL);
  landmark_tree_node* leaf = landmark_tree_find_leaf(node, index);
  landmark_set_unseen_value(node, index, leaf->landmark.unseen + 1);
}

void landmark_set_unseen_value(landmark_tree_node *node, int index, int value) {
  assert(node != NULL);
  if (node->left == NULL && node->right == NULL && node->index == index)
    node->landmark.unseen = value;
  else if (index <= node->index) {
    // copy on write
    if (node->left->references > 1) {
      assert(node->left != NULL);
      landmark_tree_node *old_left = node->left;
      node->left = landmark_tree_copy(old_left);
      landmark_tree_node_dereference(old_left);
    }
    landmark_set_unseen_value(node->left, index, value);
  } else {
    assert(node->right != NULL);
    // copy on write
    if (node->right->references > 1) {
      landmark_tree_node *old_right = node->right;
      node->right = landmark_tree_copy(old_right);
      landmark_tree_node_dereference(old_right);
    }
    landmark_set_unseen_value(node->right, index, value);
  }
}

// writes a byte buffer given the head of a landmark tree
void landmark_write_map(landmark_tree_node *head, uint8_t *buffer) {
  bzero(buffer, BUFFER_SIZE*sizeof(uint8_t));
  landmark_write_map_subtree(head, buffer);
}

// writes a subtree to the given byte buffer
void landmark_write_map_subtree(landmark_tree_node *node, uint8_t *buffer) {
  assert(node != NULL);
  if (node->left == NULL && node->right == NULL) {
    assert(node->index >= 0);
    assert(node->index < BUFFER_SIZE);
    buffer[node->index] = 255*landmark_seen_probability(node, node->index);
  } else {
    assert(node->left != NULL);
    assert(node->right != NULL);
    landmark_write_map_subtree(node->left, buffer);
    landmark_write_map_subtree(node->right, buffer);
  }
}

double landmark_seen_probability(landmark_tree_node *node, int index) {
  assert(node != NULL);
  landmark_tree_node* leaf = landmark_tree_find_leaf(node, index);
  // default to 50% probability if we have no data
  double p = 0.5;
  double sum = leaf->landmark.seen + leaf->landmark.unseen;
  if (sum > 0)
    p = leaf->landmark.seen/sum;
  return p;
}

double landmark_unseen_probability(landmark_tree_node *node, int index) {
  return 1 - landmark_seen_probability(node, index);
}

landmark_tree_node* landmark_tree_find_leaf(landmark_tree_node *node, int index) {
  assert(node != NULL);
  if (node->left == NULL && node->right == NULL && node->index == index)
    return node;
  else if (index <= node->index)
    return landmark_tree_find_leaf(node->left, index);
  else
    return landmark_tree_find_leaf(node->right, index);
}
