#include "motor.h"

const int LENGTH = 3;
const int VELOCITY = 3;

void motor_move(int steering_angle, particle *p) {

  //  printf("steering_angle: %i theta: %g\n", steering_angle, p->theta);

  p->theta += VELOCITY*sin(steering_angle*M_PI/180)/LENGTH;
  if (p->theta > 180)
    p->theta -= 360;
  else if (p->theta < -180)
    p->theta += 360;

  double theta = (p->theta + steering_angle)*M_PI/180;
  double dx = VELOCITY*cos(theta);
  double dy = VELOCITY*sin(theta);

  //  printf("dx: %g dy: %g\n\n\n", dx, dy);

  p->x += dx;
  p->y += dy;
}
