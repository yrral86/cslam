/*!
  @file		sharedLidar.h
  @brief	Contains structures used by procLIDAR in shared memory.
  @author	Brad Nelson			<nelsobra@onid.orst.edu>
*/

struct lidarScan {
  int data[681];
  int timestamp;
};

struct lidarMinScan {
  int data[13];
  int timestamp;
};
