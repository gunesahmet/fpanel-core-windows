#pragma once
#ifndef CORE_COMMANDS_H
#define CORE_COMMANDS_H
#include <iostream>
#include <Helpers/Helpers.h>
#include <Logger/Logger.h>
#include <Config/Config.h>
#include <Maps/Licence.h>
#include <Maps/Domains.h>
#include <Maps/CpuMap.h>
#include <Maps/LicenceRequests.h>
#include <Crypto/Crypto.h>
#include <Rapidjson/document.h>
#include <Rapidjson/writer.h>
#include <Rapidjson/stringbuffer.h>
#include <Socket/TcpClient.h>
#include <Cpu/Cpu.h>

using namespace rapidjson;

class Commands {
    bool debug;
    Helpers _helpers;
    Logger _logger;
    Config *_config;
    Licence *_licence_map;
    Domains *_domain_map;
	CpuMap *_cpu_map;
    LicenceRequests *_licence_requests;
    std::string licence_result = "{\"created_date\":\"none\",\"expire_date\":\"\",\"status\":0,\"domains\":1,\"clients\":1}";
    std::string default_licence_result = "{\"created_date\":\"none\",\"expire_date\":\"\",\"status\":0,\"domains\":1,\"clients\":1}";
    Crypto _crypto;
    int trying = 0;
    std::string last_request_json;
    std::string licence_server_key = "7EDbOEyDQAdg8AjZjRgcX4KOExvmfuWP";
    std::string licence_server_iv = "5AfcicxDnO7H6u7K";

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

    Commands& SetLicenceMap(Licence *licence_map)
    {
        _licence_map = licence_map;
        return *this;
    }

    Commands& SetDomainsMap(Domains *domain_map)
    {
        _domain_map = domain_map;
        return *this;
    }

	Commands& SetCpuMap(CpuMap *cpu_map)
	{
		_cpu_map = cpu_map;
		return *this;
	}

    std::string Run(std::string request)
    {
        _helpers.trim(request);
        _logger.Debug("Command running...");
        std::string cmd_result;
        if (request.compare("licencemng --check") == 0) {
            _logger.Debug("Licence checking...");
            cmd_result = licence_result;
//            std::cout << "licence_result: " << cmd_result << std::endl;
            _logger.Debug("Licence checked...");
        }
        else if (request.compare("licencemng --force-check") == 0) {
            _logger.Debug("Licence force checking...");
   //         std::string request = GenerateLicenceRequest();
			//std::cout << "Licence Request: " << request << std::endl;
   //         LicenceRequest(request);
   //         if(_licence_map->getTotalTry() > 4) {
   //             _logger.Debug("Default Result: " + default_licence_result);
   //             cmd_result = default_licence_result;
   //         }
   //         else {
                cmd_result = licence_result;
   //             _logger.Debug("Licence Request Result: " + licence_result);
   //         }
        }
        else {
			//int cpu_usage = _cpu_map->getCpuUsage();
            _logger.Debug("System Command running...");
            std::string command = _config->getValue("root_bin_path") + request;
			_helpers.trim(command);
			//command = command + " --cpu=" + std::to_string(cpu_usage); // For windows
            _logger.Debug("command: " + command);
            //cmd_result = _helpers.CmdOutput(command.c_str());
            _logger.Debug("cmd_result: " + cmd_result);
        }
        return cmd_result;
    }

    void LicenceRequest(std::string request)
    {
        TcpClient *_tcp_client = new TcpClient();
        request = _crypto.Encrypt(request, licence_server_key, licence_server_iv);
        _tcp_client->SetLogger(_logger);
        _tcp_client->SetConfig(_config);
        _tcp_client->SetServer("licence.fpanel.net", 8871);
        bool connect_status = _tcp_client->Connect(request);
        if(connect_status) {
            bool send_status = _tcp_client->Send(request);
            if(send_status) {
                _tcp_client->Read();
                std::string response = _tcp_client->Response();
                std::string decoded_response = _crypto.Decrypt(response, licence_server_key, licence_server_iv);
                _logger.Debug("Decoded Result: " + decoded_response);
                _logger.Debug(decoded_response);
                licence_result = decoded_response;
                _tcp_client->Close();
				delete _tcp_client;
            }
        }
		else {
			Sleep(5000);
		}
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
        writer.Int64(2);

        writer.Key("key");
        writer.String(licence_key.c_str());

        writer.Key("ip_addr");
        writer.String(default_ip.c_str());

        writer.EndObject();

        last_request_json = last_s_json.GetString();
//        std::string enc_cmd_result = _crypto.Encrypt(last_request_json, licence_server_key, licence_server_iv);
        last_s_json.Clear();
        return last_request_json;
    }

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
			INIReader reader(_config->getValue("htdocs_dir") + "config/config.ini");
			if (reader.ParseError() < 0) {
				_logger.Error("Can't load 'config.ini'");
				_helpers.SecSleep(5);
				continue;
			}

