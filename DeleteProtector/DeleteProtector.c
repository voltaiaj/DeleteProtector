#include <ntifs.h>
#include "DeleteProtector.h"
#include "DeleteProtectorCommon.h"
#include "MiniFilter.h"

NTSTATUS DeleteProtectorCreateClose(PDEVICE_OBJECT, PIRP);

FilterState g_FilterState;

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\DeleteProtector");
	PDEVICE_OBJECT deviceObject = NULL;
	NTSTATUS status = STATUS_SUCCESS;

	status = InitMiniFilter(DriverObject, RegistryPath);
	if (!NT_SUCCESS(status)) {
		KdPrint((DRIVER_PREFIX "Failed to initialize mini-filter (0x%08X)\n", status));
		//IoDeleteDevice(deviceObject);
		return status;
	}

	status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObject);

	if (!NT_SUCCESS(status)) {
		KdPrint((DRIVER_PREFIX "Failed to create device (0x%08X)\n", status));
		return status;
	}

	UNICODE_STRING symbolicLinkName = RTL_CONSTANT_STRING(L"\\??\\DeleteProtector");
	status = IoCreateSymbolicLink(&symbolicLinkName, &deviceName);
	if (!NT_SUCCESS(status)) {
		KdPrint((DRIVER_PREFIX "Failed to create symbolic link (0x%08X)\n", status));
		IoDeleteDevice(deviceObject);
		return status;
	}

	g_FilterState.DriverObject = DriverObject;
	
	status = FltStartFiltering(g_FilterState.FilterHandle);
	if (!NT_SUCCESS(status)) {
		KdPrint((DRIVER_PREFIX "Failed to start filtering (0x%08X)\n", status));
		IoDeleteSymbolicLink(&symbolicLinkName);
		IoDeleteDevice(deviceObject);
		return status;
	}

	

	DriverObject->MajorFunction[IRP_MJ_CREATE] = 
		DriverObject->MajorFunction[IRP_MJ_CLOSE] =	DeleteProtectorCreateClose;

    return STATUS_SUCCESS;
}

NTSTATUS DeleteProtectorCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
