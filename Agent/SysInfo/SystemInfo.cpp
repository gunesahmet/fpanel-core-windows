#pragma once
#include "SystemInfo.h"
#include <tchar.h>

#pragma comment(lib, "User32.lib")
typedef void (WINAPI *PGETSYSTEMINFO)(LPSYSTEM_INFO);
typedef BOOL(WINAPI *PGETPRODUCTINFO)(DWORD, DWORD, DWORD, DWORD, PDWORD);

#define BUFSIZE 256  
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL(WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
#define PRODUCT_PROFESSIONAL    0x00000030  
#define VER_SUITE_WH_SERVER 0x00008000  

bool SystemInfo::isValidVersion()
{
	if (IsWindows10OrGreater()) {
		return true;
	}

	return false;
}

bool SystemInfo::isServer() {
	if (IsWindowsServer()) {
		return true;
	}
	return false;
}

bool SystemInfo::getVer(LPTSTR pszOS)
{
	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	PGPI pGPI;
	BOOL bOsVersionInfoEx;
	DWORD dwType;

	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi);

	if (!bOsVersionInfoEx) return 1;

	// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.  

	pGNSI = (PGNSI)GetProcAddress(
		GetModuleHandle(TEXT("kernel32.dll")),
		"GetNativeSystemInfo");
	if (NULL != pGNSI)
		pGNSI(&si);
	else GetSystemInfo(&si);

	if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId &&
		osvi.dwMajorVersion > 4) {
		StringCchCopy(pszOS, BUFSIZE, TEXT("Microsoft "));

		// Test for the specific product.  

		if (osvi.dwMajorVersion == 6) {
			if (osvi.dwMinorVersion == 1) {
				if (osvi.wProductType == VER_NT_SERVER)
					_tcscat_s(pszOS, BUFSIZE, TEXT("Windows Server 2008 R2 "));
			}

			if (osvi.dwMinorVersion == 2) {
				if (osvi.wProductType == VER_NT_SERVER)
					_tcscat_s(pszOS, BUFSIZE, TEXT("Windows Server 2012 R2 "));
			}

			pGPI = (PGPI)GetProcAddress(
				GetModuleHandle(TEXT("kernel32.dll")),
				"GetProductInfo");

			pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);
			std::cout << dwType << std::endl;
			std::cout << PRODUCT_DATACENTER_EVALUATION_SERVER << std::endl;
			std::cout << PRODUCT_DATACENTER_SERVER_CORE << std::endl;
			switch (dwType) {
			case PRODUCT_DATACENTER_SERVER:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Datacenter Edition"));
				break;
			case PRODUCT_DATACENTER_SERVER_CORE:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Datacenter Edition (core installation)"));
				break;
			case PRODUCT_DATACENTER_EVALUATION_SERVER:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Datacenter Evaluation Edition"));
				break;
			case PRODUCT_DATACENTER_SERVER_V:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Server Datacenter without Hyper-V (full installation)"));
				break;
			case PRODUCT_DATACENTER_SERVER_CORE_V:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Server Datacenter without Hyper-V (core installation)"));
				break;

			case PRODUCT_STANDARD_EVALUATION_SERVER:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Standart Evaluation Edition"));
				break;
			case PRODUCT_STANDARD_SERVER:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Standard Edition"));
				break;
			case PRODUCT_STANDARD_SERVER_CORE:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Standard Edition (core installation)"));
				break;
			case PRODUCT_WEB_SERVER:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Web Server Edition"));
				break;
			
			
			case PRODUCT_ENTERPRISE:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Enterprise Edition"));
				break;
			case PRODUCT_BUSINESS:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Business Edition"));
				break;
			case PRODUCT_STARTER:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Starter Edition"));
				break;
			case PRODUCT_CLUSTER_SERVER:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Cluster Server Edition"));
				break;
			case PRODUCT_ENTERPRISE_SERVER:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Enterprise Edition"));
				break;
			case PRODUCT_ENTERPRISE_SERVER_CORE:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Enterprise Edition (core installation)"));
				break;
			case PRODUCT_ENTERPRISE_SERVER_IA64:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Enterprise Edition for Itanium-based Systems"));
				break;
			case PRODUCT_SMALLBUSINESS_SERVER:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Small Business Server"));
				break;
			case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
				_tcscat_s(pszOS, BUFSIZE, TEXT("Small Business Server Premium Edition"));
				break;
			}
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
			if (GetSystemMetrics(SM_SERVERR2))
				_tcscat_s(pszOS, BUFSIZE, TEXT("Windows Server 2003 R2, "));
			else if (osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER)
				_tcscat_s(pszOS, BUFSIZE, TEXT("Windows Storage Server 2003"));
			else if (osvi.wSuiteMask & VER_SUITE_WH_SERVER)
				_tcscat_s(pszOS, BUFSIZE, TEXT("Windows Home Server"));
			else if (osvi.wProductType == VER_NT_WORKSTATION &&
				si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
				_tcscat_s(pszOS, BUFSIZE, TEXT("Windows XP Professional x64 Edition"));
			}
			else _tcscat_s(pszOS, BUFSIZE, TEXT("Windows Server 2003, "));

			// Test for the server type.  
			if (osvi.wProductType != VER_NT_WORKSTATION) {
				if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						_tcscat_s(pszOS, BUFSIZE, TEXT("Datacenter Edition for Itanium-based Systems"));
					else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						_tcscat_s(pszOS, BUFSIZE, TEXT("Enterprise Edition for Itanium-based Systems"));
				}

				else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						_tcscat_s(pszOS, BUFSIZE, TEXT("Datacenter x64 Edition"));
					else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						_tcscat_s(pszOS, BUFSIZE, TEXT("Enterprise x64 Edition"));
					else _tcscat_s(pszOS, BUFSIZE, TEXT("Standard x64 Edition"));
				}

				else {
					if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
						_tcscat_s(pszOS, BUFSIZE, TEXT("Compute Cluster Edition"));
					else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						_tcscat_s(pszOS, BUFSIZE, TEXT("Datacenter Edition"));
					else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						_tcscat_s(pszOS, BUFSIZE, TEXT("Enterprise Edition"));
					else if (osvi.wSuiteMask & VER_SUITE_BLADE)
						_tcscat_s(pszOS, BUFSIZE, TEXT("Web Edition"));
					else _tcscat_s(pszOS, BUFSIZE, TEXT("Standard Edition"));
				}
			}
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
			_tcscat_s(pszOS, BUFSIZE, TEXT("Windows XP "));
			if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
				_tcscat_s(pszOS, BUFSIZE, TEXT("Home Edition"));
			else _tcscat_s(pszOS, BUFSIZE, TEXT("Professional"));
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
			_tcscat_s(pszOS, BUFSIZE, TEXT("Windows 2000 "));

			if (osvi.wProductType == VER_NT_WORKSTATION) {
				_tcscat_s(pszOS, BUFSIZE, TEXT("Professional"));
			}
			else {
				if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
					_tcscat_s(pszOS, BUFSIZE, TEXT("Datacenter Server"));
				else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
					_tcscat_s(pszOS, BUFSIZE, TEXT("Advanced Server"));
				else _tcscat_s(pszOS, BUFSIZE, TEXT("Server"));
			}
		}

		// Include service pack (if any) and build number.  

		if (_tcslen(osvi.szCSDVersion) > 0) {
			_tcscat_s(pszOS, BUFSIZE, TEXT(" "));
			_tcscat_s(pszOS, BUFSIZE, osvi.szCSDVersion);
		}

		TCHAR buf[80];

		_stprintf_s(buf, 80, TEXT(" (build %d)"), osvi.dwBuildNumber);
		_tcscat_s(pszOS, BUFSIZE, buf);

		if (osvi.dwMajorVersion >= 6) {
			if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				_tcscat_s(pszOS, BUFSIZE, TEXT(", 64-bit"));
			else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
				_tcscat_s(pszOS, BUFSIZE, TEXT(", 32-bit"));
		}

		return TRUE;
	}

	else {
		printf("This sample does not support this version of Windows.\n");
		return FALSE;
	}
}