			int is_random = reader.GetInteger("statistics", "random", 0);
			std::string calculate_time = "Undefined";
			if (is_random == 0) {
				//                _logger.Debug("Calculate statistics isn't randomized");
				calculate_time = reader.Get("statistics", "time", "Undefined");

				if ((calculate_time.compare("Undefined") == 0)) {
					calculate_time = "00:00";
				}


				//                _logger.Debug("Calculation is will doing at " + calculate_time);
				char delimiter = ':';
				std::vector<std::string> ctime = _helpers.strSplit(calculate_time, delimiter);

				std::string search_hour = ctime[0];
				std::string search_min = ctime[1];
				_helpers.ReplaceWithString(search_hour, "\"", "");
				_helpers.ReplaceWithString(search_min, "\"", "");

				//                if(_logger.debug) {
				//                    std::cout << "Calculation Search Hour: " << search_hour << std::endl;
				//                    std::cout << "Calculation Search Min: " << search_min << std::endl;
				//                    std::cout << "Calculation Current Hour: " << hour << std::endl;
				//                    std::cout << "Calculation Current Min: " << minute << std::endl;
				//                }

				if ((search_hour.compare(hour) == 0) && (search_min.compare(minute) == 0)) {
					_logger.Debug("Starting time defined calculation...");
					_helpers.CmdOutput(_config->getValue("root_bin_path") + "statisticsmng --init");
					_helpers.SecSleep(60);
				}
				_helpers.SecSleep(5);
			}
			else {
				_logger.Debug("Starting randomized time calculation...");
				_helpers.CmdOutput(_config->getValue("root_bin_path") + "statisticsmng --init");
				_helpers.SecSleep(rand_time);
			}
		}
	}

	void initUpdater()
	{
		int next_time = 60;
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

			INIReader reader(_config->getValue("htdocs_dir") + "config/config.ini");

			if (reader.ParseError() < 0) {
				_logger.Error("Can't load 'config.ini'");
				_helpers.SecSleep(5);
				continue;
			}

			int is_random = reader.GetInteger("update", "random", 0);
			int is_auto = reader.GetInteger("update", "auto", 0);
			std::string update_time = "Undefined";
			//            std::cout << "is_random: " << is_random << std::endl;

			if (is_random == 0) {
				//                _logger.Debug("Update isn't randomized");
				update_time = reader.Get("update", "time", "Undefined");
			}

			if ((update_time.compare("Undefined") == 0)) {
				update_time = "02:00";
			}

			//            _logger.Debug("Update is will doing at " + update_time);

			char delimiter = ':';
			std::vector<std::string> utime = _helpers.strSplit(update_time, delimiter);

			std::string search_hour = utime[0];
			std::string search_min = utime[1];
			_helpers.ReplaceWithString(search_hour, "\"", "");
			_helpers.ReplaceWithString(search_min, "\"", "");

			if ((search_hour.compare(hour) == 0) && (search_min.compare(minute) == 0)) {
				_logger.Debug("Starting update...");
				if (is_auto) {
					_logger.Debug("Update is auto");
					_helpers.CmdOutput(_config->getValue("root_bin_path") + "updatemng --init");
				}
				else {
					_logger.Debug("Update isn't auto");
					_helpers.CmdOutput(_config->getValue("root_bin_path") + "updatemng --init");
				}
				_helpers.SecSleep(next_time);
			}
			_helpers.SecSleep(5);
		}
	}

	void getCpuUsage()
	{
		CPU cpu;
		int sys;
		int process;
		TKTime upTime;
		while (true) {
			try {
				Sleep(300);
				process = cpu.GetUsage(&sys, &upTime);
				_cpu_map->addCpuUsage(process);
			}
			catch (...) {
			}
		}
	}

    void BackendLoops()
    {
        //std::thread tlq(&Commands::licenceRequestLoop, this);
        //tlq.detach();
        //std::thread tcs(&Commands::calculateStats, this);
        //tcs.detach();
        //std::thread tiu(&Commands::initUpdater, this);
        //tiu.detach();
		std::thread tic(&Commands::getCpuUsage, this);
		tic.detach();
    }

};
#endif //CORE_COMMANDS_H
