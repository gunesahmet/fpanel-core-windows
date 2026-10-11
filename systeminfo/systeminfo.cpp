#include "stdafx.h"
#include <atlbase.h>	// for CRegKey use
#include <iostream>	// for CRegKey use
#include "systeminfo.h"

#pragma pack(push,8)
#include "PerfCounters.h"
#pragma pack(pop)

#define SYSTEM_OBJECT_INDEX					2		// 'System' object
#define PROCESS_OBJECT_INDEX				230		// 'Process' object
#define PROCESSOR_OBJECT_INDEX				238		// 'Processor' object
#define TOTAL_PROCESSOR_TIME_COUNTER_INDEX	240		// '% Total processor time' counter (valid in WinNT under 'System' object)
#define PROCESSOR_TIME_COUNTER_INDEX		6		// '% processor time' counter (for Win2K/XP)


typedef enum
{
	WINNT,	WIN2K_XP, WIN9X, UNKNOWN
}PLATFORM;

PLATFORM GetPlatform()
{
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&osvi))
		return UNKNOWN;
	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_WINDOWS:
		return WIN9X;
	case VER_PLATFORM_WIN32_NT:
		if (osvi.dwMajorVersion == 4)
			return WINNT;
		else
			return WIN2K_XP;
	}
	return UNKNOWN;
}

CCpuUsage::CCpuUsage()
{
	m_bFirstTime = true;
	m_lnOldValue = 0;
	memset(&m_OldPerfTime100nSec, 0, sizeof(m_OldPerfTime100nSec));
}

CCpuUsage::~CCpuUsage()
{
}

