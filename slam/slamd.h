#ifndef __SLAMD_H__
#define __SLAMD_H__

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "swarm.h"


static const char param_shm_name[] = "slamd_parameters";
static const char return_shm_name[] = "slamd_return";
static const char param_sem_name[] = "slamd_parameters";
static const char return_sem_name[] = "slamd_return";

static HANDLE param_sem;
static HANDLE return_sem;

#endif
