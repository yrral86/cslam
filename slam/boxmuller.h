#ifndef __BOXMULLER__
#define __BOXMULLER__

#include <math.h>


extern float ranf();         /* ranf() is uniform in 0..1 */


float box_muller(float, float);

#endif