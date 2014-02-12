#ifndef SCIP1_H
#define SCIP1_H
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

#include "scipBase.h"

//! Display sensor version [V-Command]
/*!
  @param aPort [in] Sensor's communication handler.
  @retval 0 Success
  @retval -1 Failure in command transmission or reception
  @retval -2 Protocol mismatch (Undefined commands)
  @retval other Failure (Response status)
*/
int scip1ShowVersion(tPort* aPort);

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
int scip1SwitchLaser(tPort* aPort, int aState);

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
int scip1SetComSpeed(tPort* aPort, int aSpeed);

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
int* scip1GetScan(tPort* aPort, int aStartStep, int aEndStep, int aStepCluster, int *aStepNum);

//! Free the allocated(malloc) scan data array 
/*!
  @param aScan [in] aScan Starting address of the scan data array to be deleted
  @retval EXIT_SUCCESS Success
  @retval EXIT_FAILURE Fail
 */
int scip1FreeScan(int *aScan);

#endif  // SCIP1_H
