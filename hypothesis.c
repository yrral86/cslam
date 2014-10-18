#include "hypothesis.h"

void hypothesis_reference(hypothesis *h) {
  h->references++;
  if (h->map != NULL)
    map_reference(h->map);
}

void hypothesis_dereference(hypothesis *h) {
  h->references--;

  if (h->references == 0) {
    if (h->parent != NULL)
      hypothesis_remove_child(h->parent, h);
    if (h->map != NULL)
      map_dereference(h->map);
    free(h);
  }
}

void hypothesis_remove_child(hypothesis *p, hypothesis *c) {
  int i, j;
  //  printf("removing hypothesis %p from parent %p\n", c, p);

  for (i = 0; i < p->child_count; i++)
    if (p->children[i] == c) {
      // if we find it, reduce child count
      p->child_count--;
      // then shift remaining children forward
      for (j = i; j < p->child_count; j++)
	p->children[i] = p->children[i+1];
      hypothesis_dereference(p);
    }
}

hypothesis* hypothesis_new(hypothesis *parent, double x, double y, double theta) {
  hypothesis *h = malloc(sizeof(hypothesis));
  h->references = 1;
  h->child_count = 0;
  h->parent = parent;
  h->map = NULL;
  // add to parent's children if parent is not null
  if (parent != NULL) {
    parent->children[parent->child_count++] = h;
    // make sure we don't overrun children
    //    printf("children: %d\n", parent->child_count);
    assert(parent->child_count < 5*PARTICLE_COUNT);
    // child references parent, parent does not reference child
    hypothesis_reference(parent);
  }
  h->x = x;
  h->y = y;
  h->theta = theta;
  return h;
}
