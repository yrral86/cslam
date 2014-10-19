#include "hypothesis.h"

void hypothesis_reference(hypothesis *h) {
  //  printf("hypothesis reference %p\n", h);
  h->references++;
  if (h->map != NULL)
    map_reference(h->map);
}

void hypothesis_dereference(hypothesis *h) {
  //  printf("hypothesis dereference %p\n", h);
  h->references--;

  if (h->map != NULL)
    map_dereference(h->map);

  if (h->references == 0) {
    if (h->parent != NULL)
      hypothesis_remove_child(h->parent, h);
    free(h);
  }
}

void hypothesis_remove_child(hypothesis *p, hypothesis *c) {
  int i, j;
  //  printf("removing hypothesis %p from parent %p\n", c, p);

  for (i = 0; i < p->child_count; i++)
    if (p->children[i] == c) {
      //        printf("found, removing %p\n", c);
      c->parent = NULL;
      // if we find it, reduce child count
      p->child_count--;
      // then shift remaining children forward
      for (j = i; j < p->child_count; j++) {
	//	printf("replacing %p with %p\n", p->children[j], p->children[j+1]);
	p->children[j] = p->children[j+1];
      }
      hypothesis_dereference(p);
      return;
    }
}

void hypothesis_add_child(hypothesis *p, hypothesis *c) {
  p->children[p->child_count++] = c;
  // make sure we don't overrun children
  //    printf("children: %d\n", parent->child_count);
  assert(p->child_count < 5*PARTICLE_COUNT);
  // child references parent, parent does not reference child
  hypothesis_reference(p);
}

hypothesis* hypothesis_new(hypothesis *parent, double x, double y, double theta) {
  hypothesis *h = malloc(sizeof(hypothesis));
  h->references = 1;
  h->child_count = 0;
  h->parent = parent;
  h->map = NULL;
  // add to parent's children if parent is not null
  if (parent != NULL) {
    hypothesis_add_child(parent, h);
    if (parent->map != NULL) {
      // copy map pointer from parent
      h->map = parent->map;
      map_reference(h->map);
    }
  }
  h->x = x;
  h->y = y;
  h->theta = theta;
  return h;
}

int hypothesis_tree_size(hypothesis *root) {
  // count ourselves
  int i, size = 1;
  assert(root != NULL);
  /*  printf("root = %p\n", root);
  printf("child_count = %d\n", root->child_count);
  printf("references = %d\n", root->references);*/
  // add children and their ancestors
  for (i = 0; i < root->child_count; i++)
    size += hypothesis_tree_size(root->children[i]);
  return size;
}
