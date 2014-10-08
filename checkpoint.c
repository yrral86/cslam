#include "checkpoint.h"

checkpoint* checkpoint_new(checkpoint *previous) {
  checkpoint *cp = malloc(sizeof(checkpoint));
  cp->next = NULL;
  previous->next = cp;
  cp->x = 0;
  cp->y = 0;
  cp->theta = 0;
  cp->information = 0;
  cp->size = 0;
  cp->observation = NULL;

  // previous is initial node, free it and make this the head
  if (previous->previous == NULL && previous->observation == NULL) {
    free(previous);
    cp->previous = NULL;
    cp->head = cp;
  } else {
    cp->previous = previous;
    cp->head = previous->head;
    previous->next = cp;
  }

  return cp;
}

checkpoint* checkpoint_path_new() {
  checkpoint *path = malloc(sizeof(checkpoint));
  path->next = NULL;
  path->previous = NULL;
  path->information = 0;
  path->size = 0;
  path->observation = NULL;
  path->head = path;

  return path;
}

checkpoint* checkpoint_path_append(checkpoint *previous, checkpoint *cp) {
  checkpoint *last = checkpoint_new(previous);
  last->x = cp->x;
  last->y = cp->y;
  last->theta = cp->theta;
  last->information = cp->information;
  last->size = cp->size;
  last->observation = cp->observation;

  return last;
}

checkpoint* checkpoint_path_end(checkpoint *cp) {
  while (cp->next != NULL)
    cp = cp->next;

  return cp;
}

int checkpoint_path_length(checkpoint *cp) {
  int i;
  cp = cp->head;
  for (i = 0; cp->next != NULL; i++)
    cp = cp->next;
  return i;
}

map_node* checkpoint_path_write_map(checkpoint *cp) {
  map_node *map = map_new(20000,20000);
  cp = cp->head;
  while (cp->next != NULL) {
    map_merge(map, cp->observation, cp->x, cp->y, cp->theta);
    cp = cp->next;
  }
  map_merge(map, cp->observation, cp->x, cp->y, cp->theta);
  return map;
}

void checkpoint_path_deallocate(checkpoint *cp) {
  cp = cp->head;
  while (cp->next != NULL) {
    cp = cp->next;
    free(cp->previous);
  }
  free(cp);
}
