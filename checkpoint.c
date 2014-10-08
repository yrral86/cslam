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

checkpoint* checkpoint_path_dup_with_deltas(checkpoint *cp, int *deltas) {
  int i;
  checkpoint *dup = checkpoint_path_new();
  checkpoint *temp = checkpoint_path_new();
  cp = cp->head;
  i = 0;
  while (cp->next != NULL) {
    temp->x = cp->x + deltas[3*i];
    temp->y = cp->y + deltas[3*i+1];
    temp->theta = cp->theta + deltas[3*i+2];
    temp->observation = cp->observation;
    dup = checkpoint_path_append(dup, temp);
    cp = cp->next;
    i++;
  }

  temp->x = cp->x + deltas[3*i];
  temp->y = cp->y + deltas[3*i+1];
  temp->theta = cp->theta + deltas[3*i+2];
  temp->observation = cp->observation;
  dup = checkpoint_path_append(dup, temp);

  checkpoint_path_deallocate(temp);

  return dup;
}

checkpoint* checkpoint_path_refine(checkpoint *cp) {
  int g, p, i, j, k, x, y, theta, swap;
  int length = checkpoint_path_length(cp);
  int chromo_size = length*3;
  // population shoud be divisible by 6
  int population = 30;
  int generations = 30;
  int chromosomes[population][chromo_size];
  int tmp_chromosome[chromo_size];
  double scores[population];
  double tmp_score, best_score = 0;
  checkpoint *refined_path;
  checkpoint *best_path = checkpoint_path_new();
  map_node *map;

  // init population
  for (p = 0; p < population; p++)
    for (i = 0; i < chromo_size; i++)
      if (p == 0)
	chromosomes[p][i] = 0;
      else
	if (i % 3 == 2)
	  // theta
	  chromosomes[p][i] = rand_limit(5);
	else
	  // x, y
	  chromosomes[p][i] = rand_limit(10);

  for (g = 0; g < generations; g++) {
    printf("generation %i\n", g);
    for (p = 0; i < population; p++) {
      refined_path = checkpoint_path_dup_with_deltas(cp, &(*(chromosomes[p])));
      map = checkpoint_path_write_map(refined_path);
      scores[p] = 1/(map_get_info(map)*map_get_size(map));
      if (scores[p] > best_score) {
	checkpoint_path_deallocate(best_path);
	best_path = refined_path;
	best_score = scores[p];
      } else
	checkpoint_path_deallocate(refined_path);
    }

        // bubble sort population by score
    i = 0;
    do {
      swap = 0;
      for (j = 0; j < population - i - 1; j++)
	// left is smaller, bubble to the right
	if (scores[j] < scores[j+1]) {
	  tmp_score = scores[j];
	  scores[j] = scores[j+1];
	  scores[j+1] = tmp_score;
	  memcpy(tmp_chromosome, chromosomes + j, chromo_size);
	  memcpy(chromosomes + j, chromosomes + j + 1, chromo_size);
	  memcpy(chromosomes + j + 1, tmp_chromosome, chromo_size);
	  swap = 1;
	}
      i++;
    } while (swap);

    // replace lower 1/3rd members of population with new members by crossing top 2/3, highest to lowest
    for (i = 0; i < population/3; i++) {
      checkpoint_path_refine_crossover((int*)chromosomes, i, (2*population/3 - 1)-i, 2*population/3+i, chromo_size);
    }

    // mutate at 1/10th of random points
    checkpoint_path_refine_mutate((int*)chromosomes, chromo_size);

    printf("best path modification:\n");
    for (i = 0; i < chromo_size; i++)
      printf("(%i, %i, %i)\n", chromosomes[0][3*i], chromosomes[0][3*i+1], chromosomes[0][3*i+2]);

    // next generation
  }

  return best_path;
}

void checkpoint_path_refine_crossover(int *chromosomes, int one, int two, int new, int size) {
  // two point crossover
  int c_1, c_2, tmp;
  c_1 = rand_limit(size);
  c_2 = rand_limit(size);
  if (c_1 > c_2) {
    tmp = c_1;
    c_1 = c_2;
    c_2 = tmp;
  }

  memcpy(chromosomes + new*size, chromosomes + one*size, c_1);
  memcpy(chromosomes + new*size + c_1, chromosomes + two*size + c_1, c_2 - c_1);
  memcpy(chromosomes + new*size + c_2, chromosomes + one*size + c_2, size - c_2);
}

void checkpoint_path_refine_mutate(int *chromosomes, int size) {
  int i;

  for (i = 0; i < size/10; i++)
    *(chromosomes + size*rand_limit(size) + rand_limit(3)) +=  rand_limit(10);
}
