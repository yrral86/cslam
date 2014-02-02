/*!
  @file
  @brief SCIP2 API
  @author aokaze (HARA Yoshitaka) <bluewind@roboken.esys.tsukuba.ac.jp>
  @author YoS (YOSHIDA Tomoaki) <yoshida@furo.org>
  @par    Publisher:
          HOKUYO AUTOMATIC CO.,LTD.
  @since  2006
  @date   2007/03/30
*/
/*
  TODO:
  .Error detection using checksum
  .Obtain timestamp and sychronize with host
  .Using thread to obtain data from [MD-MS Command] 
  .Change all types of Show command to paramater obtaining/storing command
*/

#include "scip2.h"

//! Swith to SCIP2.0 mode
/*!
  Use special command to switch the sensor to SCIP 2.0 mode
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int switchToScip2(tPort* aPort)
{
  char cmd[CMD_SIZE];
  int ret;

  fprintf(stdout,"Trying to switch to SCIP2.0 mode ...\n");

  if(scipProtCheck(aPort,2)){
    fprintf(stdout,"The sensor is already in SCIP2 mode\n\n");
    return 0;
  }

  sprintf(cmd,"SCIP2.0\n");
  ret=scipSendCmd(aPort,cmd,1);
  if(ret!=0)
    return ret;
  if(scipReceiveResEnd(aPort)!=0)
    return -1;

  fprintf(stdout,"Switched to SCIP2.0 mode\n\n");

  aPort->protocol=2;
  aPort->state=STATE_IDLE;

  return 0;
}

//!  Display sensor version [VV-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/

int scip2ShowVersion(tPort* aPort)
{
  char cmd[CMD_SIZE];
  char line[LINE_SIZE];
  int ret;

  fprintf(stdout,"Showing sensor version ...\n");

  sprintf(cmd,"VV\n");
  ret=scipSendCmd(aPort,cmd,2);
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

//! Display sensor parameters[PP-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int scip2ShowParameter(tPort* aPort)
{
  char cmd[CMD_SIZE];
  char line[LINE_SIZE];
  int ret;

  fprintf(stdout,"Showing sensor parameters ...\n");

  sprintf(cmd,"PP\n");
  ret=scipSendCmd(aPort,cmd,2);
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

//! Dispalay sensor status [II-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int scip2ShowStatus(tPort* aPort)
{
  char cmd[CMD_SIZE];
  char line[LINE_SIZE];
  int ret;

  fprintf(stdout,"Showing sensor status ...\n");

  sprintf(cmd,"II\n");
  ret=scipSendCmd(aPort,cmd,2);
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

//! Display sensor and host time[TM-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int scip2ShowTime(tPort* aPort)
{
  char cmd[CMD_SIZE];
  char line[LINE_SIZE];
  int lineEnd;
  int ret;
  int sensorTime;  // [ms]
  struct timeval hostTime;
  State_t preState;

  fprintf(stdout,"Showing host time and sensor time ...\n");

  sprintf(cmd,"TM0\n");
  ret=scipSendCmd(aPort,cmd,2);
  if(ret!=0)
    return ret;
  if(scipReceiveResEnd(aPort)!=0)
    return -1;

  preState=aPort->state;
  aPort->state=STATE_TIME_ADJUST;

  gettimeofday(&hostTime,NULL);

  sprintf(cmd,"TM1\n");
  ret=scipSendCmd(aPort,cmd,2);
  if(ret!=0)
    return ret;

  // time-stamp
  if(fgets(line,sizeof(line),aPort->dev)==NULL){
    fprintf(stderr,"NO TIME-STAMP\n");
    return -1;
  }
  if(scipReceiveResEnd(aPort)!=0)
    return -1;
  // Neglect SUM + LF 
  lineEnd=strlen(line)-2;
  if(lineEnd!=4){
    fprintf(stderr,"INVALID TIME-STAMP\n");
    return -1;
  }
  line[lineEnd]='\0';
  sensorTime=scipDecode(line,ENC_4BYTE);

  fprintf(stdout,"host time: %d.%06d [s] since the Epoch (00:00:00 UTC, Jan. 1, 1970)\nsensor time: %d [ms] since boot\n\n",(int)hostTime.tv_sec, (int)hostTime.tv_usec, sensorTime);

  sprintf(cmd,"TM2\n");
  ret=scipSendCmd(aPort,cmd,2);
  if(ret!=0)
    return ret;
  if(scipReceiveResEnd(aPort)!=0)
    return -1;

  aPort->state=preState;
  return 0;
}

//! Change Communication speed [SS-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @param aSpeed [in] Setting speed (bps)
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval -3 Failure to set the communication speed
  @retval other  Failure (Response status)
*/
int scip2SetComSpeed(tPort* aPort, int aSpeed)
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

  sprintf(cmd,"SS%06d;1234567890abcdef\n",aSpeed);
  ret=scipSendCmd(aPort,cmd,2);
