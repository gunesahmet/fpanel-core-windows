#pragma once
#ifndef CORE_COMMANDS_H
#define CORE_COMMANDS_H

#include <iostream>
#include <minwinbase.h>
#include <Helpers/Helpers.h>
#include <Logger/Logger.h>
#include <Config/Config.h>
#include <Maps/Licence.h>
#include <Maps/Domains.h>
#include <Maps/LicenceRequests.h>
#include <Socket/TcpClient.h>
#include <Crypto/Crypto.h>
#include <Rapidjson/document.h>
#include <Rapidjson/writer.h>
#include <Rapidjson/stringbuffer.h>

extern FILETIME last_idle, last_kernel, last_user;
FILETIME last_idle;
FILETIME last_kernel;
FILETIME last_user;
using namespace rapidjson;

class Commands {
	bool debug;
	Helpers _helpers;
	Logger _logger;
	Config *_config;
	Domains *_domain_map;
	Licence *_licence_map;
	LicenceRequests *_licence_requests;
	std::string default_result = "{\"created_date\":\"none\",\"expire_date\":\"\",\"status\":0,\"domains\":10,\"clients\":10}";
	Crypto _crypto;
	int trying = 0;
	std::string last_request_json;
	std::string licence_server_key = "7EDbOEyDQAdg8AjZjRgcX4KOExvmfuWP";
	std::string licence_server_iv = "5AfcicxDnO7H6u7K";
	bool statistics_runnig = false;
	bool update_runnig = false;
	enum request_types { SERVER_REGISTER, LICENCE_REQUEST };

	struct cpu_info {
		int load;
	};

	struct cpu_info get_cpu_info()
	{
		struct cpu_info cpu_info;

		FILETIME idle, kernel, user;
		GetSystemTimes(&idle, &kernel, &user);

		ULARGE_INTEGER ulidle, ulkernel, uluser;
		ULARGE_INTEGER ullast_idle, ullast_kernel, ullast_user;

		ulidle.HighPart = idle.dwHighDateTime;
		ulidle.LowPart = idle.dwLowDateTime;

		ulkernel.HighPart = kernel.dwHighDateTime;
		ulkernel.LowPart = kernel.dwLowDateTime;

		uluser.HighPart = user.dwHighDateTime;
		uluser.LowPart = user.dwLowDateTime;

		ullast_idle.HighPart = last_idle.dwHighDateTime;
		ullast_idle.LowPart = last_idle.dwLowDateTime;

		ullast_kernel.HighPart = last_kernel.dwHighDateTime;
		ullast_kernel.LowPart = last_kernel.dwLowDateTime;

		ullast_user.HighPart = last_user.dwHighDateTime;
		ullast_user.LowPart = last_user.dwLowDateTime;

		ULARGE_INTEGER didle, dkernel, duser;

		didle.QuadPart = ulidle.QuadPart - ullast_idle.QuadPart;
		dkernel.QuadPart = ulkernel.QuadPart - ullast_kernel.QuadPart;
		duser.QuadPart = uluser.QuadPart - ullast_user.QuadPart;

		ULARGE_INTEGER dsystem;
		dsystem.QuadPart = dkernel.QuadPart + duser.QuadPart;

		last_idle = idle;
		last_user = user;
		last_kernel = kernel;

		if (dsystem.QuadPart == 0) {
			cpu_info.load = 0;
		}
		else {
			cpu_info.load = (int)((dsystem.QuadPart - didle.QuadPart) * 100 / dsystem.QuadPart);
		}

		return cpu_info;
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

	Commands& SetLicenceRequests(LicenceRequests *licence_requests)
	{
		_licence_requests = licence_requests;
		return *this;
	}

	Commands& SetLicenceMap(Licence *licence)
	{
		_licence_map = licence;
		return *this;
	}

	Commands& SetDomainsMap(Domains *domain_map)
	{
		_domain_map = domain_map;
		return *this;
	}

