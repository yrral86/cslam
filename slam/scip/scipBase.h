#ifndef SCIP_BASE_H
#define SCIP_BASE_H
/*!
  @file
  @brief SCIP Base API
  @author aokaze (HARA Yoshitaka) <bluewind@roboken.esys.tsukuba.ac.jp>
  @author YoS (YOSHIDA Tomoaki) <yoshida@furo.org>
  @par    Publisher:
          HOKUYO AUTOMATIC CO.,LTD.
  @since  2005
  @date   2007/03/30
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/termios.h>
#include <unistd.h>
#include <math.h>

// hardcode parameters for URG
// TODO: Obtain parameter of SCIP2 Command(PP-Command)
#define FRONT_STEP 384
#define ANG_RESOLUTION 1024

#define LINE_SIZE 80
#define CMD_SIZE 33
#define LF 0x0A
static const char LF_STRING[]={LF,'\0'};

//! Sesnor's operating state 
typedef enum
{
  STATE_UNKNOWN=0,      //!< Unknown state
  STATE_IDLE,           //!< Idle (Laser off)
  STATE_MEASURE_SEND,   //!< Measurement and data transmission (Laser on, continuous mode)
  STATE_MEASURE_STORE,  //!< Measurement and data store (Laser on, classic mode)
  STATE_TIME_ADJUST     //!< Time adjustment mode
}State_t;

//! Data encoding type
typedef enum
{
  ENC_2BYTE=2,  //!< 2 Character encoding
  ENC_3BYTE,    //!< 3 Character encoding
  ENC_4BYTE     //!< 4 Character encoding(used in Timestamp)
}EncType_t;

//! Sesnor's communication handler
typedef struct
{
  FILE *dev;     //!< File handler for sesnor connected port
  int protocol;  //!< Protocol major version (UNKNOWN if 0)
  State_t state; //!< Sesnor's operating state 
}tPort;


// Functions releated to serial communication ports

//! Communication speed setting 
/*!
  @param aBR [in] communication speed (9600-500000bps)
  Not compatible with 750000bps 
  @return cfset[i|o]speed return value of function.
  Returns B0 if fails
*/
speed_t bitrate(int aBR);

//! Obtain current serial communication speed
/*!
  @param aFD [in] File descriptor for serial port.
  @return current serial communication speed
*/
speed_t getBitrate(int aFD);

//! Remove the existing data in received buffer.
/*!
  @param aDev [in] Port file handler of sensor port
*/
void skipReceiveBuf(FILE *aDev);

//! Clear sensor buffer 
/*!
  Send LF to clear the sensor buffer. It is not necessary to use this funciton
  if highly trusted serial communication (serial port or USB-Serial converter) is used.
  In such case response will be quick but the probaility of communication error will increase. 
  @param aDev [in] File handler for sensors communication port.
*/
void flushSensorBuf(FILE* aDev);

//! Communication parameters for serial port.
/*!
  @param aFD [in] File descriptor for serial port.
  @param aSpeed [in] communication speed (bps)
  @retval EXIT_SUCCESS Success
  @retval EXIT_FAILURE Fail
*/
int setupPort(int aFD, int aSpeed);

//! Open the designated serial port for the set up 
/*!
  @param aDevName [in] Serial port device file name.
  @param aSpeed [in] communication speed (bps)
  @return Sesnor's communication handler.
  Returns NULL if failed.
*/
tPort* openPort(char *aDevName, int aSpeed);

//! Close the designated communcation handler.
/*!
  @param aPort [in] Sesnor's communication handler.
  @retval EXIT_SUCCESS Success
  @retval EXIT_FAILURE Fail
*/
int closePort(tPort *aPort);


// SCIP Command transmission/reception base function. 

//! Check the major communication protocol version with sensor's communication handler.
/*!
  @param aPort [in] Sesnor's communication handler.
  @param aProtocol [in] SCIP command major protocol version.
  @retval 1 When protocol match (or Unknown)
  @retval 0 When protocol mismatch
*/
int scipProtCheck(tPort* aPort, int aProtocol);

//! Transmit SCIP command and receive the data upto status 
/*!
  Transmit SCIP command obtained as aCmd to sensor connected to aDev port,
  From the senosr's reply, only receive data upto status.
  Remaining data has different length depending upon the transmitted command.
  Therefore it is necessary to receiver the remaining command separately 
  until the termination code or 2 continuous linefeed.
  
  @param aPort [in] Sesnor's communication handler.
  @param aCmd [in]  SCIP command to be transmitted.
  @param aProtocol [in] Major protocol version of SCIP to be transmitted.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int scipSendCmd(tPort* aPort, char *aCmd, int aProtocol);

//! Receive the last LF of the sensor's reply (response)
/*!
  @param aPort [in] Sesnor's communication handler.
  @retval 0 Success
  @retval -1 Failed to get the reply (response)
  @retval -2 Termination code is not LF
*/
int scipReceiveResEnd(tPort* aPort);

//! Decode the data
/*!
  @param aData [in] Encoded data
  @param aEncLen [in] Encode type(Character count of Character encoding type)
  @return Decoded data
*/
int scipDecode(char *aData, int aEncLen);

#endif  // SCIP_BASE_H
