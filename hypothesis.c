#include "hypothesis.h"

void hypothesis_reference(hypothesis *h) {
  h->references++;
}

void hypothesis_dereference(hypothesis *h) {
  hypothesis *p;

  h->references--;
  // if the remaining reference is the parent
  if (h->references == 1 && h->parent != NULL) {
    p = h->parent;
    h->parent = NULL;
    // remove the child (which dereferences child for the final time)
    hypothesis_remove_child(p, h);
    // dereference parent since we no longer track it
    hypothesis_dereference(p);
  }

  if (h->references == 0) {
    if (h->map != NULL)
      map_deallocate(h->map);
    if (h->buffer != NULL)
      free(h->buffer);
    free(h);
  }
}

void hypothesis_remove_child(hypothesis *p, hypothesis *c) {
  int i, j;
  printf("removing hypothesis %p from parent %p\n", c, p);

  for (i = 0; i < p->child_count; i++)
    if (p->children[i] == c) {
      // if we find it, reduce child count
      p->child_count--;
      // then shift remaining children forward
      for (j = i; j < p->child_count; j++)
	p->children[i] = p->children[i+1];
      // dreference child
      hypothesis_dereference(c);
    }
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
    printf("children: %d\n", parent->child_count);
    assert(parent->child_count < 5*PARTICLE_COUNT);
    hypothesis_reference(parent);
    hypothesis_reference(h);
  }
  h->x = x;
  h->y = y;
  h->theta = theta;
  h->buffer = NULL;
  h->map = NULL;
  return h;
}
