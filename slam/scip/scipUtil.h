#ifndef SCIP_UTIL_H
#define SCIP_UTIL_H
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

#include "scipBase.h"
#include "scip1.h"
#include "scip2.h"

#define ANG_UNIT_RAD (2.0*M_PI / ANG_RESOLUTION)

//! Try to connect with SCIP1 and SCIP2 (Communication speed will not change)
/*!
  @param aPort [in] Sesnor's communication handler
  @retval 1 Connection successful with SCIP1 
  @retval 2 Connection successful with SCIP2
  @retval -1 Connection failed with SCIP1 and SCIP2
*/
int scipTryComm(tPort* aPort);

//! Connect to designated serial port with SCIP commands
/*!
  @param aDevName [in] Serial port device file name
  @return Sesnor's communication handler
  Returns NULL if failed
*/
tPort* scipConnect(char *aDevName);

//! Adjust the scan data array number with starting step of scan
/*!
  @param aStartStep [in] scan data start step
  @param aStepCluster [in] cluster count (1-99)
  @param aId [in] Array number
  @return step number
*/
int id2step(int aStartStep, int aStepCluster, int aId);

//! Convert step number to angle (radian)
/*!
  @param aStep [in] step number
  @return radian angle
*/
double step2rad(int aStep);

#endif  // SCIP_UTIL_H
