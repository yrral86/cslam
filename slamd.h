#ifndef __SLAMD_H__
#define __SLAMD_H__

#ifndef LINUX
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "swarm.h"

static LPCWSTR param_shm_name = L"slamd_parameters";
static LPCWSTR return_shm_name = L"slamd_return";
static LPCWSTR param_sem_name = L"slamd_parameters_sem";
static LPCWSTR return_sem_name = L"slamd_return_sem";
static LPCWSTR ready_sem_name = L"slamd_ready_sem";

static HANDLE param_sem;
static HANDLE return_sem;
static HANDLE ready_sem;
static HANDLE param_handle;
static HANDLE return_handle;

static int *params;
static int *return_value;

#endif

typedef enum {SLAMD_INIT = 0,
	      SLAMD_MOVE = 1,
	      SLAMD_UPDATE = 2,
	      SLAMD_X = 3,
	      SLAMD_Y = 4,
	      SLAMD_THETA = 5,
              SLAMD_MAP = 6,
              SLAMD_CONVERGED = 7} slamd_method;

#endif
