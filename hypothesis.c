#include "hypothesis.h"

void hypothesis_reference(hypothesis *h) {
  h->references++;
}

void hypothesis_dereference(hypothesis *h) {
  h->references--;
  if (h->references == 0)
    free(h);
}

hypothesis* hypothesis_new(hypothesis *parent, double x, double y, double theta) {
  hypothesis *h = malloc(sizeof(hypothesis));
  h->references = 1;
  h->child_count = 0;
  h->parent = parent;
  // add to parent's children if parent is not null
  if (parent != NULL) {
    parent->children[parent->child_count++] = h;
    // make sure we don't overrun children
    assert(parent->child_count < PARTICLE_COUNT);
    hypothesis_reference(parent);
    hypothesis_reference(h);
  }
  h->x = x;
  h->y = y;
  h->theta = theta;
  return h;
}
