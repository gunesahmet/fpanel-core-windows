#include <windows.h>

SERVICE_STATUS_HANDLE statusHandle;
PROCESS_INFORMATION processInfo;
LPSTR commandLine;

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);

int main(int argc, char *argv[])
{
	if (argc < 2) return 0;
	commandLine = argv[1];

	// Service entry point
	SERVICE_TABLE_ENTRY table[] = { {"", ServiceMain}, {NULL, NULL} };
	StartServiceCtrlDispatcher(table);
}

void SetStatus(DWORD state)
{
	SERVICE_STATUS status = { SERVICE_WIN32_OWN_PROCESS, state, SERVICE_ACCEPT_STOP, NO_ERROR, 0, 0, 0 };

	// Set allowed commands and wait time
	if ((state == SERVICE_START_PENDING) || (state == SERVICE_STOP_PENDING))
	{
		status.dwControlsAccepted = 0;
		status.dwWaitHint = 2000;
	}

	// Set status
	SetServiceStatus(statusHandle, &status);
}

void WINAPI ControlHandler(DWORD control)
{
	if (control == SERVICE_CONTROL_STOP)
	{
		SetStatus(SERVICE_STOP_PENDING);

		TerminateProcess(processInfo.hProcess, 0);
	}
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
	// Register handler
	statusHandle = RegisterServiceCtrlHandler("FPanel Httpd Service", ControlHandler);
	SetStatus(SERVICE_RUNNING);

	PROCESS_INFORMATION processInfo;
	STARTUPINFO startupInfo;
	LPSTR cmdArgs = "cmd /c C:\fpanel\httpd\lighttpd.exe -f c:\fpanel\httpd\conf\httpd.conf";

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	ZeroMemory(&processInfo, sizeof(processInfo));
	startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	startupInfo.wShowWindow = SW_HIDE;

	if (CreateProcess(NULL, cmdArgs, NULL, NULL, false, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &processInfo)) {
		WaitForInputIdle(processInfo.hProcess, INFINITE);
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
	}

	SetStatus(SERVICE_STOPPED);
}
