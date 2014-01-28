/*
      Filename:      share.h
      Authors:       Paul Filitchkin
      Date Created:  03 April 2009
      Description:   Shared definitions for reading settings files and misc definitions
*/

#ifndef INCLUDED_SHARE_H
#define INCLUDED_SHARE_H

#ifdef __cplusplus
    extern "C" {
#endif

#include <semaphore.h>

#define byte unsigned char

//========================================Debug defines=============================================
#define DEBUG_ON 1
#define DEBUG if( DEBUG_ON )

//======================================Semaphore definitions=======================================

//Useful struct for semaphores
union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

//Semaphore names (unique IDs to label semaphores)
#define NET_SEM_IN  "netSeIn"
#define NET_SEM_OUT "netSeOut"

#define ZIG_SEM_IN 	"zigSemIn"
#define ZIG_SEM_OUT "zigSemIn"

//=====================================Shared memory definitions====================================

#define UNIQUE_ID 'A'   //Unique identifier (used for shared memory creation)

//These ids are used to identify shared memory segments
//These should exactly match the PROC_NAME parameter specified in settings.txt
#define IMU_SHM "imu"
#define MOT_SHM "mot"
#define NET_SHM "net"
#define GUI_SHM "gui"
#define ZIG_SHM "zig"
#define MASTER_SHM "master"
#define LIDAR_SHM "lidar"

//======================================Shared memory offsets=====================================//
//Shared memory types
#define MOT_SHM_TYPE 	0
#define IMU_SHM_TYPE 	1
#define NET_SHM_TYPE 	2
#define GUI_SHM_TYPE 	3
#define ZIG_SHM_TYPE 	4
#define MASTER_SHM_TYPE 5
#define LIDAR_SHM_TYPE	6

//MOT shared memory offsets
#define MOT_OFFSET_VV_CNT                0x0000  //Memory offset to VelocityVectorStruct count
#define MOT_OFFSET_VV                    0x0001  //Memory offset to VelocityVectorStruct 
#define MOT_OFFSET_MS_CNT                0x000D  //Memory offset to MotorSpeedsStruct count
#define MOT_OFFSET_MS                    0x000E  //Memory offset to MotorSpeedsStruct
#define MOT_OFFSET_SP_CNT                0x0012  //Memory offset to SetPointStructure count
#define MOT_OFFSET_SP                    0x0013  //Memory offset to SetPointStructure

//----------------

#define MOT_OFFSET_PID_MODE_CNT          0x0100
#define MOT_OFFSET_PID_MODE              0x0101

#define MOT_OFFSET_TARGET_POWER_CNT      0x0102
#define MOT_OFFSET_TARGET_POWER          0x0103

#define MOT_OFFSET_MAX_POWER_CNT         0x0104 
#define MOT_OFFSET_MAX_POWER			 0x0105

#define MOT_OFFSET_RESET_I_CNT           0x0106
#define MOT_OFFSET_RESET_I               0x0107  

#define MOT_OFFSET_ENABLE_CNT            0x0108
#define MOT_OFFSET_ENABLE_GROUP          0x0109 //(repeat intentional)
#define MOT_OFFSET_ENABLE_PITCH          0x0109
#define MOT_OFFSET_ENABLE_YAW            0x010A
#define MOT_OFFSET_ENABLE_ROLL           0x010B
#define MOT_OFFSET_ENABLE_Z              0x010C
#define MOT_OFFSET_ENABLE_MOTORS         0x010D

#define MOT_OFFSET_MODE_CNT              0x010E
#define MOT_OFFSET_MODE                  0x010F

//~ #define MOT_OFFSET_PID_CONST_GROUP_CNT   0x007F  //Memory offset to group of pidSettingsStructs
//~ #define MOT_OFFSET_PID_CONST_GROUP       0x0080  //Memory offset to group of pidSettingsStructs (repeat intentional)
//~ #define MOT_OFFSET_PID_CONST_PITCH       0x0080  //Memory offset to pidSettingsStruct for pitch
//~ #define MOT_OFFSET_PID_CONST_YAW         0x0094  //Memory offset to pidSettingsStruct for yaw
//~ #define MOT_OFFSET_PID_CONST_ROLL        0x00A8  //Memory offset to pidSettingsStruct for roll
//~ #define MOT_OFFSET_PID_CONST_PITCH_RATE  0x00BC  //Memory offset to pidSettingsStruct for pitch rate
//~ #define MOT_OFFSET_PID_CONST_YAW_RATE    0x00D0  //Memory offset to pidSettingsStruct for yaw rate
//~ #define MOT_OFFSET_PID_CONST_ROLL_RATE   0x00E4  //Memory offset to pidSettingsStruct for roll rate

#define MOT_OFFSET_PID_CONST_GROUP_CNT   0X0110  //Memory offset to group of pidSettingsStructs
#define MOT_OFFSET_PID_CONST_GROUP       0x0111  //Memory offset to group of pidSettingsStructs (repeat intentional)

#define MOT_OFFSET_PID_CONST_PITCH       0x0111  //Memory offset to pidSettingsStruct for pitch
#define MOT_OFFSET_PID_CONST_YAW         0x0125  //Memory offset to pidSettingsStruct for yaw
#define MOT_OFFSET_PID_CONST_ROLL        0x0139  //Memory offset to pidSettingsStruct for roll
#define MOT_OFFSET_PID_CONST_PITCH_RATE  0x014D  //Memory offset to pidSettingsStruct for pitch rate
#define MOT_OFFSET_PID_CONST_YAW_RATE    0x0161  //Memory offset to pidSettingsStruct for yaw rate
#define MOT_OFFSET_PID_CONST_ROLL_RATE   0x0175  //Memory offset to pidSettingsStruct for roll rate

#define MOT_OFFSET_PID_CONST_X           0x0189  //Memory offset to pidSettingsStruct for x
#define MOT_OFFSET_PID_CONST_Y           0x019D  //Memory offset to pidSettingsStruct for y
#define MOT_OFFSET_PID_CONST_Z           0x01B1  //Memory offset to pidSettingsStruct for z
#define MOT_OFFSET_PID_CONST_X_RATE      0x01C5  //Memory offset to pidSettingsStruct for x rate
#define MOT_OFFSET_PID_CONST_Y_RATE      0x01D9  //Memory offset to pidSettingsStruct for y rate
#define MOT_OFFSET_PID_CONST_Z_RATE      0x01ED  //Memory offset to pidSettingsStruct for z rate

#define MOT_OFFSET_US_ALT		 0x0201  //Memory offset to ultrasonic altitude

//NET shared memory defines
#define NET_OFFSET_OUT        0x00F0
#define NET_BUFFER_SIZE_OUT   2000

//IMU shared memory defines
#define IMU_OFFSET_ACCEL_EULER_CNT	0x0000 //Memory offset to euler angles struct count
#define IMU_OFFSET_ACCEL_EULER	    0x0001 //Memory offset to euler angles struct

//GUI shared memory defines
#define GUI_OFFSET_IMU_DATA			0x0001

//ZIG shared memory defines
#define ZIG_OFFSET_IN			0x0000
#define ZIG_OFFSET_OUT			0x0100

#define ZIG_BUFFER_SIZE_IN		100
#define ZIG_BUFFER_SIZE_OUT		300

//MASTER shared memory defines
#define MAIN_ENABLE				0x0000

//LIDAR shared memory defines
#define LIDAR_OFFSET_DATA_CNT	0x0000
#define LIDAR_OFFSET_DATA 		0x0004
#define LIDAR_OFFSET_MIN_DATA	0x6A94
#define LIDAR_OFFSET_NEXT		0x6CC4

//==================================Settings file related defines===================================
#define SETTINGSPATH "./settings.txt" //Path to settings file
#define VERSION 2        //Settings file version
#define MAXBUF       255 //Max buffer of several strings in Settings struct
#define MAX_PROCS    20  //Maximum number of processes the run binary will execute
#define MAX_VAL_LEN  40  //Maximum length of val (second parameter of each line) in settings file
#define MAX_NAME_LEN 20  //Maximum length of name (first parameter of each line) in settings file

//Settings are stored in struct once they are read from the settings file
struct Settings {
    int numProcs;        //Number of processes with access to shared memory
    char *path;          //Path to the project directory (used for creating shared memory key)
    char *procPaths;     //Path to each process
    byte *procPathsIdx;  //Index of each process path
    char *procNames;     //Path to each process
    byte *procNamesIdx;  //Index of each process path
    int  *procMemSize;   //Size of shared memory segment for each process
};

//============================================Prototypes============================================

int fileExists(const char *filename);
struct Settings *initSettings(void);
void getSettings(char *settingsPath, struct Settings *s);
key_t initKey(char *s);
int initSharedMem(key_t key, int size);
unsigned char *sharedMemPtr(int shmID);
unsigned char *attachSharedMem(struct Settings *s, char *attachName);
void dettachSharedMem(unsigned char *ptr);
int getSharedMemSize(struct Settings *s, char *nameKey);

sem_t *initSemaphore(char *semName);
void destorySemaphore(sem_t *sem, char *semName);

int serializePacket( unsigned char *serialBuffer, unsigned char type, int offset, void *dataPointer, int dataSize);
int decodePacket( unsigned char *serialBuffer, unsigned char *pktSize, unsigned char *type, int *offset);

#ifdef __cplusplus
    }
#endif 
#endif
