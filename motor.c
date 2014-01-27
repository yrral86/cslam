#include "motor.h"

const int LENGTH = 3;
const int VELOCITY = 3;

void motor_move(int angle, particle *p) {

  //  printf("steering_angle: %i theta: %g\n", angle, p->theta);

  double theta = (p->theta + angle)*M_PI/180;
  double dx = VELOCITY*cos(theta);
  double dy = -VELOCITY*sin(theta);

  //  printf("dx: %g dy: %g\n\n\n", dx, dy);

  p->x += dx;
  p->y += dy;
  p->theta += (double)angle/LENGTH;
}
