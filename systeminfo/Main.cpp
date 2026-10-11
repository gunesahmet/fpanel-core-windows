// Main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <intrin.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include "systeminfo.h"

#include <psapi.h>

#define ID_TIMER 1
#define ID_TIMER_LONGESTUPTIME 2
#define WM_UPTIMENOTIFY (WM_USER+20)
#define MAX_UPTIME_STRING_LENGTH 32


extern LONG GetUpTimeSecs();

using namespace std;

int main(void)
{

	// Cpu Usage
	int processID = 0;
	CCpuUsage usageA;
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	// Disk Usage
	__int64 total, free;
	char Dnames[4] = "c:\\";


	printf("0");

   // Memory Usage
   	MEMORYSTATUSEX statex;

	statex.dwLength = sizeof (statex);

	GlobalMemoryStatusEx (&statex);

	_tprintf (TEXT(",%*ld"), 1, statex.dwMemoryLoad); // Yüzdesi
	_tprintf (TEXT(",%*I64d"), 1, statex.ullTotalPhys); // Total Memory
	_tprintf (TEXT(",%*I64d"), 1, statex.ullTotalPhys - statex.ullAvailPhys); // Used Memory
	_tprintf (TEXT(",%*I64d"), 1, statex.ullAvailPhys); // Free Memory


	// Uptime
	DWORD UpSecs = (DWORD)GetUpTimeSecs();
	DWORD Seconds = UpSecs; 
	printf(",%d", Seconds);

	GetDiskFreeSpaceEx(Dnames,NULL,(PULARGE_INTEGER)&total,(PULARGE_INTEGER)&free);

	printf(",%I64d", total);
	printf(",%I64d", free);
	printf(",%I64d", total-free);


	int CPUInfo[4] = {-1};
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];
 
	// Get the information associated with each extended ID.
	char CPUBrandString[0x40] = { 0 };
	for( unsigned int i=0x80000000; i<=nExIds; ++i) {
		__cpuid(CPUInfo, i);
 
		// Interpret CPU brand string and cache information.
		if  (i == 0x80000002) {
			memcpy( CPUBrandString,
			CPUInfo,
			sizeof(CPUInfo));
		}
		else if( i == 0x80000003 ) {
			memcpy( CPUBrandString + 16,
			CPUInfo,
			sizeof(CPUInfo));
		}
		else if( i == 0x80000004 ) {
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
		}
	}

	cout << "," << CPUBrandString;


	DWORD dwVersion = 0; 
    DWORD dwMajorVersion = 0;
    DWORD dwMinorVersion = 0; 
    DWORD dwBuild = 0;

    dwVersion = GetVersion();
 
    // Get the Windows version.

    dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

    // Get the build number.

    if (dwVersion < 0x80000000)              
        dwBuild = (DWORD)(HIWORD(dwVersion));

	printf(",%d.%d.%d", dwMajorVersion, dwMinorVersion);
	printf(",%u", siSysInfo.dwProcessorType); 
	
	std::cout << "CPUBrandString: " << CPUBrandString << std::endl;
	return 0;

}

