#include "robot.h"

static mode robot_mode = ROBOT_INITIAL;
static const char* mode_strings[] = {"initial", "position lock", "position and orientation lock"};

void robot_set_mode(mode new_mode) {
  if (new_mode != robot_mode) {
    printf ("Switching modes: %s -> %s\n", mode_strings[robot_mode], mode_strings[new_mode]);
    robot_mode = new_mode;
  }
}

const char* robot_get_mode_string() {
  return mode_strings[robot_mode];
}

mode robot_get_mode() {
  return robot_mode;
}
