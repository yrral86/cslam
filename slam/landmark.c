#include "landmark.h"
#include "buffer.h"

landmark_tree_node* landmark_tree_head(landmark_tree_node *parent) {
  landmark_tree_node *head;
  if (parent == NULL) {
    head = landmark_build_subtree(0, BUFFER_WIDTH*BUFFER_HEIGHT);
  } else {
    head = malloc(sizeof(landmark_tree_node));
    head->references = 1;
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
  landmark_tree_node_dereference(node->left);
  landmark_tree_node_dereference(node->right);
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
  else if (index <= node->index)
    landmark_set_seen(node->left, index);
  else
    landmark_set_seen(node->right, index);
}

void landmark_set_unseen(landmark_tree_node *node, int index) {
  if (node->left == NULL && node->right == NULL && node->index == index)
    node->landmark.unseen++;
  else if (index <= node->index)
    landmark_set_unseen(node->left, index);
  else
    landmark_set_unseen(node->right, index);
}