	void LicenceResultSerialize(Writer<StringBuffer>& writer, const LicenceRequests::licence_result& s) {
		writer.StartObject();

		writer.Key("created_date");
		writer.String(s.created_date.c_str());
		writer.Key("expire_date");
		writer.String(s.expire_date.c_str());
		writer.Key("licence_hash");
		writer.String(s.licence_hash.c_str());
		writer.Key("licence_key");
		writer.String(s.licence_key.c_str());
		writer.Key("current_date");
		writer.String(s.current_date.c_str());
		writer.Key("status");
		writer.String(s.status.c_str());
		writer.Key("domains");
		writer.String(s.domains.c_str());
		writer.Key("clients");
		writer.String(s.clients.c_str());

		writer.EndObject();
	}

	std::string Run(std::string request)
	{
		_helpers.trim(request);
		_logger.Debug("Command running...");
		std::string cmd_result;
		if (request.compare("licencemng --check") == 0) {
			LicenceRequests::licence_result licence_result = _licence_requests->getLast();
			StringBuffer sb;
			Writer<StringBuffer> writer(sb);
			LicenceResultSerialize(writer, licence_result);
			cmd_result = sb.GetString();
			sb.Clear();
			_logger.Debug("Licence Result: " + cmd_result);
		}
		else if (request.compare("licencemng --force-check") == 0) {
			std::string request = GenerateLicenceRequest();
			std::cout << "Licence Force Check Request: " << request << std::endl;
			LicenceRequest(request);

			if (_licence_map->getTotalTry() > 4) {
				_logger.Debug("Default Result: " + default_result);
				cmd_result = default_result;
			}
			else {
				LicenceRequests::licence_result licence_result = _licence_requests->getLast();
				StringBuffer sb;
				Writer<StringBuffer> writer(sb);
				LicenceResultSerialize(writer, licence_result);
				cmd_result = sb.GetString();
				sb.Clear();
				_logger.Debug("Licence Request Result: " + cmd_result);
			}
		}
		else {
			_logger.Debug("Request: " + request);
			struct cpu_info cpu_info = get_cpu_info();
			int cpu_usage = cpu_info.load;
			std::string temp_file = "c:\\fpanel\\tmp\\" + std::string(_helpers.GenerateUniqueKey()) + ".bat";
			std::string cmd;
			if (request.length() > 2047) {
				std::vector<std::string> request_arr;
				_helpers.Split(request_arr, request, " ");
				std::string new_str;
				if (strstr(request_arr[0].c_str(), "sslmng")) {
					for (int i = 0; i < request_arr.size(); i++) {
						new_str.append(request_arr[i]);
						new_str.append(" ");
					}
					_helpers.ReplaceWithString(new_str, "\"", "");
					cmd = _config->getValue("root_bin_path") + new_str;
					_helpers.Write(temp_file, cmd);
					cmd_result = _helpers.CmdOutput(temp_file);
					//if (!debug)
					//_helpers.RemoveFile(temp_file);
				}
				else if (strstr(request_arr[0].c_str(), "filemng")) {
					std::string temp_file = "c:\\fpanel\\tmp\\" + std::string(_helpers.GenerateUniqueKey()) + ".tmp";
					for (int i = 0; i < request_arr.size(); i++) {
						if (strstr(request_arr[i].c_str(), "--file-path=")) {
							std::string file_content = request_arr[i];
							_helpers.ReplaceWithString(file_content, "--file-path=", "");
							_helpers.Write(temp_file, file_content);
							new_str.append("--file-path=" + temp_file);
						}
						else {
							new_str.append(request_arr[i]);
						}
						new_str.append(" ");
					}
					cmd = _config->getValue("root_bin_path") + new_str;
					cmd_result = _helpers.CmdOutput(cmd);
					//if(!debug)
					//_helpers.RemoveFile(temp_file);
				}
				else if (strstr(request_arr[0].c_str(), "backupmng")) {
					cmd = _config->getValue("root_bin_path") + request;
					_helpers.trim(cmd);
					cmd = cmd + " --cpu=" + std::to_string(cpu_usage); // For windows
					cmd_result = _helpers.CmdOutput(cmd.c_str());
				}
			}
			else {
				cmd = _config->getValue("root_bin_path") + request;
				_helpers.trim(cmd);
				cmd = cmd + " --cpu=" + std::to_string(cpu_usage); // For windows

				cmd_result = _helpers.CmdOutput(cmd.c_str());
			}
			_logger.Debug("cmd_result: " + cmd_result);
		}
		return cmd_result;
	}

