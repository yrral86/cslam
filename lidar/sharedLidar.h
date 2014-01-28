/*!
  @file		sharedLidar.h
  @brief	Contains structures used by procLIDAR in shared memory.
  @author	Brad Nelson			<nelsobra@onid.orst.edu>
*/

#define LIDAR_1_WIDTH = 681;

struct lidarScan {
  int data[LIDAR_1_WIDTH];
  int timestamp;
};

struct lidarMinScan {
  int data[13];
  int timestamp;
};
