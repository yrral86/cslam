/*!
  @file
  @brief SCIP1 API
  @author aokaze (HARA Yoshitaka) <bluewind@roboken.esys.tsukuba.ac.jp>
  @author YoS (YOSHIDA Tomoaki) <yoshida@furo.org>
  @par    Publisher:
          HOKUYO AUTOMATIC CO.,LTD.
  @since  2005
  @date   2007/03/30
*/

#include "scip1.h"

//! Display sensor version [V-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int scip1ShowVersion(tPort* aPort)
{
  char cmd[CMD_SIZE];
  char line[LINE_SIZE];
  int ret;

  fprintf(stdout,"Showing sensor version ...\n");

  sprintf(cmd,"V\n");
  ret=scipSendCmd(aPort,cmd,1);
  if(ret!=0)
    return ret;

  while(fgets(line,sizeof(line),aPort->dev)!=NULL){
    if(strcmp(line,LF_STRING)==0){
      fprintf(stdout,"\n");
      return 0;
    }
    fprintf(stdout,"%s",line);
  }
  return -1;
}

//! Laser control command[L-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @param aState [in] Parameter(0:Laser off,1:Laser on)
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval -3 Invalid parameter(parameters other than 0 and 1)
  @retval other Failure (Response status)
*/
int scip1SwitchLaser(tPort* aPort, int aState)
{
  char cmd[CMD_SIZE];
  int ret;

  if(aState<0 || aState>1)
    return -3;

  sprintf(cmd,"L%01d\n",aState);
  ret=scipSendCmd(aPort,cmd,1);
  if(ret!=0)
    return ret;
  if(scipReceiveResEnd(aPort)!=0)
    return -1;

  switch(aState){
    case 0:
      aPort->state=STATE_IDLE;
      break;
    case 1:
      aPort->state=STATE_MEASURE_STORE;
      break;
    default:
      return -3;
  }
  return 0;
}

//! Change Communication speed [S-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @param aSpeed [in] Setting speed (bps)
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval -3 Failure to set the communication speed
  @retval other  Failure (Response status)
*/
int scip1SetComSpeed(tPort* aPort, int aSpeed)
{
  speed_t speed;
  struct termios tio;
  char cmd[CMD_SIZE];
  int ret;

  fprintf(stdout,"Trying to change bit rate to %d bps ...\n",aSpeed);

  speed=bitrate(aSpeed);
  if(speed==B0){
    fprintf(stderr,"INVALID BITRATE\n");
    return EXIT_FAILURE;  // invalid bitrate
  }

  // Already running on the set speed
  if(getBitrate(fileno(aPort->dev))==speed){
    fprintf(stdout,"The sensor is already connected at %d bps\n",aSpeed);
    return 0;
  }

  sprintf(cmd,"S%06d1234567\n",aSpeed);
  ret=scipSendCmd(aPort,cmd,1);
#ifdef DEBUG
  fprintf(stdout,"ret = %d",ret);
#endif
  // Ad-Hoc,Main Correction
  if(ret==3){  // Status "3", Already running on the set speed
    fprintf(stdout,"The sensor is already worked at %d bps\n",aSpeed);
    fprintf(stdout,"The sensor would be connected on USB\n\n");
    return 0;
  }
  if(ret!=0)
    return ret;
  if(scipReceiveResEnd(aPort)!=0)
    return -1;

  tcgetattr(fileno(aPort->dev),&tio);

  cfsetospeed(&tio,speed);  // bitrate
  cfsetispeed(&tio,speed);  // bitrate

  // Commit
  if(tcsetattr(fileno(aPort->dev),TCSAFLUSH,&tio)!=0){
    perror("Could not set terminal parameters ");
    return -3;
  }

  if(getBitrate(fileno(aPort->dev))==speed){
#if 1
    flushSensorBuf(aPort->dev);
#endif
    return 0;
  } else{
    fprintf(stderr,"Could not set speed : real bitrate = %d [speed_t]\n",getBitrate(fileno(aPort->dev)));
    return -3;
  }
}

//! Obtain mesured data[G-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @param aStartStep [in] scan data start step (0-768)
  @param aEndStep [in] scan data end step (0-768)
  @param aStepCluster [in] cluster count (0-99)
  @param aStepNum [out] Scan data step number (1-769)
  @return Starting address of the scan data array.
  malloc is used for memory pointer. Set it free after use.
  Return NULL when failed to obtin scan data.
*/
// TODO: Check for step data MIN, MAX 
int* scip1GetScan(tPort* aPort, int aStartStep, int aEndStep, int aStepCluster, int *aStepNum)
{
  char cmd[CMD_SIZE];
  char line[LINE_SIZE];
  char *encData,*encDataIter;
  int lineEnd;
  int *scan,*pt;

  if(aPort->state!=STATE_MEASURE_STORE){
    fprintf(stderr,"Can not get a scan (Laser OFF)\n");
    return NULL;
  }

  sprintf(cmd,"G%03d%03d%02d\n",aStartStep,aEndStep,aStepCluster);
  if(scipSendCmd(aPort,cmd,1)!=0)
    return NULL;

  // the number of result data
  aStepCluster = aStepCluster==0 ? 1 : aStepCluster;
  if((aEndStep-aStartStep+1)%aStepCluster==0)
    *aStepNum=(aEndStep-aStartStep+1)/aStepCluster;
  else
    *aStepNum=(aEndStep-aStartStep+1)/aStepCluster+1;

  // malloc for encoded data
  encData=(char*)malloc(sizeof(char)*2*(*aStepNum)+sizeof(char));
  if(encData==NULL){
    free(encData);
    return NULL;
  }

  scan=(int*)malloc(sizeof(int)*(*aStepNum));
  if(scan==NULL){
    free(scan);
    return NULL;
  }

  *encData='\0';
  pt=scan;

  while(1){
    // Read every data from received buffer
    if(fgets(line,sizeof(line),aPort->dev)==NULL){
      skipReceiveBuf(aPort->dev);
      free(scan);
      free(encData);
      return NULL;
    }
    // Judge the end of the received data 
    if(strcmp(line,LF_STRING)==0){
      if(strlen(encData)==0){  // error
        skipReceiveBuf(aPort->dev);
        free(scan);
        free(encData);
        return NULL;
      } else  // end of one scan
        break;
    }
#ifdef DEBUG
    fprintf(stdout,"%s",line);
#endif
    // Neglect LF
    lineEnd=strlen(line)-1;
    line[lineEnd]='\0';
    // Combine encode data into single line
    // TODO: Implementing memccpy() 
    strncat(encData,line,64);
  }
#ifdef DEBUG
  fprintf(stdout,"Encoded data string = \n%s\n",encData);
#endif
  // Decode
  encDataIter=encData;
  while(strlen(encDataIter)-2>0){
    *pt=scipDecode(encDataIter,2);
    encDataIter+=2;
    ++pt;
    if(pt>=scan+(*aStepNum)){  // If memory exceed allocated space -malloc 
      free(scan);
      free(encData);
      return NULL;
    }
  }
  free(encData);
  return scan;
}

//! Free the allocated(malloc) scan data array 
/*!
  @param aScan [in] aScan Starting address of the scan data array to be deleted
  @retval EXIT_SUCCESS Success
  @retval EXIT_FAILURE Fail
 */
int scip1FreeScan(int *aScan)
{
  if(!aScan)
    return EXIT_FAILURE;
  free(aScan);
  return EXIT_SUCCESS;
}
