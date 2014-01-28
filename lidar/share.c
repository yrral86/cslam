/*
      Filename:      share.c
      Authors:       Paul Filitchkin
      Date Created:  03 April 2009
      Description:   Functions for reading shared settings file

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <semaphore.h>

#include "share.h"

int fileExists(const char *filename) {

    FILE *file = fopen(filename, "r");

    if(file) {
        fclose(file);
        return 1;
    }
    return 0;
}

struct Settings *initSettings(void) {
    //Allocate memory for Settings struct
    struct Settings *s = (struct Settings *)malloc(sizeof(struct Settings));

    //Allocate memory for arrays in Settings struct
	s->procPaths    = (char *)malloc(MAXBUF);
	s->procPathsIdx = (unsigned char *)malloc(MAXBUF);
	s->procNames    = (char *)malloc(MAXBUF);
	s->procNamesIdx = (unsigned char *)malloc(MAXBUF);
	s->procMemSize  = (int *)malloc(MAX_PROCS);

    //Initialize values to zero
	s->numProcs = 0;
    bzero(s->procPaths, MAXBUF);
    bzero(s->procPathsIdx, MAXBUF);
    bzero(s->procNames, MAXBUF);
    bzero(s->procNamesIdx, MAXBUF);
    bzero(s->procMemSize, MAX_PROCS);

    return s;
}

void getSettings(char *settingsPath, struct Settings *s) {

    FILE *settingsFd;

    char name[MAX_NAME_LEN];
    char val[MAX_VAL_LEN];

    int ver;
    int valsRead;
    int numProcs = 0;
    int pOffset  = 0;
    int nOffset  = 0;
    int memSize  = 0;

    //Open settings file
    settingsFd = fopen(settingsPath, "r");

    if (settingsFd == (FILE *)0) {
        printf("Could not open %s\n", settingsPath);
        exit(1);
    }
    
    //Read version (must be first line)
    valsRead = fscanf(settingsFd, "%s %d", name, &ver);

    //Debug
    //DEBUG printf("Settings version %d\n", ver);

    //Verify that the settings file is the correct version
    if((strcmp(name, "VERSION") != 0) || (ver != VERSION)) {
        printf("Invalid settings\n"); 
        printf("Expected: VERSION %d\n", VERSION);
        exit(1);
    }

    //Grab rest of parameters from settings file (order does not matter)
    while(1) {
        valsRead = fscanf(settingsFd, "%s %s", name, val);

        if (valsRead == 2) {
            //Debug
            //DEBUG printf("Values Read: %d\nValues: %s , %s\n", valsRead, name, val);

            //If name starts with PROC
            if( strcmp("PROC_PATH", name) == 0 ) {

                //There is a new process path to add
                s->procPathsIdx[numProcs] = pOffset;

                //Add process path (val) to collection of process names
                strcpy(s->procPaths + pOffset, val);

                //+1 because strlen does not count terminating byte
                pOffset = pOffset + strlen(val) + 1;
                
                if(pOffset > MAXBUF){ printf("procPaths buffer exceeded\n"); exit(1); }

            //Memory size
            } else if( strcmp("PROC_NAME", name) == 0 ) {

                //There is a new process name to add
                s->procNamesIdx[numProcs] = nOffset;

                //Add process path (val) to collection of process names
                strcpy(s->procNames + nOffset, val);

                //+1 because strlen does not count terminating byte
                nOffset = nOffset + strlen(val) + 1;
                
                if(nOffset > MAXBUF){ printf("procNames buffer exceeded\n"); exit(1); }

            } else if( strcmp("MEM_SIZE", name) == 0 ) {

                sscanf(val, "%d", &memSize);
                s->procMemSize[numProcs] = memSize;

 
                //DEBUG printf("\nProc name: %s\n", s->procNames + s->procNamesIdx[numProcs]);
                //DEBUG printf("Proc path: %s\n", s->procPaths + s->procPathsIdx[numProcs]);
                //DEBUG printf("Mem size: %d\n\n", (s->procMemSize[numProcs]));
                numProcs++;

            } 

        } else {
            s->numProcs = numProcs;
            break;
        }
    }
}

/* Make the key: 
   The  ftok() function returns a key based on path and id
   that is usable in subsequent calls to shmget() */