#ifdef DEBUG
  fprintf(stdout,"ret = %d",ret);
#endif
  // Ad-Hoc、Main Correction
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

//! Change scan speed [CR-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @param aRate [in] Decrease in percentage of 600 rpm (0 - 10). Setting this parameter to 99 will set the device to initial speed.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval -3 Invalid parameter(parameters other than 0-10,99)
  @retval other Failure (Response status)
*/
int scip2SetScanSpeed(tPort* aPort, int aRate)
{
  char cmd[CMD_SIZE];
  int ret;

  if(aRate<0 || (aRate>10 && aRate!=99))
    return -3;

  sprintf(cmd,"CR%02d\n",aRate);
  ret=scipSendCmd(aPort,cmd,2);
  if(ret!=0)
    return ret;
  if(scipReceiveResEnd(aPort)!=0)
    return -1;

  return 0;
}

//! Scan data obtaind after sending the command [MD-MS Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @param aStartStep [in] scan data start step (0-768)
  @param aEndStep [in] scan data end step (0-768)
  @param aStepCluster [in] cluster count (0-99)
  @param aScanInterval [in] interval between the scan  (0-9)
  @param aScanNum [in] number of scans by sending a single command (0-99)
  @param aEncType [in] data encoding type (ENC_3BYTE, ENC_2BYTE)
  @param aStepNum [out] step number of scan data (1-769)
  @return Starting address of the scan data array
  malloc is used for memory pointer. Set it free after use.
  Return NULL when failed to obtin scan data.
