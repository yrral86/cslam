#include "slam.h"

#include "map.h"
#include "lazygl.h"

// 3, one for current, one for history, one for display
#define BUFFER_HISTORY 3
//static uint8_t* map[BUFFER_HISTORY];
static uint8_t* buffer_latest;
static uint8_t* buffer_all;

static raw_sensor_scan *scans;
static particle current_particle;
//static int width = 10000;
//static int height = 10000;
static int width = 10000;
static int height = 10000;
// sample 10 times (1 sec)
static int sample_count = 1;
static int best_x, best_y, best_t;

int main (int argc, char **argv) {
  int j, iterations;
  pthread_t sensor_thread;
  double info;
  int x, y, t, size;
  map_node *map_latest, *map_all;

  // connect to sensor in new thread while we
  // initialize everything else
  sensor_thread = sensor_init_thread();

  scans = malloc(sample_count*sizeof(raw_sensor_scan));

  // swarm_init will set up buffer sizing
  //  swarm_init(RAW_SENSOR_DISTANCES_USB, SENSOR_RANGE_USB, 7380, 3880, 1500);
  //  swarm_init(RAW_SENSOR_DISTANCES_USB, SENSOR_RANGE_USB, arena_width, arena_height, 2000, 0);

  // allocate buffers
  //  for (i = 0; i < BUFFER_HISTORY; i++)
  // map[i] = buffer_allocate();

  buffer_latest = malloc((width + 1)*(height + 1));
  buffer_all = malloc((width + 1)*(height + 1));

  glutInit(&argc, argv);
  // pass size of buffer, then window size
  initGL(buffer_latest, buffer_all, width + 1, height + 1, 700, 700);

  map_latest = map_new(width, height);
  map_all = map_new(width, height);

  // wait for sensor
  assert(pthread_join(sensor_thread, NULL) == 0);

  // start a scan
  sensor_thread = sensor_read_raw_n_thread(sample_count);

  x = y = t = 0;

  iterations = 0;

  while(1) {
      /*
    do {
      swarm_move(0, 0, 360);
      */

      // wait for sensor
      assert(pthread_join(sensor_thread, NULL) == 0);

      // get samples
      for (j = 0; j < sample_count; j++) {
	scans[j] = sensor_fetch_index(j);
      }

      // start a scan
      sensor_thread = sensor_read_raw_n_thread(sample_count);

      //      map_latest = build_submap(scans);

      map_latest = map_new_from_observation(scans[0].distances);

      printf("merging using (%i, %i, %i)\n", x, y, t);
      map_merge(map_all, map_latest, x, y, t);

      x += best_x;
      y += best_y;
      t += best_t;

      map_write_buffer(map_latest, buffer_latest);
      map_write_buffer(map_all, buffer_all);

      size = map_get_size(map_all);
      info = map_get_info(map_all);
      printf("map all size: %i, info: %g, info/size: %g\n", size, info, info/size);
      size = map_get_size(map_latest);
      info = map_get_info(map_latest);
      printf("map latest size: %i, info: %g, info/size: %g\n", size, info, info/size);

      map_deallocate(map_latest);

    /*
      if (iterations == 0) {
	// map initial location 10 times
	for (i = 0; i < 10; i++) {
	  // record each scan with the certainty of 50 observations
	  for (j = 0; j < 50*sample_count; j++)
	    swarm_map(scans[j % sample_count].distances);

	  // wait for sensor
	  assert(pthread_join(sensor_thread, NULL) == 0);

	  // get samples
	  for (j = 0; j < sample_count; j++) {
	    scans[j] = sensor_fetch_index(j);
	  }

	  // start a scan
	  sensor_thread = sensor_read_raw_n_thread(sample_count);
	}
	printf("initial scan completed\n");
      }

      // give the swarm the new scans and the historical map
      swarm_update(scans[0].distances);
    } while (swarm_converged() == 0);

    printf("converged, mapping\n");
    // update map
    for (j = 0; j < sample_count; j++)
      swarm_map(scans[j].distances);
    // update current map
    swarm_map_reset_current();
    for (j = 0; j < sample_count; j++)
      swarm_map_current(scans[j].distances);

    // update localization
    current_particle.x = swarm_get_best_x();
    current_particle.y = swarm_get_best_y();
    current_particle.theta = swarm_get_best_theta();

    // copy best map to buffer
    swarm_get_best_buffer(map[0]);
    swarm_get_map_buffer(map[2]);

    // draw position
    double s, c, t;
    t = current_particle.theta*M_PI/180;
    s = sin(t);
    c = cos(t);
    for (i = -5; i < 6; i++)
      for (j = -5; j < 6; j++ ) {
	record_map_position(0, current_particle.x + i*c - j*s,
			    current_particle.y + i*s + j*c, 255);
	record_map_position(2, current_particle.x + i*c - j*s,
			    current_particle.y + i*s + j*c, 255);
      }
*/
    // draw
    /*
    // clear position
    for (i = -5; i < 6; i++)
      for (j = -5; j < 6; j++ ) {
	record_map_position(0, current_particle.x + i*c - j*s,
			    current_particle.y + i*s + j*c, 0);
	if (!x_y_protected(current_particle.x + i*c - j*s, current_particle.y + i*s + j*c))
	  record_map_position(2, current_particle.x + i*c - j*s,
			      current_particle.y + i*s + j*c, 0);
      }
*/

    //    if (iterations % 100 == 0) {
      //    }

    display();
    glutMainLoopEvent();

    iterations++;
  }

  return 0;
}

