#include "slam.h"
#include "slamd.h"
#include "replay.h"

#include "lazygl.h"

// 3, one for current, one for history, one for display
#define BUFFER_HISTORY 3
static uint8_t* map[BUFFER_HISTORY];
static int current_command;
static particle current_particle;
FILE *output_file;

int main (int argc, char **argv) {
  // sensor_scans + function indicator + return
  int i, iterations, scan_count, parsed_line[723];
  FILE *data_file;
  ssize_t read;
  char *int_string;
  char *line = NULL;
  size_t length  = 0;

  if (argc != 2) {
    printf("must specify output filename\n");
    exit(1);
  }

  data_file = fopen("slamd_record.csv", "r");
  assert(data_file != NULL);

  output_file = fopen(argv[1], "w");
  assert(output_file != NULL);

  iterations = 0;
  scan_count = 0;

  while((read = getline(&line, &length, data_file)) != -1) {
    int_string = strtok(line, ",");
    if (strncmp(int_string, "init", 4) == 0)
      continue;
    sscanf(int_string, "%d", parsed_line);
    //for (i = 1; i < 722; i++) {
    for (i = 1; i < 682; i++) {
      int_string = strtok(NULL, ",");
      sscanf(int_string, "%d", parsed_line + i);
    }
    int_string = strtok(NULL, ",");
    sscanf(int_string, "%d\n", parsed_line + i);

    current_command = parsed_line[0];

    switch(current_command) {
    case SLAMD_INIT:
      swarm_init(parsed_line[1], parsed_line[2], parsed_line[3], parsed_line[4], parsed_line[5], parsed_line[6]);

      // allocate buffers
      for (i = 0; i < BUFFER_HISTORY; i++)
	map[i] = buffer_allocate();

      // noui      glutInit(&argc, argv);
      // pass size of buffer, then window size
      // noui            initGL(map[0], map[2], buffer_get_width(), buffer_get_height(), 0.75*buffer_get_width(), 0.75*buffer_get_height());
      //      initGL(map[0], map[2], buffer_get_width(), buffer_get_height(), 2*buffer_get_width(), 2*buffer_get_height());
      break;
    case SLAMD_MOVE:
      swarm_move(parsed_line[1], parsed_line[2], parsed_line[3]);
      //swarm_move(1, 0, 0);
      break;
    case SLAMD_UPDATE:
      i = 0;
      while (!swarm_converged() && i < 1) {
     	i++;
	swarm_update(parsed_line + 1);
	if (!swarm_converged() && i < 5)
	  swarm_move(0, 0, 0);
      }
      printf("%d\n", i);
      scan_count++;
      //if ((scan_count == 5 && init == 1) || scan_count == 6) {
      //if (init == 1) init = 0;
      if(swarm_converged())
	swarm_map(parsed_line + 1);
	//scan_count = 0;
	//
      update_display();
      break;
    case SLAMD_X:
    case SLAMD_Y:
    case SLAMD_THETA:
      break;
    }

    iterations++;
  }

  fclose(data_file);
  fclose(output_file);

  free(line);

  //  save_map();

  //  glutMainLoop();

  return 0;
}

void save_map() {
  int i, s;
  landmark_map map;
  FILE *map_file = fopen("slamd_map.csv", "w");
  assert(map_file != NULL);

  map = swarm_get_map();

  // write swarm map to a file
  s = buffer_get_size();
  for (i = 0; i < s - 1; i++) {
    fprintf(map_file, "%u,%u,", map.map[i].seen, map.map[i].unseen);
  }
  fprintf(map_file, "%u,%u", map.map[i].seen, map.map[i].unseen);

  fclose(map_file);
}

landmark_map load_map() {
  int i, s;
  landmark_map map;
  char *str, *tok;
  FILE *map_file = fopen("slamd_map.csv", "r");
  assert(map_file != NULL);

  s = buffer_get_size();
  // 22 = 10 characters for 32 unsigned bits * 2 + 2 commas
  str = malloc(sizeof(char)*s*22);

  fgets(str, sizeof(char)*s*22, map_file);

  fclose(map_file);

  map.map = malloc(sizeof(landmark_map)*s);

  i = 0;
  while ((tok = strtok(str, ",")) != NULL) {
    if (i % 2 == 0)
      sscanf(tok, "%u", &(map.map[i/2].seen));
    else
      sscanf(tok, "%u", &(map.map[i/2].unseen));
  }

  free(str);

  return map;
}

void update_display() {
  int i, j, k;
  double s, c, t;
  particle *p;
  
  // noui  swarm_get_all_particles(&p);

  // copy best map to buffer
  // noui  swarm_get_best_buffer(map[0]);
  // copy map to buffer
  // noui  swarm_get_map_buffer(map[2]);
  if (current_command == SLAMD_UPDATE) {
    printf("x, y, theta = (%d, %d, %d)\n", swarm_get_best_x(),
	   swarm_get_best_y(),
	   swarm_get_best_theta());
    fprintf(output_file, "%d,%d,%d\n", swarm_get_best_x(),
	   swarm_get_best_y(),
	   swarm_get_best_theta());
  }

  /* noui
  //  for (k = 0; k < PARTICLE_COUNT; k++) {
  for (k = 0; k < 1; k++) {
    // update localization
    current_particle = p[k];
    // draw position if better than average probability
    t = swarm_get_theta(k)*M_PI/180;
    s = sin(t);
    c = cos(t);
    for (j = -20; j < 21; j++ ) {
      int lim = 21;
      if (j == 0) lim = 41;
      for (i = -20; i < lim; i++) {
	record_map_position(0, swarm_get_x(k) + i*c - j*s,
			    swarm_get_y(k) + i*s + j*c, 255);
	if (k == 0)// && current_command == SLAMD_UPDATE)
	  record_map_position(2, swarm_get_best_x() + i*c - j*s,
			      swarm_get_best_y() + i*s + j*c, 255);
      }
    }
  }

  // draw
  display();

  //  for (k = 0 ; k < PARTICLE_COUNT; k++) {
  for (k = 0 ; k < 1; k++) {
    current_particle = p[k];

    t = swarm_get_theta(k)*M_PI/180;
    s = sin(t);
    c = cos(t);

    // clear positions
    for (j = -20; j < 21; j++ ) {
      int lim = 21;
      if (j == 0) lim = 41;
      for (i = -20; i < lim; i++) {
	if (!x_y_protected(swarm_get_x(k) + i*c - j*s, swarm_get_y(k) + i*s + j*c))
	  record_map_position(0, swarm_get_x(k) + i*c - j*s,
			      swarm_get_y(k) + i*s + j*c, 0);
	if (k == 0 /&& current_command == SLAMD_UPDATE/ && !x_y_protected(swarm_get_best_x() + i*c - j*s, swarm_get_best_y() + i*s + j*c))
	  record_map_position(2, swarm_get_best_x() + i*c - j*s,
			      swarm_get_best_y() + i*s + j*c, 0);
      }
    }
  }
*/

  //  glutMainLoopEvent();
}

void record_map_position(int index, int x, int y, uint8_t value) {
  if (in_arena(x, y))
    map[index][buffer_index_from_x_y(x, y)] = value;
}

