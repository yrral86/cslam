#include "landmark.h"
#include "buffer.h"

landmark_tree_node* landmark_tree_copy(landmark_tree_node *parent) {
  landmark_tree_node *head;
  if (parent == NULL) {
    head = landmark_build_subtree(0, BUFFER_SIZE);
  } else {
    head = malloc(sizeof(landmark_tree_node));
    head->references = 1;
    head->index = parent->index;
    landmark_tree_node_copy_left(parent, head);
    landmark_tree_node_copy_right(parent, head);
  }
  return head;
}

void landmark_tree_node_copy_left(landmark_tree_node *from, landmark_tree_node *to) {
  to->left = from->left;
  landmark_tree_node_reference(to->left);
}

void landmark_tree_node_copy_right(landmark_tree_node *from, landmark_tree_node *to) {
  to->right = from->right;
  landmark_tree_node_reference(to->right);
}

void landmark_tree_node_delete(landmark_tree_node *node) {
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
  assert(node->references > 0);
  node->references--;
  if (node->references == 0)
    landmark_tree_node_delete(node);
}

void landmark_tree_node_reference(landmark_tree_node *node) {
  node->references++;
}

landmark_tree_node* landmark_build_subtree(int min, int max) {
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
  if (node->left == NULL && node->right == NULL && node->index == index)
    node->landmark.seen++;
  else if (index <= node->index) {
    if (node->left->references > 1) {
      landmark_tree_node *old_left = node->left;
      node->left = landmark_tree_copy(old_left);
      landmark_tree_node_dereference(old_left);
    }
    landmark_set_seen(node->left, index);
  } else {
    if (node->right->references > 1) {
      landmark_tree_node *old_right = node->right;
      node->right = landmark_tree_copy(old_right);
      landmark_tree_node_dereference(old_right);
    }
    landmark_set_seen(node->right, index);
  }
}

void landmark_set_unseen(landmark_tree_node *node, int index) {
  if (node->left == NULL && node->right == NULL && node->index == index)
    node->landmark.unseen++;
  else if (index <= node->index) {
    if (node->left->references > 1) {
      landmark_tree_node *old_left = node->left;
      node->left = landmark_tree_copy(old_left);
      landmark_tree_node_dereference(old_left);
    }
    landmark_set_unseen(node->left, index);
  } else {
    if (node->right->references > 1) {
      landmark_tree_node *old_right = node->right;
      node->right = landmark_tree_copy(old_right);
      landmark_tree_node_dereference(old_right);
    }
    landmark_set_unseen(node->right, index);
  }
}

// writes a byte buffer given the head of a landmark tree
void landmark_write_map(landmark_tree_node *head, uint8_t *buffer) {
  bzero(buffer, BUFFER_SIZE);
  landmark_write_map_subtree(head, buffer);
}

// writes a subtree to the given byte buffer
void landmark_write_map_subtree(landmark_tree_node *node, uint8_t *buffer) {
  if (node->left == NULL && node->right == NULL)
    buffer[node->index] = 255*landmark_seen_probability(node, node->index);
  else {
    landmark_write_map_subtree(node->left, buffer);
    landmark_write_map_subtree(node->right, buffer);
  }
}

double landmark_seen_probability(landmark_tree_node *node, int index) {
  landmark_tree_node* leaf = landmark_tree_find_leaf(node, index);
  double p = 0.001;
  double sum = leaf->landmark.seen + leaf->landmark.unseen;
  if (sum > 0)
    p = leaf->landmark.seen/sum;
  return p;
}

double landmark_unseen_probability(landmark_tree_node *node, int index) {
  return 1 - landmark_seen_probability(node, index);
}

landmark_tree_node* landmark_tree_find_leaf(landmark_tree_node *node, int index) {
  if (node->left == NULL && node->right == NULL && node->index == index)
    return node;
  else if (index <= node->index)
    return landmark_tree_find_leaf(node->left, index);
  else
    return landmark_tree_find_leaf(node->right, index);
}
