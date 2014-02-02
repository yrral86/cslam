#ifndef SCIP2_H
#define SCIP2_H
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

#include <sys/time.h>
#include "scipBase.h"

//! Swith to SCIP2.0 mode
/*!
  Use special command to switch the sensor to SCIP 2.0 mode
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int switchToScip2(tPort* aPort);

//!  Display sensor version [VV-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int scip2ShowVersion(tPort* aPort);

//! Display sensor parameters[PP-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int scip2ShowParameter(tPort* aPort);

//! Dispalay sensor status [II-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int scip2ShowStatus(tPort* aPort);

//! Display sensor and host time[TM-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int scip2ShowTime(tPort* aPort);

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
int scip2SetComSpeed(tPort* aPort, int aSpeed);

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
int scip2SetScanSpeed(tPort* aPort, int aRate);

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
int** scip2MeasureScan(tPort* aPort, int aStartStep, int aEndStep, int aStepCluster, int aScanInterval, int aScanNum, EncType_t aEncType, int *aStepNum);

//!  Measure and store the data(Laser on)[BM-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int scip2StartStoringScan(tPort* aPort);

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
int scip2StopStoringScan(tPort* aPort);

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
int* scip2GetScan(tPort* aPort, int aStartStep, int aEndStep, int aStepCluster, EncType_t aEncType, int *aStepNum);

//! Free the allocated(malloc) scan data array 
/*!
  @param aScan [in] aScan Starting address of the scan data array to be deleted
  param aScanNum [in] Distance data array number (scan count) (0-99)
  @retval EXIT_SUCCESS Success
  @retval EXIT_FAILURE Fail
 */
int scip2FreeScan(int **aScan, int aScanNum);

//! Reset sensor(Reset all parameters)[RS-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval -3 Failure to set the communication speed
  @retval other  Failure (Response status)
*/
int scip2SensorReset(tPort* aPort);

#endif  // SCIP2_H
