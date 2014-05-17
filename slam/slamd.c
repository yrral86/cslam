#include "slamd.h"

int main(int argc, char **argv) {
  int i, m, param_size, return_size, last_command;
  FILE *record;

  // set up shared memory
  param_sem = CreateSemaphore(NULL, 0, 1, param_sem_name);
  return_sem = CreateSemaphore(NULL, 0, 1, return_sem_name);
  assert(param_sem != NULL);
  assert(return_sem != NULL);

  // parameter space size is (sensor readings + 1) * sizeof(int)
  // first int specifies which function, remainder are params
  // TODO: fix constant 1081/721
  param_size = (721 + 1)*sizeof(int);
  param_handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, param_size, param_shm_name);

  // return space size is sizeof(int)
  return_size = sizeof(int);
  return_handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, return_size, return_shm_name);

  params = (int*)MapViewOfFile(param_handle, FILE_MAP_WRITE, 0, 0, param_size);
  assert(params != NULL);

  return_value = (int*)MapViewOfFile(return_handle, FILE_MAP_WRITE, 0, 0, return_size);
  assert(return_value != NULL);

  m = 0;
  while (1) {
    WaitForSingleObject(param_sem, INFINITE);
    record = fopen("slamd_record.csv", "a");
	last_command = params[0];
    switch (params[0]) {
    case SLAMD_INIT:
      fprintf(record, "init\n");
      m = params[1];
      printf("calling swarm_init(%i, %i, %i, %i, %i)\n", params[1], params[2], params[3], params[4], params[5], params[6]);
      swarm_init_internal(params[1], params[2], params[3], params[4], params[5]);
      break;
    case SLAMD_MOVE:
      printf("calling swarm_move(%i, %i, %i)\n", params[1], params[2], params[3]);
      swarm_move_internal(params[1], params[2], params[3]);
      break;
    case SLAMD_UPDATE:
      printf("calling swarm_update(%i, %i, %i, %i, %i, .... )\n", params[1], params[2], params[3], params[4], params[5]);
      swarm_update_internal(params + 1);
      break;
    case SLAMD_CONVERGED:
      *return_value = swarm_converged_internal();
      printf("converged: %i\n", *return_value);
      break;
    case SLAMD_X:
      *return_value = swarm_get_best_x_internal();
      printf("x value: %i\n", *return_value);
      break;
    case SLAMD_Y:
      *return_value = swarm_get_best_y_internal();
      printf("y value: %i\n", *return_value);
      break;
    case SLAMD_THETA:
      *return_value = swarm_get_best_theta_internal();
      printf("theta value: %i\n", *return_value);
      break;
    }

	// write to file
    for (i = 0; i < m + 1; i++)
      fprintf(record, "%i,", params[i]);
    fprintf(record, "%i\n", return_value[0]);
    fclose(record);
	

    // UPDATE releases semaphore inline to return
    // as early as possible while it normalizes,
    // sorts, and resamples in the background
    if (last_command != SLAMD_UPDATE)
      ReleaseSemaphore(return_sem, 1, NULL);
  }

  return 0;
}
