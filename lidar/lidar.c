/*!
  @file		lidar.c
  @brief	Takes data from the lidar sensor and puts it in shared memory and plugs it into CoreSLAM.
  @author	Brad Nelson			<nelsobra@onid.orst.edu>
  @			Based on source provided by Hokuyo Automatic, Inc. JP
*/

#include <semaphore.h>
#include "scipBase.h"
#include "scip1.h"
#include "scip2.h"
#include "scipUtil.h"

#include "timing.h"
#include "share.h"
#include "sharedLidar.h"

#define MIN_RANGE 20
#define MAX_RANGE 5600

unsigned char *lidShmPtr;					/// This points to the lidar shared memory allocation
struct lidarScan *lidar_data;				// [10]
struct lidarMinScan *lidar_min_data;		// [10]
int lidar_data_head;
unsigned int *lidar_data_cnt;
int stepNum;



void print_test();
void setupSharedMem();



int main(int argc, char **argv)
{
  // Connect the variables up with shared memory.
  setupSharedMem();
  tPort *port;
  FILE *output;
  int **scan2;
  int j;
  int startStep,endStep,stepCluster,scanInterval,scanNum;
//  double x,y;
//  int i,id;

  // Connect with URG 
  port=scipConnect("/dev/ttyACM0");
  if(port==NULL){
    perror("Could not connect to the sensor ");
    exit(EXIT_FAILURE);
  }

  // Switch to SCIP2.0 mode
  switchToScip2(port);

  // Change communication speed [SS-Command]
  if(scip2SetComSpeed(port,115200)!=0){
    fprintf(stderr,"Could not change speed\n");
    exit(EXIT_FAILURE);
  }

  // Initialize parameters for laser scanning
  startStep=44;
  endStep=725;
  stepCluster=1;
  scanInterval=0;
  scanNum=1;
  lidar_data_head = 0;
  // We're all set up.  Grab LiDAR data and crank it up into shared memory.
  // If CoreSLAM is enabled, pipe data to it as well.
  while (1)
  {
    // Grab a sensor reading
    scan2=scip2MeasureScan(port,startStep,endStep,stepCluster,scanInterval,scanNum,ENC_3BYTE,&stepNum);
	for (j = 0 ; j < stepNum ; ++j) {
	  if ((scan2[0][j] >= MIN_RANGE) && (scan2[0][j] <= MAX_RANGE)) {
		// Copy the data into the temp data set
		lidar_data[lidar_data_head].data[j] = scan2[0][j];
	  }
	}

	// Create the min_data set
	lidar_min_data[lidar_data_head].data[0] = scan2[0][679];	//Altitude
	lidar_min_data[lidar_data_head].data[1] = scan2[0][667];	//-115 degrees
	lidar_min_data[lidar_data_head].data[2] = scan2[0][596];	//-90 degrees
	lidar_min_data[lidar_data_head].data[3] = scan2[0][511];	//-60 degrees
	lidar_min_data[lidar_data_head].data[4] = scan2[0][469];	//-45 degrees
	lidar_min_data[lidar_data_head].data[5] = scan2[0][426];	//-30 degrees
	lidar_min_data[lidar_data_head].data[6] = scan2[0][341];	//0 degrees
	lidar_min_data[lidar_data_head].data[7] = scan2[0][256];	//30 degrees
	lidar_min_data[lidar_data_head].data[8] = scan2[0][213];	//45 degrees
	lidar_min_data[lidar_data_head].data[9] = scan2[0][170];	//60 degrees
	lidar_min_data[lidar_data_head].data[10] = scan2[0][85];	//90 degrees
	lidar_min_data[lidar_data_head].data[11] = scan2[0][15];	//115 degrees
	lidar_min_data[lidar_data_head].data[12] = scan2[0][3];		//-180 degrees

	print_test();

	lidar_data_cnt++;
	if (lidar_data_head >= 9)
	  lidar_data_head = 0;
	else
	  lidar_data_head++;

	usleep(100000);
  }

/*
  // Send data acquisition command [MD-Command] 5 times
  for(i=0; i<5; ++i){
    printf("Trying #%d\n",i);

    // Distance data parameter settings
    // Get all data (from step 44 to 725) 2 times without interval in each transmission
    startStep=44;
    endStep=725;
    stepCluster=1;
    scanInterval=0;
    scanNum=1;

    // Obtain the data measured after sending the command [MD-Command]

    if(scan2!=NULL){  // Save to file upon successful completion
      for(j=0; j<scanNum; ++j){
        for (id=0; id<stepNum; ++id){
          // Check error codes, convert data into x-y coordinates
          // Please note that the step number and array number are not the same. (First data in array is the data from step 44)
          if(scan2[j][id] >= MIN_RANGE && scan2[j][id] <= MAX_RANGE){  // Normal Data
            x=scan2[j][id] * cos(step2rad(id2step(startStep,stepCluster,id)));
            y=scan2[j][id] * sin(step2rad(id2step(startStep,stepCluster,id)));
          } else  // Error Code
            continue;

          // Save step number, measurement data and x-y coordinate into file
          fprintf(output,"%d %d %f %f\n",id2step(startStep,stepCluster,id),scan2[j][id],x,y);
        }
        fprintf(output,"\n");
      }
//       fprintf(output,"\n");

      // Free the allocated(malloc) scan data array
      scip2FreeScan(scan2,scanNum);
    } else  // Obtaining the measurement data is failed
      fprintf(stderr,"#%d failed\n",i);
  }
*/
  // Close communication Handler
  closePort(port);

  fclose(output);

  return EXIT_SUCCESS;
}

