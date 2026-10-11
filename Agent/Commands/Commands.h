#pragma once
#ifndef AGENT_COMMANDS_H
#define AGENT_COMMANDS_H

#include <iostream>
#include <minwinbase.h>
#include <Helpers/Helpers.h>
#include <Logger/Logger.h>
#include <Config/Config.h>
#include <Socket/TcpClient.h>
#include <Crypto/Crypto.h>
#include <Rapidjson/document.h>
#include <Rapidjson/writer.h>
#include <Rapidjson/stringbuffer.h>
#include <boost/asio/ip/host_name.hpp>
#include <windows.h>
#include <thread>
#define INFO_BUFFER_SIZE 32767

using namespace rapidjson;

class Commands {
	private:
	Helpers *_helpers = new Helpers();
	Logger _logger;
	Config *_config;
	Crypto _crypto;
	bool _debug;
	std::string server_key = "7EDbOEyDQAdg8AjZjRgcX4KOExvmfuWP";
	std::string server_iv = "5AfcicxDnO7H6u7K";

	std::string panel_conf = "c:/fpanel/etc/panel.conf";
	std::string sql_pass_file= "c:/fpanel/etc/.sql_pass";
	std::string config_location = "c:/fpanel/etc/cloud.config.ini";
	std::string system_packages = "c:/fpanel/etc/.sys.packages";


	std::string sysInfo()
	{
		StringBuffer last_s_json;
		Writer<StringBuffer> writer(last_s_json);
		writer.StartObject();
		writer.Key("result");


		// CPU Info
		// Disk Space
		// Ram
		// OS Info
		// IP Addresses
		// Hostname
		writer.StartObject();
		std::string os_name = _helpers->GetOsName();
		writer.Key("cpu");
		std::string cpu_name = _helpers->GetProcessorName();
		_helpers->trim(cpu_name);
		writer.String(cpu_name.c_str());

		BOOL  disk_result;
		unsigned __int64 i64FreeBytesToCaller,
			i64TotalBytes,
			i64FreeBytes;
		disk_result = GetDiskFreeSpaceEx("C:",
			(PULARGE_INTEGER)&i64FreeBytesToCaller,
			(PULARGE_INTEGER)&i64TotalBytes,
			(PULARGE_INTEGER)&i64FreeBytes);

		std::string total_disk = "0";
		std::string used_disk = "0";
		if (disk_result) {
			total_disk = std::to_string(i64TotalBytes);
			used_disk = std::to_string(i64TotalBytes - i64FreeBytes);
		}

		writer.Key("disk");
		writer.StartObject();
		writer.Key("total");
		writer.String(total_disk.c_str());
		writer.Key("used");
		writer.String(used_disk.c_str());
		writer.EndObject();



		std::string hostname = boost::asio::ip::host_name();


		writer.Key("hostname");
		writer.String(hostname.c_str());

		MEMORYSTATUSEX status;
		status.dwLength = sizeof(status);
		GlobalMemoryStatusEx(&status);
		unsigned long long l_available_memory = status.ullAvailPhys;
		unsigned long long l_total_memory = status.ullTotalPhys;

		std::string total_memory = std::to_string(l_total_memory);
		std::string used_memory = std::to_string(l_total_memory - l_available_memory);

		writer.Key("memory");
		writer.StartObject();
		writer.Key("total");
		writer.String(total_memory.c_str());
		writer.Key("used");
		writer.String(used_memory.c_str());
		writer.EndObject();

		writer.Key("os");
		writer.String(os_name.c_str());

		writer.Key("default_ip");
		writer.String(_helpers->defaultIpAddress().c_str());

		writer.Key("ips");
		writer.StartArray();
		std::vector<std::string> ips = _helpers->IpAddresses();
		for (std::string i : ips) {
			writer.String(i.c_str());
		}
		writer.EndArray();
		writer.EndObject();

		writer.EndObject();
		std::string response_json = last_s_json.GetString();
		last_s_json.Clear();

		return response_json;
	}

	std::string installation(std::string command, std::string parameters)
	{
		_helpers->CreateDirectory("c:/fpanel");
		_helpers->CreateDirectory("c:/fpanel/tmp");

		if (_helpers->IsFileExists("c:/fpanel/tmp/installer.exe")) {
			_helpers->RemoveFile("c:/fpanel/tmp/installer.exe");
		}

		_logger.Debug("Download processing...");
		bool download_status = _helpers->downloadFile("repo.fpanel.net", "installer.exe", "c:/fpanel/tmp/installer.exe");
		_logger.Debug("Installer downloaded...");

		if (download_status) {
			_logger.Debug("Installation starting...");
			_logger.Debug("Running Command: %s", std::string("c:/fpanel/tmp/installer.exe --exec --fast --cloud " + parameters).c_str());
			_helpers->ExecuteWindows("c:/fpanel/tmp/installer.exe --fast --cloud " + parameters, true);
			_helpers->Write("c:/fpanel/core/bin/debug", "test");
			return R"({"result":"success"})";
		}
		return R"({"result":"failed"})";
	}

