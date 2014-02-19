#ifndef __LANDMARK_H__
#define __LANDMARK_H__

#include <assert.h>
#include <stdint.h>

typedef struct landmark {
  double x;
  double y;
  unsigned int seen;
  unsigned int unseen;
} landmark;

typedef struct landmark_tree_node {
  int references;
  int index;
  struct landmark_tree_node *left;
  struct landmark_tree_node *right;
  landmark landmark;
} landmark_tree_node;

landmark_tree_node* landmark_tree_copy(landmark_tree_node*);
void landmark_tree_node_copy_left(landmark_tree_node*, landmark_tree_node*);
void landmark_tree_node_copy_right(landmark_tree_node*, landmark_tree_node*);
void landmark_tree_node_delete(landmark_tree_node*);
void landmark_tree_node_dereference(landmark_tree_node*);
void landmark_tree_node_reference(landmark_tree_node*);
landmark_tree_node* landmark_build_subtree(int, int);
void landmark_set_seen(landmark_tree_node*, int);
void landmark_set_unseen(landmark_tree_node*, int);
void landmark_write_map(landmark_tree_node*, uint8_t*);
void landmark_write_map_subtree(landmark_tree_node*, uint8_t*);
double landmark_seen_probability(landmark_tree_node*, int);
landmark_tree_node* landmark_tree_find_leaf(landmark_tree_node*, int);

#endif