void test() 
{
	OSVERSIONINFO osvi;
	BOOL bIsWindowsXPorLater;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osvi);

	int majorVersion = osvi.dwMajorVersion;
	int minorVersion = osvi.dwMinorVersion;

	std::cout << "majorVersion: " << majorVersion << std::endl;
	std::cout << "minorVersion: " << minorVersion << std::endl;
}

typedef void (WINAPI * RtlGetVersion_FUNC) (OSVERSIONINFOEXW *);
int SystemInfo::getVersion()
{
	HMODULE hMod;
	RtlGetVersion_FUNC func;
	//TCHAR szOS[BUFSIZE];
	//getVer(szOS);
	//_tprintf(TEXT("\n%s\n"), szOS);
	OSVERSIONINFOEXW osvi;
	OSVERSIONINFOEXW * osw = &osvi;
	hMod = LoadLibrary(TEXT("ntdll.dll"));
	if (hMod) {
		func = (RtlGetVersion_FUNC)GetProcAddress(hMod, "RtlGetVersion");
		if (func == 0) {
			FreeLibrary(hMod);
			return FALSE;
		}
		ZeroMemory(osw, sizeof(*osw));
		osw->dwOSVersionInfoSize = sizeof(*osw);
		func(osw);
		//std::cout << "majorVersion: " << osw->dwMajorVersion << std::endl;
		//std::cout << "minorVersion: " << osw->dwMinorVersion << std::endl;
		//std::cout << "buildNumber : " << osw->dwBuildNumber << std::endl;
		//std::cout << "dwPlatformId : " << osw->dwPlatformId << std::endl;
		//std::cout << "dwOSVersionInfoSize : " << osw->dwOSVersionInfoSize << std::endl;

		int majorVersion = osw->dwMajorVersion;
		int minorVersion = osw->dwMinorVersion;
		int buildNumber = osw->dwBuildNumber;
		int dwPlatformId = osw->dwPlatformId;


		FreeLibrary(hMod);

		if (majorVersion == 10) {
			if (minorVersion == 0) {
				if (buildNumber < 14999) {
					return Win2k16R2;
				}
				else if (buildNumber < 17900) {
					return Win2k19R2;
				}
			}
		}
		return -1;
	}
	else
		return -1;
}

bool SystemInfo::getCore(std::string str) {
	if (str.find("Core") != std::string::npos) {
		return true;
	}
	return false;
}
