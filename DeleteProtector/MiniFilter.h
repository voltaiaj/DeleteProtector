#pragma once

#include <ntifs.h>      // or <wdm.h>
#include <fltKernel.h>  // defines PFLT_FILTER

typedef struct _FilterState {
	PDRIVER_OBJECT DriverObject;
	UNICODE_STRING FilterName;
	PFLT_FILTER FilterHandle;
	UNICODE_STRING Extensions; // Array to hold extensions to protect
} FilterState;

extern FilterState g_FilterState;

NTSTATUS InitMiniFilter(PDRIVER_OBJECT, PUNICODE_STRING);
NTSTATUS DeleteProtectorUnload(FLT_FILTER_UNLOAD_FLAGS);
NTSTATUS DeleteProtectorInstanceSetup(PCFLT_RELATED_OBJECTS, FLT_INSTANCE_SETUP_FLAGS, DEVICE_TYPE, FLT_FILESYSTEM_TYPE);
NTSTATUS DeleteProtectorInstanceQueryTeardown(PCFLT_RELATED_OBJECTS, FLT_INSTANCE_QUERY_TEARDOWN_FLAGS);
VOID DeleteProtectorInstanceTeardownStart(PCFLT_RELATED_OBJECTS, FLT_INSTANCE_TEARDOWN_FLAGS);
VOID DeleteProtectorInstanceTeardownComplete(PCFLT_RELATED_OBJECTS, FLT_INSTANCE_TEARDOWN_FLAGS);