	std::string getConfigs()
	{
		StringBuffer last_s_json;
		Writer<StringBuffer> writer(last_s_json);
		writer.StartObject();
		writer.Key("result");

		std::ifstream secret_file;
		secret_file.open(panel_conf);
		std::string secret_key;
		std::string secret_iv;
		std::string panel_sql_secret;
		if (secret_file.is_open()) {
			std::string word;
			std::string first_key;

			int i = 0;
			while (secret_file >> word) {
				if (i == 0)
					secret_key = word;
				if (i == 1)
					secret_iv = word.substr(0, 16);
				if (i == 2)
					panel_sql_secret = word;
				i++;
			}
			_helpers->trim(secret_key);
			_helpers->trim(secret_iv);
			_helpers->trim(panel_sql_secret);
			secret_file.close();
		}
		writer.StartObject();
		writer.Key("key");
		writer.String(secret_key.c_str());
		writer.Key("iv");
		writer.String(secret_iv.c_str());
		writer.Key("sql_secret");
		writer.String(panel_sql_secret.c_str());

		writer.Key("system_packages");
		writer.StartObject();
		std::vector<std::string> packages_file_content = _helpers->readFile(system_packages);
		for (std::string line : packages_file_content) {
			if (!line.empty()) {
				std::vector<std::string> line_arr;
				_helpers->Split(line_arr, line, "=");
				if (line_arr.size() > 1) {
					writer.Key(line_arr[0].c_str());
					writer.String(line_arr[1].c_str());
				}
			}
		}
		writer.EndObject();

		std::string sql_pass;
		std::ifstream s_file;
		s_file.open(sql_pass_file);
		if (s_file.is_open()) {
			std::string word;
			while (s_file >> word) {
				if (!word.empty()) {
					sql_pass = word;
				}
			}
			s_file.close();
		}
		writer.Key("sql_pass");
		writer.String(sql_pass.c_str());
		writer.EndObject();

		writer.EndObject();
		std::string response_json = last_s_json.GetString();
		last_s_json.Clear();

		return response_json;
	}

	std::string reboot()
	{
		_helpers->ExecuteWindows("shutdown -t 3 -r -f", false);
		return "{\"result\":\"success\"}";
	}

	std::string initialize()
	{
		StringBuffer last_s_json;
		Writer<StringBuffer> writer(last_s_json);
		writer.StartObject();
		writer.Key("result");

		std::string command = "c:/fpanel/core/bin/adminmng.exe --init";
		std::string cmd_result = _helpers->CmdOutput(command);
		_helpers->trim(cmd_result);
		writer.String(cmd_result.c_str());

		writer.EndObject();
		std::string response_json = last_s_json.GetString();
		last_s_json.Clear();

		return response_json;
	}

	std::string setMain(std::string parameters)
	{
		_helpers->trim(parameters);
		std::string cloud_config_ini = "[socket.server]\n";
		cloud_config_ini += "address=" + parameters;
		_helpers->Write(config_location, cloud_config_ini);
		return R"({"result":"success"})";
	}

	std::string convertToCloud(std::string parameters)
	{
		StringBuffer last_s_json;
		Writer<StringBuffer> writer(last_s_json);
		writer.StartObject();
		writer.Key("result");

		std::string command = "c:/fpanel/core/bin/cloudmng.exe --convert-cloud --server-addr=" + parameters;
		std::string cmd_result = _helpers->CmdOutput(command.c_str());

		_helpers->trim(cmd_result);
		writer.String(cmd_result.c_str());

		writer.EndObject();
		std::string response_json = last_s_json.GetString();
		last_s_json.Clear();

		return response_json;
	}

public:
	Commands& SetLogger(Logger logger)
	{
		_logger = logger;
		return *this;
	}

	Commands& SetConfig(Config *config)
	{
		_config = config;
		return *this;
	}

	Commands& SetDebug(bool debug)
	{
		_debug = debug;
		return *this;
	}

	std::string Run(std::string command, const std::string& parameters)
	{
		_helpers->trim(command);
		_logger.Debug("Command: %s", command.c_str());
		_logger.Debug("Parameters: %s", parameters.c_str());

		std::string result;

		if (command == "sysinfo") {
			_logger.Debug("SystemInfo getting...");
			result = sysInfo();
		}
		else if (command == "install") {
			_logger.Debug("Installation starting...");
			result = installation(command, parameters);
		}
		else if (command == "get-config") {
			_logger.Debug("Config getting...");
			// including .sql_pass
			result = getConfigs();
		}
		else if (command == "reboot") {
			_logger.Debug("Rebooting...");
			result = reboot();
		}
		else if (command == "set-main") {
			_logger.Debug("Main server setting...");
			result = setMain(parameters);
		}
		else if (command == "init") {
			_logger.Debug("Initialize processing...");
			result = initialize();
		}
		else if (command == "convert-cloud") {
			_logger.Debug("Making a part of Cloud...");
			result = convertToCloud(parameters);
		}

		_logger.Debug("cmd_result: %s", result.c_str());
		return result;
	}
};
#endif //AGENT_COMMANDS_H
