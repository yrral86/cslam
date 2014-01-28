/*!
  @file
  @brief  Sample program to obtain and save 2×5sets of scan data
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
#include "scipUtil.h"

int main(int argc, char **argv)
{
  tPort *port;
  FILE *output;
  int **scan2;
  int i,j,id;
  int startStep,endStep,stepCluster,scanInterval,scanNum,stepNum;
  double x,y;

  // Display usage if arguments are invalid 
  if(argc != 3){
    printf("USAGE:\n");
    printf(" %s dev_port_name output_file_name\n",argv[0]);
    printf(" ex) %s /dev/ttyS0 logfile.dat\n",argv[0]);
    exit(EXIT_SUCCESS);
  }

  // Connect with URG 
  port=scipConnect(argv[1]);
  if(port==NULL){
    perror("Could not connect to the sensor ");
    exit(EXIT_FAILURE);
  }

#if 0
  // Change Communication speed [S-Command]
  if(scip1SetComSpeed(port,115200) != 0){
    fprintf(stderr,"Could not change speed\n");
    exit(EXIT_FAILURE);
  }
#endif

  // Swith to SCIP2.0 mode
  switchToScip2(port);

  // Change communication speed [SS-Command]
  if(scip2SetComSpeed(port,115200)!=0){
    fprintf(stderr,"Could not change speed\n");
    exit(EXIT_FAILURE);
  }

  // Open output file
  output=fopen(argv[2],"w");
  if(output==NULL){
    perror("Could not open output file");
    exit(EXIT_FAILURE);
  }

  // Send data acquisition command [MD-Command] 5 times
  for(i=0; i<5; ++i){
    printf("Trying #%d\n",i);

    // Distance data parameter settings
    // Get all data (from step 44 to 725) 2 times without interval in each transmission
    startStep=44;
    endStep=725;
    stepCluster=1;
    scanInterval=0;
    scanNum=2;

    // Obtain the data measured after sending the command [MD-Command]
    scan2=scip2MeasureScan(port,startStep,endStep,stepCluster,scanInterval,scanNum,ENC_3BYTE,&stepNum);

    if(scan2!=NULL){  // Save to file upon successful completion
      for(j=0; j<scanNum; ++j){
        for(id=0; id<stepNum; ++id){
          // Check error codes, convert data into x-y coordinates
          // Please note that the step number and array number are not the same. (First data in array is the data from step 44)
          if(scan2[j][id] >= 20 && scan2[j][id] <= 5600){  // Normal Data
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

  // Close communication Handler
  closePort(port);

  fclose(output);

  return EXIT_SUCCESS;
}
