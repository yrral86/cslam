#ifndef __HYPOTHESIS_H__
#define __HYPOTHESIS_H__

#include "observation.h"

typedef struct hypothesis {
  double x;
  double y;
  double theta;
  observations *obs;
} hypothesis;


#endif
