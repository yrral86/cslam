#ifndef __SLAMD_H__
#define __SLAMD_H__

#ifndef LINUX
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "swarm.h"

typedef enum {SLAMD_INIT, SLAMD_MOVE, SLAMD_UPDATE, SLAMD_X, SLAMD_Y, SLAMD_THETA} slamd_method;

static LPCWSTR param_shm_name = L"slamd_parameters";
static LPCWSTR return_shm_name = L"slamd_return";
static LPCWSTR param_sem_name = L"slamd_parameters_sem";
static LPCWSTR return_sem_name = L"slamd_return_sem";

static HANDLE param_sem;
static HANDLE return_sem;
static HANDLE param_handle;
static HANDLE return_handle;

static int *params;
static int *return_value;

#endif

#endif