*/
// TODO: Check for step data MIN, MAX 
// TODO: Use of thread
int** scip2MeasureScan(tPort* aPort, int aStartStep, int aEndStep, int aStepCluster, int aScanInterval, int aScanNum, EncType_t aEncType, int *aStepNum)
{
  static const char SCIP2_MEASURE_STATUS[]={'9','9','b',LF,'\0'};
  char cmdCode[2];
  char cmd[CMD_SIZE];
  char line[LINE_SIZE];
  char *encData,*encDataIter;
  int i,j,k;
  int lineEnd;
  int **scan,*pt;
  State_t preState;

  if(aEncType==ENC_2BYTE)
    sprintf(cmdCode,"MS");
  else{  // Default is 3-Character encoding [MD-Command]
    sprintf(cmdCode,"MD");
    aEncType=ENC_3BYTE;
  }

  sprintf(cmd,"%s%04d%04d%02d%01d%02d\n",cmdCode,aStartStep,aEndStep,aStepCluster,aScanInterval,aScanNum);
  if(scipSendCmd(aPort,cmd,2)!=0)
    return NULL;
  if(scipReceiveResEnd(aPort)!=0)
    return NULL;

  preState=aPort->state;
  aPort->state=STATE_MEASURE_SEND;

  // the number of result data
  aStepCluster = aStepCluster==0 ? 1 : aStepCluster;
  if((aEndStep-aStartStep+1)%aStepCluster==0)
    *aStepNum=(aEndStep-aStartStep+1)/aStepCluster;
  else
    *aStepNum=(aEndStep-aStartStep+1)/aStepCluster+1;

  // malloc for encoded data
  encData=(char*)malloc(sizeof(char)*aEncType*(*aStepNum)+sizeof(char));
  if(encData==NULL){
    free(encData);
    return NULL;
  }

  // TODO: Subroutine
  scan=(int**)malloc(sizeof(int*)*aScanNum);
  if(scan==NULL){
    free(scan);
    free(encData);
    return NULL;
  }
  for(i=0;i<aScanNum;i++){
    scan[i]=(int*)malloc(sizeof(int)*(*aStepNum));
    if(scan[i]==NULL){
      for(j=0;j<=i;++j)
        free(scan[j]);
      free(scan);
      free(encData);
      return NULL;
    }
  }

  // Receive data
  for(i=0;i<aScanNum;++i){
    *encData='\0';
    pt=scan[i];

    // echo-back
    if(fgets(line,sizeof(line),aPort->dev)==NULL){
      fprintf(stderr,"NO ECHO BACK\n");
      for(j=0;j<aScanNum;++j)
        free(scan[j]);
      free(scan);
      free(encData);
      return NULL;
    }
    if(strncmp(cmd,line,13)!=0){
      // invalid
      fprintf(stderr,"INVALID ECHO BACK\n\tExp : %s\tRec : %s\n",cmd,line);
      skipReceiveBuf(aPort->dev);
      for(j=0;j<aScanNum;++j)
        free(scan[j]);
      free(scan);
      free(encData);
      return NULL;
    }

    // status
    if(fgets(line,sizeof(line),aPort->dev)==NULL){
      fprintf(stderr,"NO STATUS\n");
      for(j=0;j<aScanNum;++j)
        free(scan[j]);
      free(scan);
      free(encData);
      return NULL;
    }
    if(strcmp(line,SCIP2_MEASURE_STATUS)!=0){
      //invalid
      fprintf(stderr,"STATUS CHECK FAILED : %s",line);
      skipReceiveBuf(aPort->dev);
      for(j=0;j<aScanNum;++j)
        free(scan[j]);
      free(scan);
      free(encData);
      return NULL;
    }

    // time-stamp
    if(fgets(line,sizeof(line),aPort->dev)==NULL){
      fprintf(stderr,"NO TIME-STAMP\n");
      for(j=0;j<aScanNum;++j)
        free(scan[j]);
      free(scan);
      free(encData);
      return NULL;
    }

    // main data
    while(1){
      // Read every data from received buffer
      if(fgets(line,sizeof(line),aPort->dev)==NULL){
        skipReceiveBuf(aPort->dev);
        for(j=0;j<aScanNum;++j)
          free(scan[j]);
        free(scan);
        free(encData);
        return NULL;
      }
      // Judge the end of the received data 
      if(strcmp(line,LF_STRING)==0){
        if(strlen(encData)==0){  // error
          skipReceiveBuf(aPort->dev);
          for(j=0;j<aScanNum;++j)
            free(scan[j]);
          free(scan);
          free(encData);
          return NULL;
        } else  // end of one scan
          break;
      }
#ifdef DEBUG
      fprintf(stdout,"%s",line);
#endif
      // Neglect SUM + LF 
      lineEnd=strlen(line)-2;
      line[lineEnd]='\0';
      // Combine encode data into single line
      // TODO: Implementing memccpy() 
      strncat(encData,line,64);
    }
#ifdef DEBUG
    fprintf(stdout,"Encoded data string = \n%s\n",encData);
#endif
    // decode
    encDataIter=encData;
    while(strlen(encDataIter)-aEncType>0){
      *pt=scipDecode(encDataIter,aEncType);
      encDataIter+=aEncType;
      ++pt;
      if(pt>=scan[i]+(*aStepNum)){  //If memory exceed allocated space -malloc 
        for(k=0;k<aScanNum;++k)
          free(scan[k]);
        free(scan);
        free(encData);
        return NULL;
      }
    }
  }
  aPort->state=preState;
  free(encData);
  return scan;
}

//!  Measure and store the data(Laser on)[BM-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int scip2StartStoringScan(tPort* aPort)
{
  char cmd[CMD_SIZE];
  int ret;

  sprintf(cmd,"BM\n");
  ret=scipSendCmd(aPort,cmd,2);
  if(ret!=0)
    return ret;
  if(scipReceiveResEnd(aPort)!=0)
    return -1;

  aPort->state=STATE_MEASURE_STORE;

  return 0;
}

