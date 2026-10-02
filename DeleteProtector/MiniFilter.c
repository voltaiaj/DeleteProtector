#include <ntifs.h>
#include "DeleteProtector.h"
#include "MiniFilter.h"

NTSTATUS InitMiniFilter(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(DriverObject);
	// Initialize your mini-filter here
	// For example, you can register a callback for file operations
	WCHAR extension[] = L".txt"; // Example extension to protect
	g_FilterState.Extentions.Buffer = (PWSTR)ExAllocatePool2(PagedPool, sizeof(extension), DRIVER_TAG);
	if (g_FilterState.Extentions.Buffer == NULL) {
		KdPrint((DRIVER_PREFIX "Failed to allocate memory for extensions\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	memcpy(g_FilterState.Extentions.Buffer, extension, sizeof(extension));
	g_FilterState.Extentions.Length = sizeof(extension) - sizeof(WCHAR);

	HANDLE hKey = NULL;
	HANDLE hSubKey = NULL;
	NTSTATUS status = STATUS_SUCCESS;

	OBJECT_ATTRIBUTES keyAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES(RegistryPath, OBJ_KERNEL_HANDLE);
	status = ZwOpenKey(&hKey, KEY_WRITE, &keyAttributes);
	if (!NT_SUCCESS(status)) {
		KdPrint((DRIVER_PREFIX "Failed to open registry key (0x%08X)\n", status));
		return status;
	}

	UNICODE_STRING subKey = RTL_CONSTANT_STRING(L"Instances");
	OBJECT_ATTRIBUTES subKeyAttributes;
	InitializeObjectAttributes(&subKeyAttributes, &subKey, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, hKey, NULL);

	status = ZwCreateKey(&hSubKey, KEY_WRITE, &subKeyAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, NULL);
	if (!NT_SUCCESS(status)) {
		KdPrint((DRIVER_PREFIX "Failed to create subkey (0x%08X)\n", status));
		ZwClose(hKey);
		return status;
	}

	UNICODE_STRING valueName = RTL_CONSTANT_STRING(L"DefaultInstance");
	UNICODE_STRING valueData = RTL_CONSTANT_STRING(L"DeleteProtectorInstance");
	status = ZwSetValueKey(hSubKey, &valueName, 0, REG_SZ, valueData.Buffer, valueData.Length + sizeof(WCHAR));
	if (!NT_SUCCESS(status)) {
		KdPrint((DRIVER_PREFIX "Failed to set value (0x%08X)\n", status));
		ZwClose(hSubKey);
		ZwClose(hKey);
		return status;
	}

	UNICODE_STRING instKeyName;
	RtlInitUnicodeString(&instKeyName, valueData.Buffer);
	HANDLE hInstKey = NULL;
	OBJECT_ATTRIBUTES instKeyAttributes;
	InitializeObjectAttributes(&instKeyAttributes, &instKeyName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, hKey, NULL);
	status = ZwCreateKey(&hInstKey, KEY_WRITE, &instKeyAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, NULL);
	if (!NT_SUCCESS(status)) {
		KdPrint((DRIVER_PREFIX "Failed to create instance key (0x%08X)\n", status));
		ZwClose(hSubKey);
		ZwClose(hKey);
		return status;
	}

	WCHAR altitude[] = L"425342";
	UNICODE_STRING altitudeName = RTL_CONSTANT_STRING(L"Altitude");
	status = ZwSetValueKey(hInstKey, &altitudeName, 0, REG_SZ, altitude, sizeof(altitude));
	if (!NT_SUCCESS(status)) {
		KdPrint((DRIVER_PREFIX "Failed to set altitude value (0x%08X)\n", status));
		ZwClose(hInstKey);
		ZwClose(hSubKey);
		ZwClose(hKey);
		return status;
	}

	UNICODE_STRING flagsName = RTL_CONSTANT_STRING(L"Flags");
	ULONG flags = 0;
	status = ZwSetValueKey(hInstKey, &flagsName, 0, REG_DWORD, &flags, sizeof(flags));
	if (!NT_SUCCESS(status)) {
		KdPrint((DRIVER_PREFIX "Failed to set flags value (0x%08X)\n", status));
		ZwClose(hInstKey);
		ZwClose(hSubKey);
		ZwClose(hKey);
		return status;
	}

	FLT_OPERATION_REGISTRATION const callbacks[] = {
		{ IRP_MJ_CREATE, 0, NULL, NULL },
		{ IRP_MJ_SET_INFORMATION, 0, NULL, NULL },
		{ IRP_MJ_OPERATION_END }
	};

	FLT_REGISTRATION const filterRegistration = {
		sizeof(FLT_REGISTRATION),
		FLT_REGISTRATION_VERSION,
		0,
		NULL,
		callbacks,
		DeleteProtectorUnload,
		DeleteProtectorInstanceSetup,
		DeleteProtectorInstanceQueryTeardown,
		DeleteProtectorInstanceTeardownStart,
		DeleteProtectorInstanceTeardownComplete,
	};

	status = FltRegisterFilter(DriverObject, &filterRegistration, &g_FilterState.FilterHandle);
	if (!NT_SUCCESS(status)) {
		KdPrint((DRIVER_PREFIX "Failed to register filter (0x%08X)\n", status));
		ZwClose(hInstKey);
		ZwClose(hSubKey);
		ZwClose(hKey);
		return status;
	}

	ZwClose(hInstKey);
	ZwClose(hSubKey);
	ZwClose(hKey);

	return STATUS_SUCCESS;
}

NTSTATUS DeleteProtectorUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {
	UNREFERENCED_PARAMETER(Flags);
	if (g_FilterState.Extentions.Buffer) {
		ExFreePoolWithTag(g_FilterState.Extentions.Buffer, DRIVER_TAG);
		g_FilterState.Extentions.Buffer = NULL;
	}
	return STATUS_SUCCESS;
}

NTSTATUS DeleteProtectorInstanceSetup(PCFLT_RELATED_OBJECTS FltObjects, FLT_INSTANCE_SETUP_FLAGS Flags, DEVICE_TYPE VolumeDeviceType, FLT_FILESYSTEM_TYPE VolumeFilesystemType) {
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);
	UNREFERENCED_PARAMETER(VolumeDeviceType);
	UNREFERENCED_PARAMETER(VolumeFilesystemType);
	return STATUS_SUCCESS;
}

NTSTATUS DeleteProtectorInstanceQueryTeardown(PCFLT_RELATED_OBJECTS FltObjects, FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags) {
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);
	return STATUS_SUCCESS;
}

VOID DeleteProtectorInstanceTeardownStart(PCFLT_RELATED_OBJECTS FltObjects, FLT_INSTANCE_TEARDOWN_FLAGS Flags) {
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);
}

VOID DeleteProtectorInstanceTeardownComplete(PCFLT_RELATED_OBJECTS FltObjects, FLT_INSTANCE_TEARDOWN_FLAGS Flags) {
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);
}

FLT_PREOP_CALLBACK_STATUS DeleteProtectorPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);
	if (Data->Iopb->MajorFunction == IRP_MJ_CREATE) {
		PUNICODE_STRING fileName = &Data->Iopb->TargetFileObject->FileName;
		if (fileName->Length >= g_FilterState.Extentions.Length &&
			RtlCompareMemory(fileName->Buffer + (fileName->Length / sizeof(WCHAR)) - (g_FilterState.Extentions.Length / sizeof(WCHAR)),
				g_FilterState.Extentions.Buffer, g_FilterState.Extentions.Length) == g_FilterState.Extentions.Length) {
			Data->IoStatus.Status = STATUS_ACCESS_DENIED;
			return FLT_PREOP_COMPLETE;
		}
	}
	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}