map_node* build_submap(raw_sensor_scan* scans) {
  double c, s, theta, best_score, tmp_score, info;
  int i, j, k, d, o_x, o_y, x, y, t, g, p, swap, size;
  map_node *best_map, *map;
  // x, y, theta for each position except the first
  int chromo_size = (sample_count-1)*3;
  // poulation should be divisble by 6
  int population = 6;
  int chromosomes[population][chromo_size];
  int tmp_chromosome[chromo_size];
  double scores[population];

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

  map = map_new(width, height);

  o_x = width/2;
  o_y = height/2;

  // generations
  for (g = 0; g < 1; g++) {
    printf ("generation %i\n", g);
    best_map = map_new(width, height);
    best_score = 0;
    // population
    for (p = 0; p < population; p++) {

  // samples
      for (k = 0; k < sample_count; k++) {
	if (k == 0) {
	  x = 0;
	  y = 0;
	  t = 0;
	} else {
	  x = chromosomes[p][3*(k-1)];
	  y = chromosomes[p][3*(k-1)+1];
	  t = chromosomes[p][3*(k-1)+2];
	}

	// map_new_from_observation

	// record observations
	for (i = 0; i < RAW_SENSOR_DISTANCES_USB; i++) {
	  theta = (t + -SENSOR_RANGE_USB/2.0 + i*SENSOR_SPACING_USB)*M_PI/180;
	  c = cos(theta);
	  s = sin(theta);
	  d = scans[k].distances[i];
	  map_set_seen(map, o_x + x + d*c, height - (o_y + y) + d*s);
	  for (j = d - 30; j >= 30; j -= 30)
	    map_set_unseen(map, o_x + x + (d-j)*c, height - (o_y + y) + (d-j)*s);
	}
      }
      scores[p] = map_get_info(map)/map_get_size(map);
      if (scores[p] > best_score) {
	map_deallocate(best_map);
	best_map = map;
	best_score = scores[p];
	best_x = x;
	best_y = y;
	best_t = t;
      } else
	map_deallocate(map);

      map = map_new(width, height);
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
      crossover((int*)chromosomes, i, (2*population/3 - 1)-i, 2*population/3+i, chromo_size);
    }

    // mutate at 1/10th of random points
    mutate((int*)chromosomes, chromo_size);

    size = map_get_size(best_map);
    info = map_get_info(best_map);
    printf("best map  size: %i, info: %g, info/size: %g\n", size, info, info/size);
    printf("best path:\n");
    printf("(0, 0, 0)\n");
    for (i = 0; i < chromo_size; i++)
      printf("(%i, %i, %i)\n", chromosomes[0][3*i], chromosomes[0][3*i+1], chromosomes[0][3*i+2]);

    // next generation
  }

  map_deallocate(map);

  return best_map;
      
  //      return map;
}

void crossover(int *chromosomes, int one, int two, int new, int size) {
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

void mutate(int *chromosomes, int size) {
  int i;

  for (i = 0; i < size/10; i++)
    *(chromosomes + size*rand_limit(size) + rand_limit(3)) +=  rand_limit(10);
}
