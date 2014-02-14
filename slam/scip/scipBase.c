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

#include "scipBase.h"

// Functions releated to serial communication ports.

//! Communication speed setting 
/*!
  @param aBR [in] communication speed (9600-500000bps)
  Not compatible with 750000bps 
  @return cfset[i|o]speed return value of function.
  Returns B0 if fails
*/
speed_t bitrate(int aBR)
{
  switch(aBR)
    {
    case 9600:
      return B9600;

    case 19200:
      return B19200;

    case 38400:
      return B38400;

    case 57600:
      return B57600;

    case 115200:
      return B115200;

    case 230400:
      return B230400;

    case 460800:
      return B460800;

    case 500000:
      return B500000;

    default:  // invalid bitrate
      return B0;
    }
}

//! Obtain current serial communication speed
/*!
  @param aFD [in] File descriptor for serial port.
  @return current serial communication speed
*/
speed_t getBitrate(int aFD)
{
  struct termios tio;

  tcgetattr(aFD,&tio);
  return cfgetospeed(&tio);
}

//! Remove the existing data in received buffer.
/*!
  @param aDev [in] Port file handler of sensor port
*/
void skipReceiveBuf(FILE *aDev)
{
  char line[LINE_SIZE];

  tcflush(fileno(aDev),TCIOFLUSH);
  while(fgets(line,sizeof(line),aDev)!=NULL)
    if(strcmp(line,LF_STRING)!=0)
      fprintf(stdout,"skipping receive buffer : %s\n",line);
}

//! Clear sensor buffer 
/*!
  Send LF to clear the sensor buffer. It is not necessary to use this funciton
  if highly trusted serial communication (serial port or USB-Serial converter) is used.
  In such case response will be quick but the probaility of communication error will increase. 
  @param aDev [in] File handler for sensors communication port.
*/
void flushSensorBuf(FILE* aDev)
{
  fwrite("\n",sizeof(char),1,aDev);
  usleep(100000);  // 100 ms (URG's response time)
  skipReceiveBuf(aDev);
}