key_t initKey(char *s) {

    key_t key;

    if ((key = ftok(s, UNIQUE_ID)) == -1) {
        printf("ERROR: Could not acquire key\n");
        exit(1);
    }

    return key;
}

//Create/connect to shared memory segment and return ID
int initSharedMem(key_t key, int size) {

    int shmID;

    if ((shmID = shmget(key, size, (0644 | IPC_CREAT))) == -1) {
        printf("ERROR: Could not create/attach to shared memory segment\n");
        exit(1);
    }

    return shmID;
}

unsigned char *sharedMemPtr(int shmID) {

    unsigned char *shmPtr = shmat(shmID, (void *)0, 0);

    if (shmPtr == (unsigned char *)(-1)) {
        printf("ERROR: Could not get pointer to shared memory\n");
        exit(1);
    }

    return shmPtr;

}

//Attach to a shared memory segment
unsigned char *attachSharedMem(struct Settings *s, char *attachName) {

    int i = 0;
    char name[MAXBUF];
    char path[MAXBUF];
    key_t key;
    int shmID;

    //Figure out which entry attachName refers to
    for (i = 0; i < (s->numProcs); i++) {

        strcpy(path, s->procPaths + s->procPathsIdx[i]); //Get process path
        strcpy(name, s->procNames + s->procNamesIdx[i]); //Get process name

        if (strcmp(name, attachName) == 0) break;
    }
    
    if (i == s->numProcs) { printf("ERROR: no matching name in settings"); exit(1); }

    key = initKey(path); //Get unique key
    shmID = initSharedMem(key, s->procMemSize[i]); //Get shared memory ID

    return sharedMemPtr(shmID); //Return pointer to shared memory
}

//Dettach from a shared memory segment
void dettachSharedMem(unsigned char *ptr) {
    /* Detach from but do not destroy the segment: */
    if (shmdt(ptr) == -1) {
        printf("Could not detach segment\n");
        exit(1);
    }
}

//Get the size of shared memory
int getSharedMemSize(struct Settings *s, char *nameKey) {

    int i = 0;
    char name[MAXBUF];

    //Figure out which entry name refers to
    for (i = 0; i < (s->numProcs); i++) {
        
        strcpy(name, s->procNames + s->procNamesIdx[i]); //Get process name

        if (strcmp(name, nameKey) == 0) {
            return s->procMemSize[i]; //Return size
        }
    }

    return -1; //Could not find name
}

//Create/connect semaphore and return ID
sem_t *initSemaphore(char *semName) {
    sem_t *sem;

    sem = sem_open(semName, O_CREAT, 0644, 1);

    if(sem == SEM_FAILED) {
        printf("Could not create semaphore: %s", semName);
        sem_unlink(semName);
        exit(1);
    }

    return sem;
}

//Destroy/unlink semaphores
void destorySemaphore(sem_t *sem, char *semName) {

    sem_close(sem);
    sem_unlink(semName);

}


int serializePacket(unsigned char *serialBuffer, unsigned char type, int offset, void *dataPointer, int dataSize) {

    unsigned char pktSize = sizeof(type) + sizeof(offset) + sizeof(pktSize) + dataSize;
    unsigned char writeCount = 0;
    
    memcpy( serialBuffer, &pktSize, sizeof(pktSize));
    writeCount += sizeof(pktSize);
    
    memcpy( serialBuffer + writeCount, &type, sizeof(type));
    writeCount += sizeof(type);

    memcpy( serialBuffer + writeCount, &offset, sizeof(offset));
    writeCount += sizeof(offset);

    memcpy( serialBuffer + writeCount, dataPointer, dataSize);
    
    return pktSize;

}


int decodePacket( unsigned char *serialBuffer, unsigned char *pktSize, unsigned char *type, int *offset) {

    unsigned char readCount = 0;

    memcpy( pktSize, serialBuffer, sizeof(char));
    readCount += sizeof(char);

    memcpy( type, serialBuffer + readCount, sizeof(char));
    readCount += sizeof(char);

    memcpy( offset, serialBuffer + readCount, sizeof(int));
    readCount += sizeof(int);

    return readCount;

}





