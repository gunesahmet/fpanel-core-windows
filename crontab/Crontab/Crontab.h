#pragma once
#ifndef CRONTAB_H
#define CRONTAB_H

#include <iostream>
#include <map>
#include <shared_mutex>
#include <Ini/inireader.h>
#include <Crypto/Crypto.h>
#include <Helpers/Helpers.h>


class Config {
private:
	std::string config_location = "c:/fpanel/httpd/htdocs/config/config.ini";
	std::map<std::string, std::string> configurations;
	Helpers _helpers;
	std::shared_mutex config_mutex;

public:
	Config()
	{
		configurations = std::map<std::string, std::string>();
	}

	~Config()
	{
		configurations.clear();
	}

	void setConfigFile(std::string &config_file)
	{
		config_location = config_file;
	}

	void init()
	{
		std::unique_lock<std::shared_mutex> slk(config_mutex);
		Crypto crypto;

		INIReader reader(config_location);
		if (reader.ParseError() < 0) {
			std::cout << "Config file not loaded!!!" << std::endl;
			return;
		}
		configurations.empty();
		configurations.clear();


		std::string secret_key;
		std::string secret_iv;

		std::ifstream file;
		file.open("c:\\fpanel\\etc\\panel.conf");
		if (!file.is_open()) {
			std::cout << "Config can't open" << std::endl;
			exit(EXIT_FAILURE);
		}

		std::string word;
		std::string first_key;

		int i = 0;
		while (file >> word) {
			if (i == 0)
				secret_key = word;
			if (i == 1)
				secret_iv = word.substr(0, 16);
			i++;
		}
		_helpers.trim(secret_key);
		_helpers.trim(secret_iv);
		configurations.insert({ "secret.key", secret_key });
		configurations.insert({ "secret.iv", secret_iv });

		// TCP Sockets
		std::string sql_socket_server_address = reader.Get("socket.server", "address", "127.0.0.1");
		std::string sql_socket_server_port = reader.Get("socket.server", "port", "8889");
		//_logger.Debug("Port  : " + sql_socket_server_port);
		std::string panel_dir = "c:\\fpanel\\";

		configurations.insert({ "socket.server.address", sql_socket_server_address });
		configurations.insert({ "socket.server.port", sql_socket_server_port });
		configurations.insert({ "panel_dir", panel_dir });
		configurations.insert({ "root_bin_path", panel_dir + "core\\bin\\" });
		configurations.insert({ "root_sbin_path", panel_dir + "core\\sbin\\" });
		configurations.insert({ "httpd_dir", panel_dir + "httpd\\" });
		configurations.insert({ "htdocs_dir", panel_dir + "httpd\\htdocs\\" });

	}

	std::map<std::string, std::string> getConfigs()
	{
		std::shared_lock<std::shared_mutex> slk(config_mutex);

		std::map<std::string, std::string> result = configurations;
		return result;
	}

	std::string getValue(std::string key)
	{
		std::shared_lock<std::shared_mutex> slk(config_mutex);

		std::string result;
		if (configurations.rbegin() != configurations.rend()) {
			if (configurations.size() > 0) {
				result = configurations.find(key)->second;
			}
		}
		return result;
	}
};


#endif //FPANEL_CORE_CONFIG_H
