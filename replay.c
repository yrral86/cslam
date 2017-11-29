#include "slam.h"
#include "slamd.h"
#include "replay.h"

#include "lazygl.h"

#include <time.h>

long getMillis() {
  struct timespec spec;
  clock_gettime(CLOCK_MONOTONIC, &spec);

  return 1000*spec.tv_sec + lround(spec.tv_nsec / 1.0e6);
}

static int current_command;
FILE *output_file;

int main (int argc, char **argv) {
  int i, iterations, scan_count, parsed_line[723];
  FILE *data_file;
  ssize_t read;
  char *int_string;
  char *line = NULL;
  size_t length  = 0;
  long start_time;

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

      start_time = getMillis();
      break;
    case SLAMD_MOVE:
      swarm_move(parsed_line[1], parsed_line[2], parsed_line[3]);
      break;
    case SLAMD_UPDATE:
      i = 0;
      while (!swarm_converged() && i < 1) {
     	i++;
	swarm_update(parsed_line + 1);
	if (!swarm_converged() && i < 5)
	  swarm_move(0, 0, 0);
      }
      scan_count++;
      if(swarm_converged())
	swarm_map(parsed_line + 1);
      break;
    case SLAMD_X:
    case SLAMD_Y:
    case SLAMD_THETA:
      break;
    }

    iterations++;
  }

  long difference = getMillis() - start_time;
  fprintf(output_file, "%ld", difference);

  fclose(data_file);
  fclose(output_file);

  free(line);

  return 0;
}
