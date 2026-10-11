#pragma once
#ifndef LICENCE_CONFIG_H
#define LICENCE_CONFIG_H

#include <iostream>
#include <map>
#include <shared_mutex>
#include <Ini/inireader.h>
#include <Crypto/Crypto.h>
#include <Helpers/Helpers.h>


class Config {
private:
    std::string cloud_config_location = "c:\\fpanel\\etc\\cloud.config.ini";
	std::string panel_config_file = "c:\\fpanel\\etc\\panel.conf";
	std::string panel_dir = "c:\\fpanel\\";
    std::map<std::string, std::string> configurations;
	Helpers _helpers;
	Logger _logger;
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

	void setLogger(Logger logger)
	{
		_logger = logger;
	}

    void setConfigFile(std::string &config_file)
    {
		cloud_config_location = config_file;
    }

    void init()
    {
		std::unique_lock<std::shared_mutex> slk(config_mutex);
		configurations.empty();
		configurations.clear();

		if (!_helpers.IsFileExists(panel_config_file)) {
			_logger.Error("System Config not found");
			exit(1);
		}

		std::string main_ip = _helpers.CmdOutput(panel_dir + "core\\bin\\networkmng.exe --show-default-ip");
		_helpers.trim(main_ip);

		Crypto crypto;

		INIReader cloud_reader(cloud_config_location);
		std::string allowed_ip = "127.0.0.1";
		std::string default_server_ip = "127.0.0.1";
		std::string default_server_port = "8889";
		if (cloud_reader.ParseError() < 0) {
			configurations.insert({ "cloud", "0" });
			configurations.insert({ "is_main", "1" });
		}
		else {
			allowed_ip = cloud_reader.Get("socket.server", "address", "127.0.0.1");
			_helpers.trim(allowed_ip);
			default_server_ip = "0.0.0.0";
			default_server_port = "8889";

			configurations.insert({ "cloud", "1" });
			if (main_ip.compare(allowed_ip) == 0) {
				configurations.insert({ "is_main", "1" });
			}
			else {
				configurations.insert({ "is_main", "0" });
			}
		}




        std::string secret_key;
        std::string secret_iv;

        std::ifstream file;
        file.open (panel_config_file);
        if (!file.is_open()) {
            std::cout << "Config can't open" << std::endl;
            exit (EXIT_FAILURE);
        }

        std::string word;
        std::string first_key;

        int i = 0;
        while (file >> word) {
            if(i == 0)
                secret_key = word;
            if(i == 1)
                secret_iv = word.substr(0, 16);
            i++;
        }
        _helpers.trim(secret_key);
        _helpers.trim(secret_iv);
        configurations.insert({ "secret.key", secret_key });
        configurations.insert({ "secret.iv", secret_iv });

        // TCP Sockets

		configurations.insert({ "socket.allowed.address", allowed_ip });
		configurations.insert({ "socket.server.address", default_server_ip });
		configurations.insert({ "socket.server.port", default_server_port });

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