	std::string LicenceRequest(std::string request)
	{
		std::string decoded_response;
		TcpClient *_tcp_client = new TcpClient();
		request = _crypto.Encrypt(request, licence_server_key, licence_server_iv);
_tcp_client->SetLogger(_logger);
_tcp_client->SetConfig(_config);
_tcp_client->SetServer("licence.fpanel.net", 8871);
bool connect_status = _tcp_client->Connect(request);
if (connect_status) {
	bool send_status = _tcp_client->Send(request);
	if (send_status) {
		_tcp_client->RecvResponse();
		std::string response = _tcp_client->Response();
		decoded_response = _crypto.Decrypt(response, licence_server_key, licence_server_iv);
		_logger.Debug("Decoded Result: " + decoded_response);
		_tcp_client->Close();

		Document d;
		const char *json = decoded_response.c_str();
		if (!d.Parse(json).HasParseError()) {
			if (d.HasMember("licence_hash")) {
				_licence_requests->add(_licence_requests->MakeResult(
					d["created_date"].GetString(),
					d["expire_date"].GetString(),
					d["licence_key"].GetString(),
					d["licence_hash"].GetString(),
					d["current_date"].GetString(),
					d["status"].GetString(),
					d["domains"].GetString(),
					d["clients"].GetString()
				));
			}
		}

		delete _tcp_client;
	}
}
else {
	_logger.Error("Failed connect to licence server");
}
return decoded_response;
	}

	std::string GenerateLicenceRequest()
	{
		std::string default_ip = _helpers.CmdOutput(_config->getValue("root_bin_path") + "networkmng --show-default-ip");
		_helpers.trim(default_ip);
		std::string licence_key = _helpers.createLicenceKey(default_ip, _config->getValue("secret.key"), _config->getValue("secret.iv"));
		StringBuffer last_s_json;
		Writer<StringBuffer> writer(last_s_json);

		writer.StartObject();

		writer.Key("type");
		writer.Int64(request_types::LICENCE_REQUEST);

		writer.Key("hash");
		writer.String(licence_key.c_str());

		writer.Key("key");
		writer.String(_config->getValue("secret.key").c_str());

		writer.Key("iv");
		writer.String(_config->getValue("secret.iv").c_str());

		writer.Key("ip_addr");
		writer.String(default_ip.c_str());

		writer.EndObject();

		last_request_json = last_s_json.GetString();
		//        std::string enc_cmd_result = _crypto.Encrypt(last_request_json, licence_server_key, licence_server_iv);
		last_s_json.Clear();
		return last_request_json;
	}

	/**
	* cloud parts and standalones
	*/
	void calculateStats()
	{
		// int min_time = 2;
		// int max_time = 3;
		int min_time = 21600;
		int max_time = 32400;
		while (1) {
			time_t t = time(0);   // get time now
			struct tm * now = localtime(&t);
			std::string hour = std::to_string(now->tm_hour);
			if (hour.length() == 1) {
				hour = "0" + hour;
			}

			int min = now->tm_min;
			std::string minute = std::to_string(now->tm_min);
			if (min < 10)
				minute = "0" + minute;

			int rand_time;
			rand_time = rand() % (max_time - min_time + 1) + min_time;
			INIReader reader(_config->getValue("panel_dir") + "etc/system.conf");
			if (reader.ParseError() < 0) {
				_logger.Error("Can't load 'system.conf'");
				_helpers.SecSleep(5);
				continue;
			}

			std::string server_address;
			if (_helpers.IsFileExists(_config->getValue("panel_dir") + "etc/cloud.config.ini")) {
				INIReader reader_cloud(_config->getValue("panel_dir") + "etc/cloud.config.ini");
				if (reader_cloud.ParseError() < 0) {
					_logger.Error("Can't load 'system.conf'");
					_helpers.SecSleep(5);
					continue;
				}
				server_address = reader_cloud.Get("socket.server", "address", "127.0.0.1");
			}
			else {
				server_address = reader.Get("server", "address", "127.0.0.1");
			}

			int is_random = reader.GetInteger("statistics", "random", 0);
			std::string calculate_time = "Undefined";
			if (is_random == 0) {
				calculate_time = reader.Get("statistics", "time", "Undefined");

				if ((calculate_time.compare("Undefined") == 0)) {
					calculate_time = "00:00";
				}

				char delimiter = ':';
				std::vector<std::string> ctime = _helpers.strSplit(calculate_time, delimiter);

				std::string search_hour = ctime[0];
				std::string search_min = ctime[1];
				_helpers.ReplaceWithString(search_hour, "\"", "");
				_helpers.ReplaceWithString(search_min, "\"", "");

				if ((search_hour.compare(hour) == 0) && (search_min.compare(minute) == 0)) {
					while (true) {
						if (update_runnig)
							_helpers.SecSleep(1);
						else
							break;
					}
					_logger.Debug("Starting time defined calculation...");
					statistics_runnig = true;
					_helpers.CmdOutput(_config->getValue("root_bin_path") + "statisticsmng --init --server-addr=" + server_address);
					_helpers.SecSleep(60);
					statistics_runnig = false;
				}
				_helpers.SecSleep(5);
			}
			else {
				statistics_runnig = true;
				_logger.Debug("Starting randomized time calculation...");
				_helpers.CmdOutput(_config->getValue("root_bin_path") + "statisticsmng --init --server-addr=" + server_address);
				_helpers.SecSleep(rand_time);
				statistics_runnig = false;
			}
		}
	}

