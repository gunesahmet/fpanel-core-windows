#include "Logger.h"
#include <stdio.h>
#include <stdarg.h>

static std::string systemDirectory;
static std::string error_logs;
static std::string info_logs;
static std::string debug_logs;
static std::string tmp_path;

static bool ifDirectoryExists(boost::filesystem::path p);

void FileLogger::InitializeInstanceFields()
{
	char *sysDrive = getenv("SystemDrive");
	if (sysDrive == NULL) {
		systemDirectory = "c:";
	}
	else {
		systemDirectory = std::string(sysDrive);
	}

	error_logs = systemDirectory + "\\fpanel\\var\\log\\error.log";
	info_logs = systemDirectory + "\\fpanel\\var\\log\\info.log";
	debug_logs = systemDirectory + "\\fpanel\\var\\log\\debug.log";
	tmp_path = systemDirectory + "\\fpanel\\var\\log";

	try {
		if (ifDirectoryExists(boost::filesystem::path(tmp_path)) == false) {
			//std::cout << tmp_path << std::endl;
			boost::filesystem::create_directories(tmp_path);
		}
	}
	catch (std::exception &e) {
		errorLog(e.what(), 1003);
	}
}

static bool ifDirectoryExists(boost::filesystem::path p)
{
	if (boost::filesystem::exists(p)) {
		if (is_directory(p))
			return true;
		else
			return false;
	}
	return false;
}

static const std::string currentDateTime()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
	std::string current_date(buf);
	return current_date;
}

static bool writeLog(std::string log, std::string log_files) {
	std::string current_time = currentDateTime();
	std::ofstream log_file;
	log_file.open(log_files, std::ios_base::app);
	log_file << log << std::endl;
	log_file.close();
	return true;
}

void FileLogger::errorLog(const std::string message, int code)
{
	std::string current_date = currentDateTime();
	writeLog("["+ current_date +"] [ERROR]["+ std::to_string(code) +"]\t"+ message, error_logs);
}

void FileLogger::infoLog(const std::string message, int code)
{
	std::string current_date = currentDateTime();
	writeLog("[" + current_date + "] [INFO][" + std::to_string(code) + "]\t" + message, info_logs);
}

void FileLogger::debugLog(const std::string message)
{
	if (boost::filesystem::exists(".debug"))
	{
		std::string current_date = currentDateTime();
		writeLog("[" + current_date + "] [DEBUG]\t" + message,debug_logs);
	}
}
