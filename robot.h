#ifndef __ROBOT_H__
#define __ROBOT_H__

#include <stdio.h>

enum modeEnum {
  ROBOT_INITIAL,
  ROBOT_P_LOCK,
  ROBOT_PO_LOCK
};

typedef enum modeEnum mode;

void robot_set_mode(mode);
const char* robot_get_mode_string();
mode robot_get_mode();

#endif
