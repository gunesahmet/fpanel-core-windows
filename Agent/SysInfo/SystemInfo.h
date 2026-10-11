#pragma once
#ifndef INSTALLER_SYSTEMINFO_H
#define INSTALLER_SYSTEMINFO_H
#include <C:\Program Files (x86)\Windows Kits\10\Include\10.0.10240.0\um\windows.h>
#include <C:\Program Files (x86)\Windows Kits\10\Include\10.0.10240.0\um\VersionHelpers.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <string.h>
#include <lm.h>

class SystemInfo  
{
	enum versions { Win2k16, Win2k16R2, Win2k19, Win2k19R2 };
public:
	bool isValidVersion();
	bool isServer();
	bool getVer(LPTSTR pszOS);
	int getVersion();
	bool getCore(std::string str);
};
#endif