#include <ntifs.h>
#include "MiniFilter.h"

NTSTATUS InitMiniFilter(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);
	// Initialize your mini-filter here
	// For example, you can register a callback for file operations

	HANDLE hKey = NULL;
	HANDLE hSubKey = NULL;
	NTSTATUS status = STATUS_SUCCESS;

	OBJECT_ATTRIBUTES keyAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES(RegistryPath, OBJ_KERNEL_HANDLE);
	status = ZwOpenKey(&hKey, KEY_WRITE, &keyAttributes);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to open registry key (0x%08X)\n", status));
		return status;
	}

	UNICODE_STRING subKey = RTL_CONSTANT_STRING(L"Instances");
	OBJECT_ATTRIBUTES subKeyAttributes;
	InitializeObjectAttributes(&subKeyAttributes, &subKey, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, hKey, NULL);

	status = ZwCreateKey(&hSubKey, KEY_WRITE, &subKeyAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, NULL);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create subkey (0x%08X)\n", status));
		ZwClose(hKey);
		return status;
	}

	UNICODE_STRING valueName = RTL_CONSTANT_STRING(L"DefaultInstance");
	UNICODE_STRING valueData = RTL_CONSTANT_STRING(L"DeleteProtectorInstance");
	status = ZwSetValueKey(hSubKey, &valueName, 0, REG_SZ, valueData.Buffer, valueData.Length + sizeof(WCHAR));
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to set value (0x%08X)\n", status));
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
		KdPrint(("Failed to create instance key (0x%08X)\n", status));
		ZwClose(hSubKey);
		ZwClose(hKey);
		return status;
	}

	WCHAR altitude[] = L"425342";
	UNICODE_STRING altitudeName = RTL_CONSTANT_STRING(L"Altitude");
	status = ZwSetValueKey(hInstKey, &altitudeName, 0, REG_SZ, altitude, sizeof(altitude));
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to set altitude value (0x%08X)\n", status));
		ZwClose(hInstKey);
		ZwClose(hSubKey);
		ZwClose(hKey);
		return status;
	}

	UNICODE_STRING flagsName = RTL_CONSTANT_STRING(L"Flags");
	ULONG flags = 0;
	status = ZwSetValueKey(hInstKey, &flagsName, 0, REG_DWORD, &flags, sizeof(flags));
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to set flags value (0x%08X)\n", status));
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