BOOL CCpuUsage::EnablePerformaceCounters(BOOL bEnable)
{
	if (GetPlatform() != WIN2K_XP)
		return TRUE;

	CRegKey regKey;
	if (regKey.Open(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\PerfOS\\Performance") != ERROR_SUCCESS)
		return FALSE;

	regKey.SetValue(!bEnable, "Disable Performance Counters");
	regKey.Close();

	if (regKey.Open(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\PerfProc\\Performance") != ERROR_SUCCESS)
		return FALSE;

	regKey.SetValue(!bEnable, "Disable Performance Counters");
	regKey.Close();

	return TRUE;
}

int CCpuUsage::GetCpuUsage()
{
	static PLATFORM Platform = GetPlatform();

	if (m_bFirstTime)
		EnablePerformaceCounters();
	
	// Cpu usage counter is 8 byte length.
	CPerfCounters<LONGLONG> PerfCounters;
	char szInstance[256] = {0};

	DWORD dwObjectIndex;
	DWORD dwCpuUsageIndex;
	
	switch (Platform)
	{
	case WINNT:
		dwObjectIndex = SYSTEM_OBJECT_INDEX;
		dwCpuUsageIndex = TOTAL_PROCESSOR_TIME_COUNTER_INDEX;
		break;
	case WIN2K_XP:
		dwObjectIndex = PROCESSOR_OBJECT_INDEX;
		dwCpuUsageIndex = PROCESSOR_TIME_COUNTER_INDEX;
		strcpy(szInstance,"_Total");
		break;
	default:
		return -1;
	}

	int				CpuUsage = 0;
	LONGLONG		lnNewValue = 0;
	PPERF_DATA_BLOCK pPerfData = NULL;
	LARGE_INTEGER	NewPerfTime100nSec = {0};

	lnNewValue = PerfCounters.GetCounterValue(&pPerfData, dwObjectIndex, dwCpuUsageIndex, szInstance);
	NewPerfTime100nSec = pPerfData->PerfTime100nSec;

	if (m_bFirstTime)
	{
		m_bFirstTime = false;
		m_lnOldValue = lnNewValue;
		m_OldPerfTime100nSec = NewPerfTime100nSec;
		return 0;
	}

	LONGLONG lnValueDelta = lnNewValue - m_lnOldValue;
	double DeltaPerfTime100nSec = (double)NewPerfTime100nSec.QuadPart - (double)m_OldPerfTime100nSec.QuadPart;

	m_lnOldValue = lnNewValue;
	m_OldPerfTime100nSec = NewPerfTime100nSec;

	double a = (double)lnValueDelta / DeltaPerfTime100nSec;

	double f = (1.0 - a) * 100.0;
	CpuUsage = (int)(f + 0.5);	// rounding the result
	if (CpuUsage < 0)
		return 0;
	return CpuUsage;
}

int CCpuUsage::GetCpuUsage(LPCTSTR pProcessName)
{
	static PLATFORM Platform = GetPlatform();
	
	if (m_bFirstTime)
		EnablePerformaceCounters();

	// Cpu usage counter is 8 byte length.
	CPerfCounters<LONGLONG> PerfCounters;
	char szInstance[256] = {0};


	DWORD dwObjectIndex = PROCESS_OBJECT_INDEX;
	DWORD dwCpuUsageIndex = PROCESSOR_TIME_COUNTER_INDEX;
	strcpy(szInstance,pProcessName);

	int				CpuUsage = 0;
	LONGLONG		lnNewValue = 0;
	PPERF_DATA_BLOCK pPerfData = NULL;
	LARGE_INTEGER	NewPerfTime100nSec = {0};

	lnNewValue = PerfCounters.GetCounterValue(&pPerfData, dwObjectIndex, dwCpuUsageIndex, szInstance);
	NewPerfTime100nSec = pPerfData->PerfTime100nSec;

	if (m_bFirstTime)
	{
		m_bFirstTime = false;
		m_lnOldValue = lnNewValue;
		m_OldPerfTime100nSec = NewPerfTime100nSec;
		return 0;
	}

	LONGLONG lnValueDelta = lnNewValue - m_lnOldValue;
	double DeltaPerfTime100nSec = (double)NewPerfTime100nSec.QuadPart - (double)m_OldPerfTime100nSec.QuadPart;

	m_lnOldValue = lnNewValue;
	m_OldPerfTime100nSec = NewPerfTime100nSec;

	double a = (double)lnValueDelta / DeltaPerfTime100nSec;

	CpuUsage = (int) (a*100);
	if (CpuUsage < 0)
		return 0;
	return CpuUsage;
}

int CCpuUsage::GetCpuUsage(DWORD dwProcessID)
{
	static PLATFORM Platform = GetPlatform();
	

	if (m_bFirstTime)
		EnablePerformaceCounters();

	// Cpu usage counter is 8 byte length.
	CPerfCounters<LONGLONG> PerfCounters;


	DWORD dwObjectIndex = PROCESS_OBJECT_INDEX;
	DWORD dwCpuUsageIndex = PROCESSOR_TIME_COUNTER_INDEX;

	int				CpuUsage = 0;
	LONGLONG		lnNewValue = 0;
	PPERF_DATA_BLOCK pPerfData = NULL;
	LARGE_INTEGER	NewPerfTime100nSec = {0};

	lnNewValue = PerfCounters.GetCounterValueForProcessID(&pPerfData, dwObjectIndex, dwCpuUsageIndex, dwProcessID);
	NewPerfTime100nSec = pPerfData->PerfTime100nSec;

	if (m_bFirstTime)
	{
		m_bFirstTime = false;
		m_lnOldValue = lnNewValue;
		m_OldPerfTime100nSec = NewPerfTime100nSec;
		return 0;
	}

	LONGLONG lnValueDelta = lnNewValue - m_lnOldValue;
	double DeltaPerfTime100nSec = (double)NewPerfTime100nSec.QuadPart - (double)m_OldPerfTime100nSec.QuadPart;

	m_lnOldValue = lnNewValue;
	m_OldPerfTime100nSec = NewPerfTime100nSec;

	double a = (double)lnValueDelta / DeltaPerfTime100nSec;

	CpuUsage = (int) (a*100);
	//if (CpuUsage < 0)
	//	return 0;
	return CpuUsage;
}

