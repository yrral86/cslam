/*!
  @file
  @brief SCIP API to support user program
  @author aokaze (HARA Yoshitaka) <bluewind@roboken.esys.tsukuba.ac.jp>
  @author YoS (YOSHIDA Tomoaki) <yoshida@furo.org>
  @par    Publisher:
          HOKUYO AUTOMATIC CO.,LTD.
  @since  2006
  @date   2007/03/30
*/

#include "scipUtil.h"

//! Try to connect with SCIP1 and SCIP2 (Communication speed will not change)
/*!
  @param aPort [in] Sesnor's communication handler
  @retval 1 Connection successful with SCIP1 
  @retval 2 Connection successful with SCIP2
  @retval -1 Connection failed with SCIP1 and SCIP2
*/
int scipTryComm(tPort* aPort)
{
  char cmd[CMD_SIZE];
  char line[LINE_SIZE];
  int ret;

  flushSensorBuf(aPort->dev);
  fprintf(stdout,"Trying to communicate in SCIP1 ...\n");
  sprintf(cmd,"V\n");
  ret=scipSendCmd(aPort,cmd,1);
  while(fgets(line,sizeof(line),aPort->dev)!=NULL)
    if(strcmp(line,LF_STRING)==0)
      break;
  if(ret==0){
    fprintf(stdout,"Communicating in SCIP1\n");
    aPort->protocol=1;
    aPort->state=STATE_MEASURE_STORE;
    return 1;
  }

  flushSensorBuf(aPort->dev);
  fprintf(stdout,"Trying to communicate in SCIP2 ...\n");
  sprintf(cmd,"VV\n");
  ret=scipSendCmd(aPort,cmd,2);
  while(fgets(line,sizeof(line),aPort->dev)!=NULL)
    if(strcmp(line,LF_STRING)==0)
      break;
  if(ret==0){
    fprintf(stdout,"Communicating in SCIP2\n");
    aPort->protocol=2;
    aPort->state=STATE_IDLE;
    return 2;
  }

  flushSensorBuf(aPort->dev);
  return -1;
}

//! Connect to designated serial port with SCIP commands
/*!
  @param aDevName [in] Serial port device file name
  @return Sesnor's communication handler
  Returns NULL if failed
*/
tPort* scipConnect(char *aDevName)
{
  tPort *port;
  int ret;

  fprintf(stdout,"Connecting at 19200 bps ...\n");
  port=openPort(aDevName,19200);
  ret=scipTryComm(port);
  switch(ret){
    case 1:
    case 2:
      fprintf(stdout,"Connected at 19200 bps\n\n");
      return port;
    default:
      fprintf(stdout,"Could not connect at 19200 bps\n\n");
      break;
  }

  fprintf(stdout,"Connecting at 57600 bps ...\n");
  if(setupPort(fileno(port->dev),57600)!=EXIT_SUCCESS){
    perror("Could not set terminal parameters ");
    return NULL;
  }
  ret=scipTryComm(port);
  switch(ret){
    case 1:
    case 2:
      fprintf(stdout,"Connected at 57600 bps\n\n");
      return port;
    default:
      fprintf(stdout,"Could not connect at 57600 bps\n\n");
      break;
  }

  fprintf(stdout,"Connecting at 115200 bps ...\n");
  if(setupPort(fileno(port->dev),115200)!=EXIT_SUCCESS){
    perror("Could not set terminal parameters ");
    return NULL;
  }
  ret=scipTryComm(port);
  switch(ret){
    case 1:
    case 2:
      fprintf(stdout,"Connected at 115200 bps\n\n");
      return port;
    default:
      fprintf(stdout,"Could not connect at 115200 bps\n\n");
      break;
  }

  return NULL;
}

//! Adjust the scan data array number with starting step of scan
/*!
  @param aStartStep [in] scan data start step
  @param aStepCluster [in] cluster count (1-99)
  @param aId [in] Array number
  @return step number
*/
int id2step(int aStartStep, int aStepCluster, int aId)
{
  aStepCluster = aStepCluster==0 ? 1 : aStepCluster;
  return aStartStep + aId * aStepCluster;
}

//! Convert step number to angle (radian)
/*!
  @param aStep [in] step number
  @return radian angle
*/
double step2rad(int aStep)
{
  return (aStep-FRONT_STEP)*ANG_UNIT_RAD;
}
