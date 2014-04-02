#include "slam.h"
#include "slamd.h"
#include "replay.h"

#include "lazygl.h"

// 3, one for current, one for history, one for display
#define BUFFER_HISTORY 3
static uint8_t* map[BUFFER_HISTORY];

static particle current_particle;

int main (int argc, char **argv) {
  // sensor_scans + function indicator + return
  int i, iterations, parsed_line[723];
  FILE *data_file;
  ssize_t read;
  char *int_string;
  char *line = NULL;
  size_t length  = 0;

  data_file = fopen("slamd_record.csv", "r");
  assert(data_file != NULL);

  iterations = 0;

  while((read = getline(&line, &length, data_file)) != -1) {
    int_string = strtok(line, ",");
    if (strncmp(int_string, "init", 4) == 0)
      continue;
    sscanf(int_string, "%d", parsed_line);
    for (i = 1; i < 722; i++) {
      int_string = strtok(NULL, ",");
      sscanf(int_string, "%d", parsed_line + i);
    }
    int_string = strtok(NULL, ",");
    sscanf(int_string, "%d\n", parsed_line + i);

    switch(parsed_line[0]) {
    case SLAMD_INIT:
      printf("before init\n");
      swarm_init(parsed_line[1], parsed_line[2], parsed_line[3], parsed_line[4], parsed_line[5]);
      printf("after init\n");

      // allocate buffers
      for (i = 0; i < BUFFER_HISTORY; i++)
	map[i] = buffer_allocate();

      glutInit(&argc, argv);
      // pass size of buffer, then window size
      initGL(map[0], map[2], buffer_get_width(), buffer_get_height(), buffer_get_width(), buffer_get_height());

      break;
    case SLAMD_MOVE:
      printf("before move\n");
      swarm_move(parsed_line[1], parsed_line[2], parsed_line[3]);
      printf("after move\n");
      break;
    case SLAMD_UPDATE:
      printf("before update\n");
      swarm_update(parsed_line + 1);
      printf("after update/before display\n");
      update_display();
      printf("after display\n");
      break;
    case SLAMD_X:
    case SLAMD_Y:
    case SLAMD_THETA:
      break;
    }

    iterations++;
  }

  fclose(data_file);

  free(line);

  glutMainLoop();

  return 0;
}

void update_display() {
  int i, j;
  double s, c, t;

  // update localization
  current_particle.x = swarm_get_best_x();
  current_particle.y = swarm_get_best_y();
  current_particle.theta = swarm_get_best_theta();

  printf("x, y, theta = (%d, %d, %d)\n", current_particle.x,
	 current_particle.x,
	 current_particle.theta);

  // copy best map to buffer
  swarm_get_best_buffer(map[0]);

  // draw position
  t = current_particle.theta*M_PI/180;
  s = sin(t);
  c = cos(t);
  for (j = -50; j < 51; j++ ) {
    int lim = 51;
    if (j == 0) lim = 71;
    for (i = -50; i < lim; i++) {
      record_map_position(0, current_particle.x + i*c - j*s,
			  current_particle.y + i*s + j*c, 255);
      record_map_position(2, current_particle.x + i*c - j*s,
			  current_particle.y + i*s + j*c, 255);
    }
  }

  // draw
  display();

  // clear position
  for (j = -50; j < 51; j++ ) {
    int lim = 51;
    if (j == 0) lim = 71;
    for (i = -50; i < lim; i++) {
      record_map_position(0, current_particle.x + i*c - j*s,
			  current_particle.y + i*s + j*c, 0);
      if (!x_y_protected(current_particle.x + i*c - j*s, current_particle.y + i*s + j*c))
	record_map_position(2, current_particle.x + i*c - j*s,
			    current_particle.y + i*s + j*c, 0);
    }
  }

  glutMainLoopEvent();
}

void record_map_position(int index, int x, int y, uint8_t value) {
  if (in_arena(x, y))
    map[index][buffer_index_from_x_y(x, y)] = value;
}

