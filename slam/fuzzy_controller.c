#include "fuzzy_controller.h"

int fuzzy_get_angle(particle p) {
  int angle = 0;
  double t = p.theta;

  if (t > 180)
    t -= 360;

  if (p.theta > 0)
    angle = -15;
  else if (p.theta < 0)
    angle = 15;
  return angle;
}