	/**
	* all
	*/
	void initUpdater()
	{
		while (1) {
			time_t t = time(0);   // get time now
			struct tm * now = localtime(&t);
			std::string hour = std::to_string(now->tm_hour);

			if (hour.length() == 1) {
				hour = "0" + hour;
			}

			int min = now->tm_min;
			std::string minute = std::to_string(now->tm_min);
			if (min < 10)
				minute = "0" + minute;

			INIReader reader(_config->getValue("panel_dir") + "etc/system.conf");

			if (reader.ParseError() < 0) {
				_logger.Error("Can't load 'system.conf'");
				_helpers.SecSleep(5);
				continue;
			}

			std::string server_address = reader.Get("server", "address", "127.0.0.1");
			std::string update_time = "Undefined";

			update_time = reader.Get("update", "time", "Undefined");
			if ((update_time.compare("Undefined") == 0)) {
				update_time = "01:00";
			}
			char delimiter = ':';
			std::vector<std::string> utime = _helpers.strSplit(update_time, delimiter);

			std::string search_hour = utime[0];
			std::string search_min = utime[1];
			_helpers.ReplaceWithString(search_hour, "\"", "");
			_helpers.ReplaceWithString(search_min, "\"", "");

			if ((search_hour.compare(hour) == 0) && (search_min.compare(minute) == 0)) {
				while (true) {
					if (statistics_runnig)
						_helpers.SecSleep(1);
					else
						break;
				}
				update_runnig = true;
				_logger.Debug("Starting update...");
				_helpers.CmdOutput(_config->getValue("root_bin_path") + "updatemng --init --server-addr=" + server_address);
				update_runnig = false;
				_helpers.SecSleep(60);
			}
			else {
				_helpers.SecSleep(1);
			}
		}
	}

	/**
	* cloud parts and standalones
	*/
	void licenceRequestLoop()
	{
		int min_time = 21600;
		int max_time = 43200;

		std::string request = GenerateLicenceRequest();

		while (1) {
			int rand_time = rand() % (max_time - min_time + 1) + min_time;
			LicenceRequest(request);
			_helpers.SecSleep(rand_time);
		}
	}

	void BackendLoops()
	{
		std::string is_cloud= _config->getValue("cloud");
		std::string is_main = _config->getValue("is_main");

		if (is_cloud.compare("1") == 0) {
			if (is_main.compare("0") == 0) {
				std::thread tcs(&Commands::calculateStats, this);
				tcs.detach();
			}
		}
		else {
			std::thread tcs(&Commands::calculateStats, this);
			tcs.detach();
		}


		std::thread til(&Commands::licenceRequestLoop, this);
		til.detach();

		std::thread tiu(&Commands::initUpdater, this);
		tiu.detach();
	}

};
#endif //CORE_COMMANDS_H
