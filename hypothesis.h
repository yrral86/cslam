#ifndef __HYPOTHESIS_H__
#define __HYPOTHESIS_H__

#include "observation.h"

typedef struct hypothesis {
  unsigned int x;
  unsigned int y;
  int theta;
  observations *obs;
} hypothesis;


#endif
