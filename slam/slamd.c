#include "slamd.h"

int main(int argc, char **argv) {
  int inputs[5], i;

  if (argc != 6) {
    printf("Usage: %s sensor_steps sensor_degrees arena_long arena_short start\n", argv[0]);
    return 1;
  }

  for (i = 0; i < 5; i++)
    if (sscanf(argv[i+1], "%i", inputs + i) != 1) {
      printf("argument %i is not an integer\n", i + 1);
      return 1;
    }

  // initialize swarm
  swarm_init(inputs[0], inputs[1], inputs[2], inputs[3], inputs[4]);

  // set up shared memory
  param_sem = CreateSemaphore(NULL, 0, 1, param_sem_name);
  return_sem = CreateSemaphore(NULL, 0, 1, return_sem_name);
  assert(param_sem != NULL);
  assert(return_sem != NULL);

  HANDLE ap = CreateFileMapping(

  while (1) {

  }

}
