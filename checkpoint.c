#include "checkpoint.h"

checkpoint* checkpoint_new(checkpoint *previous) {
  checkpoint *cp = malloc(sizeof(checkpoint));
  cp->next = NULL;
  previous->next = cp;

  // previous is initial node, free it and make this the head
  if (previous->previous == NULL) {
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
  path->head = path;

  return path;
}

checkpoint* checkpoint_path_append(checkpoint *previous, hypothesis *h) {
  checkpoint *last = checkpoint_new(previous);
  hypothesis_reference(h);
  last->h = h;

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
  i++;
  return i;
}

map_node* checkpoint_path_write_map(checkpoint *cp) {
  map_node *map = map_new(MAP_SIZE, MAP_SIZE);
  map_node *cp_map, *mask_map;
  cp = cp->head;
  printf("map size: %d\n", map->current_size);
  while (cp->next != NULL) {
    /*    printf("merging hypothesis (%d,%d,%d) with observations (%d,%d,%d...)\n", cp->h->x,
	  cp->h->y, cp->h->theta, cp->h->obs->list[0].r, cp->h->obs->list[1].r, cp->h->obs->list[2].r);*/
    mask_map = map_get_shifted_mask(cp->h->x, cp->h->y);
    cp_map = map_from_mask_and_hypothesis(mask_map, cp->h);
    map_deallocate(mask_map);
    map = map_merge(map, cp_map);
    // merge frees original map and cp_map
    //    printf("after merge, size: %i, variance: %g\n", map->current_size, map_variance(map));
    cp = cp->next;
  }
  mask_map = map_get_shifted_mask(cp->h->x, cp->h->y);
  cp_map = map_from_mask_and_hypothesis(mask_map, cp->h);
  map_deallocate(mask_map);
  map = map_merge(map, cp_map);
  return map;
}
/*
map_node* checkpoint_path_write_map_with_path(checkpoint *cp) {
  int i, j;
  map_pixel p;
  map_node *map = map_new(MAP_SIZE, MAP_SIZE);
  cp = cp->head;
  while (cp->next != NULL) {
    map = map_merge(map, cp->h);
    for (i = -5; i < 6; i++)
      for (j = -5; j < 6; j++) {
	p.x = cp->h->x+i;
	p.y = cp->h->y+i;
	p.l.seen = 1;
	p.l.seen = 0;
	map_add_pixel(map, p);
      }
    cp = cp->next;
  }
  map = map_merge(map, cp->h);
  for (i = -5; i < 6; i++)
    for (j = -5; j < 6; j++) {
      p.x = cp->h->x+i;
      p.y = cp->h->y+i;
      p.l.seen = 1;
      p.l.seen = 0;
      map_add_pixel(map, p);
    }

  return map;
}
*/
void checkpoint_path_deallocate(checkpoint *cp) {
  cp = cp->head;
  while (cp->next != NULL) {
    cp = cp->next;
    hypothesis_dereference(cp->previous->h);
    free(cp->previous);
  }
  free(cp);
}

checkpoint* checkpoint_path_dup_with_deltas(checkpoint *cp, int *deltas) {
  int i;
  checkpoint *dup = checkpoint_path_new();
  hypothesis *temp;
  cp = cp->head;
  i = 0;
  while (cp->next != NULL) {
    temp = hypothesis_new(cp->h,
			  cp->h->x + deltas[3*i],
			  cp->h->y + deltas[3*i+1],
			  cp->h->theta + deltas[3*i+2]);
    temp->obs = cp->h->obs;
    dup = checkpoint_path_append(dup, temp);
    hypothesis_dereference(temp);
    cp = cp->next;
    i++;
  }

  temp = hypothesis_new(cp->h,
			cp->h->x + deltas[3*i],
			cp->h->y + deltas[3*i+1],
			cp->h->theta + deltas[3*i+2]);
  temp->obs = cp->h->obs;
  dup = checkpoint_path_append(dup, temp);
  hypothesis_dereference(temp);

  return dup;
}

checkpoint* checkpoint_path_refine(checkpoint *cp) {
  int g, p, i, j, swap;//k, x, y, theta;
  int length = checkpoint_path_length(cp);
  int chromo_size = length*3;
  // population shoud be divisible by 6
  int population = 1000;
  int generations = 100;
  int chromosomes[population][chromo_size];
  int tmp_chromosome[chromo_size];
  double scores[population];
  double tmp_score, best_score;
  checkpoint *refined_path;
  checkpoint *best_path = checkpoint_path_new();
  map_node *map;

  // init population
  for (p = 0; p < population; p++)
    for (i = 0; i < chromo_size; i++)
      if (p < 5)
	chromosomes[p][i] = 0;
      else
	if (i % 3 == 2)
	  // theta [-2,2]
	  chromosomes[p][i] = rand_limit(5)-2;
	else
	  // x, y [-5,5]
	  chromosomes[p][i] = rand_limit(11)-5;

  for (g = 0; g < generations; g++) {
    best_score = 0;
    printf("generation %i\n", g);
    for (p = 0; p < population; p++) {
      refined_path = checkpoint_path_dup_with_deltas(cp, &(*(chromosomes[p])));
      map = checkpoint_path_write_map(refined_path);

      scores[p] = 1.0/map_variance(map);
      // maximize info/size
      //      scores[p] = map_get_info(map)/map_get_size(map);
      map_deallocate(map);
      if (scores[p] > best_score) {
	printf("best score %g\n", scores[p]);
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

    // replace lower 1/3rd members of population two randomly selected members of top 1/3 and top 2/3
    for (i = 0; i < population/3; i++) {
      checkpoint_path_refine_crossover((int*)chromosomes, rand_limit(population/3), rand_limit(2*population/3), 2*population/3+i, chromo_size);
    }

    // mutate at a population/3 random points in back 1/3rd
    for (i = 0; i < population/6; i++)
      checkpoint_path_refine_mutate((int*)chromosomes[2*population/3 + rand_limit(population/3)]);

    printf("end of generation best path modification (length %d):\n", length);
    for (i = 0; i < length; i++)
      printf("path modification %i: (%i, %i, %i)\n", i + 1, chromosomes[0][3*i], chromosomes[0][3*i+1], chromosomes[0][3*i+2]);

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

void checkpoint_path_refine_mutate(int *chromosome) {
  int i;

  for (i = 0; i < 3; i++) {
    if (i == 2)
      *(chromosome + i) +=  rand_normal(INITIAL_ANGLE_VARIANCE);
    else
      *(chromosome + i) +=  rand_normal(INITIAL_POSITION_VARIANCE);
  }
}

void checkpoint_path_debug(checkpoint *cp) {
  int i;

  cp = cp->head;
  printf("checkpoint_debug:\n");
  i = 1;
  while (cp->next != NULL) {
    printf("%d: (%g,%g,%g)\n", i, cp->h->x, cp->h->y, cp->h->theta);
    cp = cp->next;
    i++;
  }
  printf("%d: (%g,%g,%g)\n", i, cp->h->x, cp->h->y, cp->h->theta);
}
