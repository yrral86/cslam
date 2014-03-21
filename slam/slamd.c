#include "slamd.h"

int main(int argc, char **argv) {
  int inputs[5], i, param_size, return_size;

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
  swarm_init_internal(inputs[0], inputs[1], inputs[2], inputs[3], inputs[4]);

  // set up shared memory
  param_sem = CreateSemaphore(NULL, 0, 1, param_sem_name);
  return_sem = CreateSemaphore(NULL, 0, 1, return_sem_name);
  assert(param_sem != NULL);
  assert(return_sem != NULL);

  // parameter space size is (sensor readings + 1) * sizeof(int)
  // first int specifies which function, remainder are params
  param_size = (inputs[0] + 1)*sizeof(int);
  param_handle = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, param_size, param_shm_name);

  // return space size is sizeof(int)
  return_size = sizeof(int);
  return_handle = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, return_size, return_shm_name);

  params = (int*)MapViewOfFile(param_handle, FILE_MAP_WRITE, 0, 0, param_size);
  assert(params != NULL);

  return_value = (int*)MapViewOfFile(return_handle, FILE_MAP_WRITE, 0, 0, return_size);
  assert(return_value != NULL);

//  ReleaseSemaphore(return_sem);

  while (1) {
	  WaitForSingleObject(param_sem, INFINITE);
	  switch (params[0]) {
	  case SLAMD_MOVE:
		  swarm_move_internal(params[1], params[2], params[3]);
		  break;
	  case SLAMD_UPDATE:
		  swarm_update_internal(params + 1);
		  break;
	  case SLAMD_X:
		  *return_value = swarm_get_best_x_internal();
		  break;
	  case SLAMD_Y:
		  *return_value = swarm_get_best_y_internal();
		  break;
	  case SLAMD_THETA:
		  *return_value = swarm_get_best_theta_internal();
		  break;
	  }
	  ReleaseSemaphore(return_sem, 1, NULL);
  }

  return 0;
}