//! Stop measurement[BM-Command](Laser off)and quit scan [QT-Command]
/*!
  現状ではMD/MSコマンドに対応する関数(scip2MeasureScan)は Blocking 実装のため、
  本関数(scip2StopStoringScan)はBMコマンド(scip2StartStoringScan)にのみ対応。
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int scip2StopStoringScan(tPort* aPort)
{
  char cmd[CMD_SIZE];
  int ret;

  sprintf(cmd,"QT\n");
  ret=scipSendCmd(aPort,cmd,2);
  if(ret!=0)
    return ret;
  if(scipReceiveResEnd(aPort)!=0)
    return -1;

  aPort->state=STATE_IDLE;

  return 0;
}

//! Obtain the sensor data[GD-GS Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @param aStartStep [in] scan data start step (0-768)
  @param aEndStep [in] scan data end step (0-768)
  @param aStepCluster [in] cluster count (0-99)
  @param aEncType [in] data encoding type (ENC_3BYTE, ENC_2BYTE)
  @param aStepNum [out] step number of scan data (1-769)
  @return Starting address of the scan data array.
  malloc is used for memory pointer. Set it free after use.
  Return NULL when failed to obtin scan data.
*/
// TODO: Check for step data MIN, MAX
int* scip2GetScan(tPort* aPort, int aStartStep, int aEndStep, int aStepCluster, EncType_t aEncType, int *aStepNum)
{
  char cmdCode[2];
  char cmd[CMD_SIZE];
  char line[LINE_SIZE];
  char *encData,*encDataIter;
  int lineEnd;
  int *scan,*pt;

  if(aPort->state!=STATE_MEASURE_STORE){
    fprintf(stderr,"Can not get a scan (Laser OFF, No stored scan)\n");
    return NULL;
  }

  if(aEncType==ENC_2BYTE)
    sprintf(cmdCode,"GS");
  else{  // Default is 3-Character encoding [GD-Command]
    sprintf(cmdCode,"GD");
    aEncType=ENC_3BYTE;
  }

  sprintf(cmd,"%s%04d%04d%02d\n",cmdCode,aStartStep,aEndStep,aStepCluster);
  if(scipSendCmd(aPort,cmd,2)!=0)
    return NULL;

  // time-stamp
  if(fgets(line,sizeof(line),aPort->dev)==NULL){
    fprintf(stderr,"NO TIME-STAMP\n");
    free(scan);
    free(encData);
    return NULL;
  }
  // TODO: Decode time stamp

  // the number of result data
  aStepCluster = aStepCluster==0 ? 1 : aStepCluster;
  if((aEndStep-aStartStep+1)%aStepCluster==0)
    *aStepNum=(aEndStep-aStartStep+1)/aStepCluster;
  else
    *aStepNum=(aEndStep-aStartStep+1)/aStepCluster+1;

  // malloc for encoded data
  encData=(char*)malloc(sizeof(char)*aEncType*(*aStepNum)+sizeof(char));
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
    // Neglect SUM + LF
    lineEnd=strlen(line)-2;
    line[lineEnd]='\0';
    // Combine encode data into single line
    // TODO: Implementing memccpy()
    strncat(encData,line,64);
  }
#ifdef DEBUG
  fprintf(stdout,"Encoded data string = \n%s\n",encData);
#endif
  // デコード
  encDataIter=encData;
  while(strlen(encDataIter)-aEncType>0){
    *pt=scipDecode(encDataIter,aEncType);
    encDataIter+=aEncType;
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
  param aScanNum [in] Distance data array number (scan count) (0-99)
  @retval EXIT_SUCCESS Success
  @retval EXIT_FAILURE Fail
 */
int scip2FreeScan(int **aScan, int aScanNum)
{
  int i;

  if(!aScan)
    return EXIT_FAILURE;
  for(i=0;i<aScanNum;++i)
    if(aScan[i])
      free(aScan[i]);
  free(aScan);
  return EXIT_SUCCESS;
}

//! Reset sensor(Reset all parameters)[RS-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval -3 Failure to set the communication speed
  @retval other  Failure (Response status)
*/
int scip2SensorReset(tPort* aPort)
{
  speed_t speed;
  struct termios tio;
  char cmd[CMD_SIZE];
  int ret;

  sprintf(cmd,"RS\n");
  ret=scipSendCmd(aPort,cmd,2);
  if(ret!=0)
    return ret;
  if(scipReceiveResEnd(aPort)!=0)
    return -1;

  aPort->state=STATE_IDLE;

  speed=bitrate(19200);
  tcgetattr(fileno(aPort->dev),&tio);

  cfsetospeed(&tio,speed);  // bitrate
  cfsetispeed(&tio,speed);  // bitrate

  // Commit
  if(tcsetattr(fileno(aPort->dev),TCSAFLUSH,&tio)==0){
#if 1
    flushSensorBuf(aPort->dev);
#endif
    return 0;
  } else {  // fail
    perror("Could not set terminal parameters ");
    return -3;
  }
}