void print_test()
{
  printf("%3d  -120  -115   -90   -60   -45   -30     0    30    45    60    90   115   120 %5d%4d\n", lidar_data_head, lidar_data_cnt, stepNum);
  /*printf("   %6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d\n\n", 
		  lidar_min_data[lidar_data_head].data[0],
		  lidar_min_data[lidar_data_head].data[1], 
		  lidar_min_data[lidar_data_head].data[2], 
		  lidar_min_data[lidar_data_head].data[3],
		  lidar_min_data[lidar_data_head].data[4], 
		  lidar_min_data[lidar_data_head].data[5], 
		  lidar_min_data[lidar_data_head].data[6], 
		  lidar_min_data[lidar_data_head].data[7], 
		  lidar_min_data[lidar_data_head].data[8], 
		  lidar_min_data[lidar_data_head].data[9],
		  lidar_min_data[lidar_data_head].data[10], 
		  lidar_min_data[lidar_data_head].data[11], 
		  lidar_min_data[lidar_data_head].data[12]); */
  printf("   %6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d\n\n", 
		  lidar_data[lidar_data_head].data[681],
		  lidar_data[lidar_data_head].data[679], 
		  lidar_data[lidar_data_head].data[677], 
		  lidar_data[lidar_data_head].data[675],
		  lidar_data[lidar_data_head].data[673], 
		  lidar_data[lidar_data_head].data[671], 
		  lidar_data[lidar_data_head].data[669], 
		  lidar_data[lidar_data_head].data[667], 
		  lidar_data[lidar_data_head].data[665], 
		  lidar_data[lidar_data_head].data[663],
		  lidar_data[lidar_data_head].data[661], 
		  lidar_data[lidar_data_head].data[659], 
		  lidar_data[lidar_data_head].data[657]);
}

void setupSharedMem()
{
	struct Settings *s = initSettings();								// Get the variables set and pointed to shared memory locations
	getSettings(SETTINGSPATH, s);
	lidShmPtr = attachSharedMem(s, LIDAR_SHM);

	lidar_data			= (lidShmPtr + LIDAR_OFFSET_DATA);
	lidar_min_data		= lidShmPtr + LIDAR_OFFSET_MIN_DATA;
	lidar_data_cnt		= lidShmPtr + LIDAR_OFFSET_DATA_CNT;

	*lidar_data_cnt = 0;
}