//! Communication parameters for serial port.
/*!
  @param aFD [in] File descriptor for serial port.
  @param aSpeed [in] communication speed (bps)
  @retval EXIT_SUCCESS Success
  @retval EXIT_FAILURE Fail
*/
int setupPort(int aFD, int aSpeed)
{
  speed_t speed;
  struct termios tio;

  speed=bitrate(aSpeed);
  if(speed==B0){
    fprintf(stderr,"INVALID BITRATE\n");
    return EXIT_FAILURE;  // invalid bitrate
  }

  tcgetattr(aFD,&tio);

  //   cfsetspeed(&tio,speed);  // bitrate
  cfsetospeed(&tio,speed);  // bitrate
  cfsetispeed(&tio,speed);  // bitrate

  cfmakeraw(&tio);
  tio.c_cflag&= ~CRTSCTS;  // no CTS/RTS  No hardware flow control.
#if 0
  tio.c_iflag &= ~( BRKINT | ICRNL | ISTRIP );
  tio.c_iflag &= ~IXON;     // no XON/XOFF
  tio.c_cflag &= ~PARENB;   // no parity
  tio.c_cflag &= ~CSTOPB;   // stop bit = 1bit
  tio.c_cflag = (tio.c_cflag & ~CSIZE) | CS8;  // data bits = 8bit
  tio.c_lflag &= ~( ISIG | ICANON | ECHO );    // Other
#endif

  // Non canonical mode option 
  tio.c_cc[VTIME] = 5;  // 500 ms to timeout
  tio.c_cc[VMIN] = 0;   // Minimum length

  // Commit
  if(tcsetattr(aFD,TCSADRAIN,&tio)==0)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

//! Open the designated serial port for the set up 
/*!
  @param aDevName [in] Serial port device file name.
  @param aSpeed [in] communication speed (bps)
  @return Sesnor's communication handler.
  Returns NULL if failed.
*/
tPort* openPort(char *aDevName, int aSpeed)
{
  tPort* port;
  FILE* dev;

  dev=fopen(aDevName,"w+");
  if(dev==NULL)
    return NULL;  // invalid device file

  // setup parameters
  if(setupPort(fileno(dev),aSpeed)==EXIT_SUCCESS){
#if 1
    flushSensorBuf(dev);
#endif
    port=(tPort*)calloc(1,sizeof(tPort));
    port->dev=dev;
    return port;
  } else {  // fail
    perror("Could not set terminal parameters ");
    fclose(dev);
    return NULL;
  }
}

//! Close the designated communcation handler.
/*!
  @param aPort [in] Sesnor's communication handler.
  @retval EXIT_SUCCESS Success
  @retval EXIT_FAILURE Fail
*/
int closePort(tPort *aPort)
{
  if(!aPort)
    return EXIT_FAILURE;
  flushSensorBuf(aPort->dev);
  fclose(aPort->dev);
  free(aPort);
  return EXIT_SUCCESS;
}


// SCIP Command transmission/reception base function. 

//! Check the major communication protocol version with sensor's communication handler.
/*!
  @param aPort [in] Sesnor's communication handler.
  @param aProtocol [in] SCIP command major protocol version.
  @retval 1 When protocol match (or Unknown)
  @retval 0 When protocol mismatch
*/
int scipProtCheck(tPort* aPort, int aProtocol)
{
  if(aPort->protocol==aProtocol || aPort->protocol==0)
    return 1;
  else
    return 0;
}

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
  
int scipSendCmd(tPort* aPort, char *aCmd, int aProtocol)
{
  static const char SCIP1_NORMAL_STATUS[]={'0',LF,'\0'};
  static const char SCIP2_NORMAL_STATUS[]={'0','0','P',LF,'\0'};
  char line[LINE_SIZE];
  int i;

  if(!scipProtCheck(aPort,aProtocol)){
    fprintf(stderr,"INVALID VERSION SCIP COMMAND :\tThe sensor is not work in SCIP%d mode\n",aProtocol);
    return -2;
  }

  i=fwrite(aCmd,sizeof(char),strlen(aCmd),aPort->dev);
  if(i==0){
    perror("Could not send command ");
    return -1;
  }

  // echo-back
  if(fgets(line,sizeof(line),aPort->dev)==NULL){
    fprintf(stderr,"NO ECHO BACK\n");
    return -1;
  }
  if(strcmp(line,aCmd)!=0){
    // invalid
    fprintf(stderr,"INVALID ECHO BACK :\tExp = %s\tRec = %s\n",aCmd,line);
    skipReceiveBuf(aPort->dev);
    return -1;
  }

  // status
  if(fgets(line,sizeof(line),aPort->dev)==NULL){
    fprintf(stderr,"NO STATUS\n");
    return -1;
  }
// #ifdef DEBUG
/*#if 1
  fprintf(stdout,"status = %s",line);
#endif */
  // It may be necessary to check the version. However at the stage when scipTryComm() is called version is unknown.
  if(!(strcmp(line,SCIP1_NORMAL_STATUS)==0) &&
     !(strcmp(line,SCIP2_NORMAL_STATUS)==0)){
    //invalid
    fprintf(stderr,"STATUS CHECK FAILED : %s",line);
    skipReceiveBuf(aPort->dev);
    i=atoi(line);
    if(i==0)
      i=-2;
    return i;
  }
#if 0
  aPort->protocol=aProtocol;
#endif
  return 0;
}

//! Receive the last LF of the sensor's reply (response)
/*!
  @param aPort [in] Sesnor's communication handler.
  @retval 0 Success
  @retval -1 Failed to get the reply (response)
  @retval -2 Termination code is not LF
*/
int scipReceiveResEnd(tPort* aPort)
{
  char line[LINE_SIZE];

  if(fgets(line,sizeof(line),aPort->dev)==NULL)
    return -1;
  if(strcmp(line,LF_STRING)!=0){  // invalid
    fprintf(stderr,"This response is not the end code\n");
    skipReceiveBuf(aPort->dev);
    return -2;
  }
  return 0;
}

//! Decode the data
/*!
  @param aData [in] Encoded data
  @param aEncLen [in] Encode type(Character count of Character encoding type)
  @return Decoded data
*/
int scipDecode(char *aData, int aEncLen)
{
  int ret=0;
  while(aEncLen--){
    if(*aData<0x30 || *aData>0x6f){
      fprintf(stderr,"Could not decode : %s\n",aData);
      return -1;
    }
    ret= (ret<<6) + (*(aData++) - 0x30);
  }
  return ret;
}
