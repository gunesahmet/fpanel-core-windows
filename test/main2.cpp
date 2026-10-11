#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <thread>
#include <windows.h>

HANDLE hStdInPipeRead = NULL;
HANDLE hStdInPipeWrite = NULL;
HANDLE hStdOutPipeRead = NULL;
HANDLE hStdOutPipeWrite = NULL;
PROCESS_INFORMATION pi = {};

int writePipe(LPWSTR lpCommandLine)
{
	BOOL ok = TRUE;
	// Create two pipes.
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	ok = CreatePipe(&hStdInPipeRead, &hStdInPipeWrite, &sa, 0);
	if (ok == FALSE) return -1;
	ok = CreatePipe(&hStdOutPipeRead, &hStdOutPipeWrite, &sa, 0);
	if (ok == FALSE) return -1;

	// Create the process.
	STARTUPINFO si = {};
	si.cb = sizeof(STARTUPINFO);

	//startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	//startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	//startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);

	//si.dwFlags = STARTF_USESTDHANDLES;
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdError = hStdOutPipeWrite;
	si.hStdOutput = hStdOutPipeWrite;
	si.hStdInput = hStdInPipeRead;
	si.wShowWindow = SW_HIDE;
	LPCWSTR lpApplicationName = L"C:\\Windows\\System32\\cmd.exe";
	//lpCommandLine = (LPWSTR)L"C:\\Windows\\System32\\cmd.exe /c ping google.com";
	LPSECURITY_ATTRIBUTES lpProcessAttributes = NULL;
	LPSECURITY_ATTRIBUTES lpThreadAttribute = NULL;
	BOOL bInheritHandles = TRUE;
	DWORD dwCreationFlags = 0;
	LPVOID lpEnvironment = NULL;
	LPCWSTR lpCurrentDirectory = NULL;
	if (CreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttribute, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, &si, &pi)) {
	/*	WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return true;*/
	}
	else {
		DWORD errorCode = GetLastError();
		//displayError(TEXT("Unable to execute."), errorCode);
	}
	if (ok == FALSE) return -1;

	// Close pipes we do not need.
	CloseHandle(hStdOutPipeWrite);
	CloseHandle(hStdInPipeRead);


	return 0;
}

int readPipe()
{
	BOOL ok = TRUE;
	char buf[1024 + 1] = {};
	DWORD dwRead = 0;
	DWORD dwAvail = 0;
	ok = ReadFile(hStdOutPipeRead, buf, 1024, &dwRead, NULL);
	while (ok == TRUE) {
		buf[dwRead] = '\0';
		OutputDebugStringA(buf);
		puts(buf);
		ok = ReadFile(hStdOutPipeRead, buf, 1024, &dwRead, NULL);
	}

	// Clean up and exit.
	CloseHandle(hStdOutPipeRead);
	CloseHandle(hStdInPipeWrite);
	DWORD dwExitCode = 0;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);
	return dwExitCode;
}

int __cdecl main2(int argc, char** argv)
{
	//std::cout << "test 1" << std::endl;
	//Sleep(150);
	//std::cout << "test 1" << std::endl;
	//Sleep(350);
	//std::cout << "test 1" << std::endl;
	//Sleep(50);
	//std::cout << "test 1" << std::endl;
	//Sleep(500);
	//std::cout << "test 1" << std::endl;
	//Sleep(1050);
	//std::cout << "test 1" << std::endl;
	//Sleep(1500);
	//std::cout << "test 1" << std::endl;
	//Sleep(2500);
	//std::cout << "test 1" << std::endl;
	//Sleep(5000);
	//std::cout << "test 1" << std::endl;
	//Sleep(4050);
	//std::cout << "test 1" << std::endl;
	//Sleep(5500);
	//std::cout << "test 1" << std::endl;
	//Sleep(15000);
	//std::cout << "test 1" << std::endl;


	std::thread clientThread(&writePipe, (LPWSTR)L"C:\\Windows\\System32\\cmd.exe /c C:\\Windows\\System32\\cmd.exe /c  C:\\Users\\Administrator\\Desktop\\installer --exec --fast --verbose --iis --perl --python39 --php56 --php72 --php74 --mysql80 --mssql16 --bind --hmail --spamassassin --roundcube --postgresql13 --phpmyadmin50 --mylittleadmin --awstats --webalizer --hostname=windows.demo.fpanel.net");
	clientThread.detach();
	Sleep(1000);
	readPipe();
	return 0;
}