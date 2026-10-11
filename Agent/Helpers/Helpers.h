#pragma once
#define WIN32_LEAN_AND_MEAN 1
#ifndef LICENCE_HELPERS_H
#define LICENCE_HELPERS_H

#include <iostream>
#include <sys/stat.h>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <sys/timeb.h>
#include <algorithm>
#include <sstream>
#include <string>
#include <cstring>
#include <initializer_list>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <vector>
#include <iomanip>
#include <fstream>
#include <regex>
#include <unordered_map>
#include <thread>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <random>
#include <boost/foreach.hpp>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>
#include <Helpers/md5.h>

#include <atlbase.h>
#include <atlsecurity.h>
#include <stdlib.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <Winreg.h>
#include <Shlobj.h>
#include <random>
#include <functional>
#include <Shellapi.h>
#include <atlconv.h>
#include <atlstr.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <windows.h>

#define DELIM "."
#define INFO_BUFFER_SIZE 32767
#pragma comment(lib, "urlmon.lib")

using namespace boost::asio;
using namespace boost::asio::ip;

static enum color { DARKBLUE = 1, DARKGREEN, DARKTEAL, DARKRED, DARKPINK, DARKYELLOW, GRAY, DARKGRAY, BLUE, GREEN, TEAL, RED, PINK, YELLOW, WHITE };

struct setColor
{
	color _c;
	HANDLE _console_handle;

	setColor(color c, HANDLE console_handle) : _c(c), _console_handle(0)
	{
		_console_handle = console_handle;
	}
};

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &s, const setColor &ref)
{
	SetConsoleTextAttribute(ref._console_handle, ref._c);
	return s;
}

class Helpers {
private:
	HANDLE hStdInPipeRead = NULL;
	HANDLE hStdInPipeWrite = NULL;
	HANDLE hStdOutPipeRead = NULL;
	HANDLE hStdOutPipeWrite = NULL;
	PROCESS_INFORMATION pi = {};

public:
	static enum colors { DARKBLUE = 1, DARKGREEN, DARKTEAL, DARKRED, DARKPINK, DARKYELLOW, GRAY, DARKGRAY, BLUE, GREEN, TEAL, RED, PINK, YELLOW, WHITE };

    unsigned int RandomChar()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        return dis(gen);
    }

    std::string GenerateHex(const unsigned int len)
    {
        std::stringstream ss;
        for (auto i = 0; i < len; i++) {
            const auto rc = RandomChar();
            std::stringstream hexstream;
            hexstream << std::hex << rc;
            auto hex = hexstream.str();
            ss << (hex.length() < 2 ? '0' + hex : hex);
        }
        return ss.str();
    }

    std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        str.erase(0, str.find_first_not_of(chars));
        return str;
    }

    std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        str.erase(str.find_last_not_of(chars) + 1);
        return str;
    }

    std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        return ltrim(rtrim(str, chars), chars);
    }

    bool ReplaceWithString(std::string& str, const std::string& oldstr, const std::string& newstr)
    {
        size_t start_pos = str.find(oldstr);
        if(start_pos == std::string::npos)
            return false;
        str.replace(start_pos, oldstr.length(), newstr);

        size_t start_pos_s = str.find(oldstr);
        if(start_pos_s != std::string::npos)
            return ReplaceWithString(str, oldstr, newstr);
        return true;
    }

    bool CmdOptionExists(char **begin, char **end, const std::string& option)
    {
        return find(begin, end, option) != end;
    }

    bool IsDirExist(const std::string& path)
    {
        struct stat info;
        if (stat(path.c_str(), &info) != 0) {
            return false;
        }
        return (info.st_mode & S_IFDIR) != 0;
    }

    std::vector<std::string> strSplit(std::string strToSplit, char delimeter)
    {
        std::stringstream ss(strToSplit);
        std::string item;
        std::vector<std::string> splittedStrings;
        while (std::getline(ss, item, delimeter)) {
            splittedStrings.push_back(std::string(item));
        }
        return splittedStrings;
    }

    uint64_t Milliseconds()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    uint64_t Microseconds()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    uint64_t Nanoseconds()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    void MilliSleep(int millis)
    {
        if(millis == 0) {
            millis = 1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    }

    void SecSleep(int seconds)
    {
        if(seconds == 0) {
            seconds = 1;
        }
        seconds = seconds * 1000;
        std::this_thread::sleep_for(std::chrono::milliseconds(seconds));
    }

    void Println(int t)
    {
        std::cout << t <<std::endl;
    }

    void Println(char t)
    {
        std::cout << t <<std::endl ;
    }

    void Println(std::string t)
    {
        std::cout << t <<std::endl ;
    }

    void Print(int t)
    {
        std::cout << t;
    }

    void Print(char t)
    {
        std::cout << t;
    }

    void Print(std::string t)
    {
        std::cout << t;
    }

    bool Write(std::string path, std::string content)
    {
		std::ofstream fs(path.c_str());

		if (!fs) {
			return false;
		}
		fs << content;
		fs.close();
    }

    void Split(std::vector<std::string> &tokens, std::string str, std::string delimiter)
    {
        boost::split(tokens, str, boost::is_any_of(delimiter.c_str()));
    }

    void RemoveLastChar(std::string &str)
    {
        trim(str, "\t\n\v\f\r ");
        str = str.substr(0, str.size()-1);
    }

    void ToLower(std::string &str)
    {
        boost::algorithm::to_lower(str);
    }

    const char* GenerateUniqueKey()
    {
        size_t length = 4;
        auto randchar = []() -> char {
            const char charset[] =
                    "0123456789"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[ rand() % max_index ];
        };
        uint64_t current_nanosecond = Nanoseconds();
        std::string str(length,0);
        std::generate_n( str.begin(), length, randchar );
        return str.append(std::to_string(current_nanosecond)).c_str();
    }

    int RandomInteger(int min, int max)
    {
        return min + (rand() % static_cast<int>(max - min + 1));
    }

    int valid_digit(char *ip_str)
    {
        while (*ip_str) {
            if (*ip_str >= '0' && *ip_str <= '9')
                ++ip_str;
            else
                return 0;
        }
        return 1;
    }

    int IsV4(char *ip_str)
    {
        int num, dots = 0;
        char *ptr;

        if (ip_str == NULL)
            return 0;

        // See following link for strtok()
        // http://pubs.opengroup.org/onlinepubs/009695399/functions/strtok_r.html
        ptr = strtok(ip_str, DELIM);

        if (ptr == NULL)
            return 0;

        while (ptr) {

            /* after parsing string, it must contain only digits */
            if (!valid_digit(ptr))
                return 0;

            num = atoi(ptr);

            /* check for valid IP */
            if (num >= 0 && num <= 255) {
                /* parse remaining string */
                ptr = strtok(NULL, DELIM);
                if (ptr != NULL)
                    ++dots;
            } else
                return 0;
        }

        /* valid IP string must contain 3 dots */
        if (dots != 3)
            return 0;
        return 1;
    }

	const int *ConvertConstInt(int var)
	{
		return new int[var];
	}

    std::string GetIpAddressFromUrl(const std::string &host)
    {

        std::string ip;
    //    try {
    //        boost::asio::io_service io_service;
    //        ip::tcp::resolver resolver(io_service);
    //        ip::tcp::resolver::query query(host, "");
    //        ip::tcp::resolver::iterator iter = resolver.resolve(query);
    //        ip::tcp::resolver::iterator end; // End marker.
    //        while (iter != end) {
    //            ip::tcp::endpoint endpoint = *iter++;
    //            ip = std::string(endpoint.address().to_string());
    //            ReplaceWithString(ip, ":80","");
				//int _ip_size = ip.size() + 1;
				//const int* ip_size = ConvertConstInt(_ip_size);
    //            char cstr[ip_size];
    //            strcpy(cstr, ip.c_str());	// or pass &s[0]


    //            if(IsV4(cstr)){
    //                break;
    //            }
    //        }
    //    }
    //    catch (...) {
    //        return {};
    //    }

        return ip;
    }

    bool IsFileExists(const std::string& name)
    {
        struct stat buffer;
        return (stat (name.c_str(), &buffer) == 0);
    }

    bool IsValidDomain(std::string &domain)
    {
        //^([a-z0-9-]{1,63}|[a-z0-9][a-z0-9-]{0,61}[a-z0-9]-)$
        std::regex rx("^(([a-z0-9-]{1,63}|[a-z0-9-][a-z0-9-]{0,61}[a-z0-9-]))$");
        if (std::regex_match(domain, rx)) {
            return true;
        }
        return false;
    }

    std::string CmdOutput(std::string cmd)
    {
		char buffer[8192];
		std::string result = "";
		std::shared_ptr<FILE> pipe(_popen(cmd.c_str(), "r"), _pclose);
		if (!pipe) {
			result = "";
		}
		while (!feof(pipe.get())) {
			if (fgets(buffer, 128, pipe.get()) != NULL)
				result += buffer;
		}
		return result;
    }

	std::wstring s2ws(const std::string& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}

	bool ExecuteWindows(std::string command, bool wait)
	{
		PROCESS_INFORMATION processInfo;
		STARTUPINFO startupInfo;
		LPSTR cmdArgs = const_cast<char *>(command.c_str());
		ZeroMemory(&startupInfo, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);
		ZeroMemory(&processInfo, sizeof(processInfo));
		startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		startupInfo.wShowWindow = SW_HIDE;

		if (CreateProcess(NULL, cmdArgs, NULL, NULL, false, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &processInfo)) {
			if (wait) {
				WaitForSingleObject(processInfo.hProcess, INFINITE);
				CloseHandle(processInfo.hThread);
				CloseHandle(processInfo.hProcess);
			}
			return true;
		}
		else {
			DWORD errorCode = GetLastError();
			//displayError(TEXT("Unable to execute."), errorCode);
		}
		return false;
	}

	bool ExecuteWindowsWritePipe(std::string command, bool wait)
	{
		BOOL ok = TRUE;
		// Create two pipes.
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
		ok = CreatePipe(&hStdInPipeRead, &hStdInPipeWrite, &sa, 0);
		if (ok == FALSE) return false;
		ok = CreatePipe(&hStdOutPipeRead, &hStdOutPipeWrite, &sa, 0);
		if (ok == FALSE) return false;

		// Create the process.
		STARTUPINFO si = {};
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdError = hStdOutPipeWrite;
		si.hStdOutput = hStdOutPipeWrite;
		si.hStdInput = hStdInPipeRead;
		si.wShowWindow = SW_HIDE;

		//LPCSTR lpApplicationName = "C:\\Windows\\System32\\cmd.exe";
		//LPWSTR lpCommandLine = (LPWSTR)L"C:\\Windows\\System32\\cmd.exe /c ping google.com";
		LPSECURITY_ATTRIBUTES lpProcessAttributes = NULL;
		LPSECURITY_ATTRIBUTES lpThreadAttribute = NULL;
		BOOL bInheritHandles = TRUE;
		DWORD dwCreationFlags = 0;
		LPVOID lpEnvironment = NULL;
		LPCSTR lpCurrentDirectory = NULL;
		LPSTR cmdArgs = const_cast<char *>(command.c_str());

		if (CreateProcess(NULL, cmdArgs, lpProcessAttributes, lpThreadAttribute, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, &si, &pi)) {
			if (wait) {
				WaitForSingleObject(pi.hProcess, INFINITE);
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
			}
			return true;
		}
		else {
			DWORD errorCode = GetLastError();
			//displayError(TEXT("Unable to execute."), errorCode);
		}
		//if (CreateProcess( NULL,             cmdArgs,      NULL,               NULL,             false,          NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL,          NULL,               &startupInfo, &processInfo)) {

		//if (ok == FALSE) return false;

		// Close pipes we do not need.
		CloseHandle(hStdOutPipeWrite);
		CloseHandle(hStdInPipeRead);

		return false;
	}

	bool ExecuteWindowsReadPipe()
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

    bool MoveFile(std::string old_path, std::string new_path)
    {
        if(std::rename(old_path.c_str(), new_path.c_str()) < 0) {
            return false;
        }
        return true;
    }

    long GetFileSize(std::string filename)
    {
        struct stat stat_buf;
        int rc = stat(filename.c_str(), &stat_buf);
        return rc == 0 ? stat_buf.st_size : -1;
    }

    bool CopyFile(std::string old_path, std::string new_path)
    {
        std::ifstream  src(old_path.c_str(), std::ios::binary);
        std::ofstream  dst(new_path.c_str(),   std::ios::binary);
        dst << src.rdbuf();
    }

    bool RemoveFile(std::string filename)
    {
        bool rstatus = false;
        int status = remove(filename.c_str());
        if(status == 0) {
            rstatus = true;
        }
        return rstatus;
    }

	static bool isDirExists(boost::filesystem::path p)
	{
		if (boost::filesystem::exists(p)) {
			if (is_directory(p))
				return true;
			else
				return false;
		}
		return false;
	}

    bool CreateDirectory(std::string path)
    {
		if (!isDirExists(path)) {
			return boost::filesystem::create_directories(path);
		}
    }

    std::string GenerateUuid()
    {
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        const std::string tmp = boost::lexical_cast<std::string>(uuid);
        return std::string(tmp);
    }

    bool isAlpha(std::string &str) {
        if(!isalpha(str[0]) || !isspace(str[0])) {
            return true;
        }
        return false;
    }

    bool is_number(const std::string& s)
    {
        std::string::const_iterator it = s.begin();
        while (it != s.end() && std::isdigit(*it)) ++it;
        return !s.empty() && it == s.end();
    }

    std::string createLicenceKey(std::string default_ip, std::string key, std::string iv)
    {
        std::string licence_iv = ("");
        licence_iv += default_ip;
        licence_iv += key;
        licence_iv += iv;

        std::string licence_md5key = md5(licence_iv.c_str());
        std::string licence_key = "";

        for (int i = 5; (unsigned)i <= licence_md5key.size(); i+=5+1)
            licence_md5key.insert(i, "-");

        licence_md5key = licence_md5key.substr(0, licence_md5key.size()-3);

        return licence_md5key;
    }

    bool commandIsValid(std::string str)
    {
        if(strstr(str.c_str(),"wrapper")) {
            return true;
        }
        if(strstr(str.c_str(),"appinstaller")) {
            return true;
        }
        if(strstr(str.c_str(),"webstatistics")) {
            return true;
        }
        if(strstr(str.c_str(),"mng")) {
            return true;
        }
        else {
            return false;
        }
    }

	std::string getfileContent(std::string file_path)
	{
		std::ifstream read_file;
		read_file.open(file_path);
		std::string line;
		std::string output;
		if (read_file.is_open()) {
			while (!read_file.eof()) {
				read_file >> line;
				output.append(line);
				output.append("\n");
			}
		}
		read_file.close();
		return output;
	}

	std::string exec(const char* cmd)
	{
		FILE* pipe = _popen(cmd, "r");
		if (!pipe) return "ERROR";
		char buffer[262144];
		std::string data;
		std::string result;
		int dist = 0;
		int size;
		while (!feof(pipe)) {
			size = (int)fread(buffer, 1, 262144, pipe);
			data.resize(data.size() + size);
			memcpy(&data[dist], buffer, size);
			dist += size;
		}
		_pclose(pipe);
		return data;
	}

	bool downloadFile(const std::string &repo_address, const std::string &package, const std::string &target)
	{
		std::string fpanel_repo_path = "/";

		std::ofstream outFile(target, std::ofstream::out | std::ofstream::binary);
		boost::asio::io_service io_service;

		// Get a list of endpoints corresponding to the server name.
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(repo_address, "http");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::resolver::iterator end;

		// Try each endpoint until we successfully establish a connection.
		tcp::socket socket(io_service);
		boost::system::error_code error = boost::asio::error::host_not_found;
		while (error && endpoint_iterator != end) {
			socket.close();
			socket.connect(*endpoint_iterator++, error);
		}


		if (error)
			return false;
			//throw boost::system::system_error(error);

		boost::asio::streambuf request;
		std::ostream request_stream(&request);

		request_stream << "GET " << fpanel_repo_path << package << " HTTP/1.1\r\n";
		request_stream << "Host: " << repo_address << "\r\n";
		request_stream << "Accept: */*\r\n";
		request_stream << "Connection: close\r\n\r\n";

		// Send the request.
		boost::asio::write(socket, request);

		// Read the response status line.
		boost::asio::streambuf response;
		boost::asio::read_until(socket, response, "\r\n");

		// Check that response is OK.
		std::istream response_stream(&response);
		std::string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream, status_message);


		// Read the response headers, which are terminated by a blank line.
		boost::asio::read_until(socket, response, "\r\n\r\n");

		// Process the response headers.
		std::string header;
		while (std::getline(response_stream, header) && header != "\r") {
		}

		// Write whatever content we already have to output.
		if (response.size() > 0) {
			outFile << &response;
		}
		// Read until EOF, writing data to output as we go.
		while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error)) {
			if (error) {
				//logger->errorLog("Read downloaded package error: " + std::string(error.message()), 999);
				return false;
			}
			outFile << &response;
		}

		outFile.close();

		return true;
	}

	bool isRegexMatch(const std::string input, const std::regex expression)
	{
		using namespace std::literals::string_literals;
		std::smatch base_match;
		if (std::regex_search(input, base_match, expression)) {
			return true;
		}
		return false;
	}

	bool validateIpAddress(const std::string ip_address)
	{
		return isRegexMatch(ip_address, std::regex("(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})"));
	}

	void consoleWrite(std::string message, int color, bool end)
	{
		HANDLE chandle = GetStdHandle(STD_OUTPUT_HANDLE);
		switch (color) {
			case color::DARKBLUE:
				if (end)
					std::cout << setColor(color::DARKBLUE, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::DARKBLUE, chandle) << message << setColor(color::GRAY, chandle);
				break;
			case color::DARKGRAY:
				if (end)
					std::cout << setColor(color::DARKGRAY, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::DARKGRAY, chandle) << message << setColor(color::GRAY, chandle);
				break;
			case color::DARKGREEN:
				if (end)
					std::cout << setColor(color::DARKGREEN, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::DARKGREEN, chandle) << message << setColor(color::GRAY, chandle);
				break;
			case color::DARKPINK:
				if (end)
					std::cout << setColor(color::DARKPINK, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::DARKPINK, chandle) << message << setColor(color::GRAY, chandle);
				break;
			case color::DARKRED:
				if (end)
					std::cout << setColor(color::DARKRED, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::DARKRED, chandle) << message << setColor(color::GRAY, chandle);
				break;
			case color::DARKTEAL:
				if (end)
					std::cout << setColor(color::DARKTEAL, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::DARKTEAL, chandle) << message << setColor(color::GRAY, chandle);
				break;
			case color::DARKYELLOW:
				if (end)
					std::cout << setColor(color::DARKYELLOW, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::DARKYELLOW, chandle) << message << setColor(color::GRAY, chandle);
				break;
			case color::BLUE:
				if (end)
					std::cout << setColor(color::BLUE, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::BLUE, chandle) << message << setColor(color::GRAY, chandle);
				break;
			case color::GRAY:
				if (end)
					std::cout << setColor(color::GRAY, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::GRAY, chandle) << message << setColor(color::GRAY, chandle);
				break;
			case color::GREEN:
				if (end)
					std::cout << setColor(color::GREEN, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::GREEN, chandle) << message << setColor(color::GRAY, chandle);
				break;
			case color::PINK:
				if (color)
					std::cout << setColor(color::PINK, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::PINK, chandle) << message << setColor(color::GRAY, chandle);
				break;
			case color::RED:
				if (end)
					std::cout << setColor(color::RED, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::RED, chandle) << message << setColor(color::GRAY, chandle);
				break;
			case color::TEAL:
				if (end)
					std::cout << setColor(color::TEAL, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::TEAL, chandle) << message << setColor(color::GRAY, chandle);
				break;
			case color::YELLOW:
				if (end)
					std::cout << setColor(color::YELLOW, chandle) << message << setColor(color::GRAY, chandle) << std::endl;
				else
					std::cout << setColor(color::YELLOW, chandle) << message << setColor(color::GRAY, chandle);
				break;
			default:
				break;
		}
		std::cout << setColor(color::GRAY, chandle);
	}

	long getStringRegKey(HKEY hKey, const std::wstring &strValueName, std::wstring &strValue, const std::wstring &strDefaultValue)
	{
		strValue = strDefaultValue;
		WCHAR szBuffer[512];
		DWORD dwBufferSize = sizeof(szBuffer);
		ULONG nError;
		nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
		if (ERROR_SUCCESS == nError) {
			strValue = szBuffer;
		}
		return nError;
	}

	std::string GetOsName()
	{
		HKEY hKey;
		LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey);

		std::wstring strKeyDefaultValue;
		getStringRegKey(hKey, L"ProductName", strKeyDefaultValue, L"NOT_EXISTS");
		std::string os_name(strKeyDefaultValue.begin(), strKeyDefaultValue.end());

		std::wstring core_name;
		getStringRegKey(hKey, L"InstallationType", core_name, L"NOT_EXISTS");
		std::string is_core(core_name.begin(), core_name.end());

		if (isCore(is_core)) {
			os_name += " (Core)";
		}
		return os_name;
	}

	bool isCore(std::string str)
	{
		if (str.find("Core") != std::string::npos) {
			return true;
		}
		return false;
	}

	std::vector<std::string> IpAddresses()
	{
		std::string ip_lists = exec("c:\\windows\\system32\\route.exe PRINT 0.0.0.0 -4 | find \"0.0.0.0\" | findstr /V \"Default\"");
		std::vector<std::string> ips = std::vector<std::string>();
		wchar_t charsToTrim[3] = { '*', ' ', '\'' };
		std::string expression("0.0.0.0(.*)0.0.0.0");
		int i = 0;

		std::istringstream list_stream(ip_lists);
		std::string line;
		while (std::getline(list_stream, line)) {
			if (line != "") {
				typedef std::vector<std::string> Tokens;
				Tokens tokens;
				boost::split(tokens, line, boost::is_any_of(" "));
				int k = 0;
				BOOST_FOREACH(const std::string& data, tokens)
				{
					if (data != "") {
						//std::cout << "Data: " << data << std::endl;
						if (k == 3) {
							//std::cout << "Found: " << data << std::endl;
							if (validateIpAddress(data.c_str())) {
								trim(std::string(data));
								ips.push_back(data);
							}
						}
						k++;
					}
				}
			}
		}
		return ips;
	}

	bool regexMatch(const std::string input, const std::string expression)
	{
		using namespace std::literals::string_literals;
		std::regex base_regex(expression);
		std::smatch base_match;
		std::string matched;
		if (std::regex_search(input, base_match, base_regex)) {
			if (base_match.size() == 2) {
				return true;
			}
		}
		return false;

	}

	std::string defaultIpAddress()
	{
		std::string ip_lists = exec("c:\\windows\\system32\\route.exe PRINT 0.0.0.0 -4 | find \"0.0.0.0\" | findstr /V \"Default\"");
		std::string default_ip = "";
		wchar_t charsToTrim[3] = { '*', ' ', '\'' };
		std::string expression("0.0.0.0(.*)0.0.0.0");
		int i = 0;

		std::istringstream list_stream(ip_lists);
		std::string line;
		while (std::getline(list_stream, line)) {
			if (line != "") {
				if (regexMatch(line, expression) == true) {
					//std::cout << "Line: " << line << std::endl;
					if (i == 0) {
						typedef std::vector<std::string> Tokens;
						Tokens tokens;
						boost::split(tokens, line, boost::is_any_of(" "));
						int k = 0;
						BOOST_FOREACH(const std::string& data, tokens)
						{
							if (data != "") {
								//std::cout << "Data: " << data << std::endl;
								if (k == 3) {
									//std::cout << "Found: " << data << std::endl;
									if (validateIpAddress(data.c_str())) {
										default_ip = data;
										trim(std::string(default_ip));
									}
								}
								k++;
							}
						}
						i++;
					}
				}
			}
		}
		return std::string(default_ip);
	}

	std::string GetProcessorName()
	{
		int CPUInfo[4] = { -1 };
		__cpuid(CPUInfo, 0x80000000);
		unsigned int nExIds = CPUInfo[0];

		// Get the information associated with each extended ID.
		char CPUBrandString[0x40] = { 0 };
		for (unsigned int i = 0x80000000; i <= nExIds; ++i) {
			__cpuid(CPUInfo, i);

			// Interpret CPU brand string and cache information.
			if (i == 0x80000002) {
				memcpy(CPUBrandString,
					CPUInfo,
					sizeof(CPUInfo));
			}
			else if (i == 0x80000003) {
				memcpy(CPUBrandString + 16,
					CPUInfo,
					sizeof(CPUInfo));
			}
			else if (i == 0x80000004) {
				memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
			}
		}
		std::string str(CPUBrandString);
		return str;
	}
	
	std::vector<std::string> readFile(std::string file_path)
	{
		std::ifstream file_content(file_path.c_str());
		std::string line;
		std::vector<std::string> line_result;

		while (getline(file_content, line)) {
			if (!line.empty()) {
				line_result.push_back(line);
			}
		}
		file_content.close();
		return line_result;
	}
};


#endif //LICENCE_HELPERS_H
