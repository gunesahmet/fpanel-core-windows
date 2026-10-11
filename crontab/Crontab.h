#pragma once
#ifndef CRONTAB_H
#define CRONTAB_H

#include <iostream>
#include <map>
#include <shared_mutex>
#include <Ini/inireader.h>
#include <Crypto/Crypto.h>
#include <Helpers/Helpers.h>
#include <Logger/Logger.h>

class Crontab {
private:
	std::string crontab_location = "c:/fpanel/etc/crontabs";
	Helpers *helpers;
public:
	Crontab()
	{
		Crypto crypto;
		helpers = new Helpers();

		INIReader reader(crontab_location);
		if (reader.ParseError() < 0) {
			Logger::Error("Cron file not loaded!!!");
			return;
		}

		std::string secret_key;
		std::string secret_iv;

		std::ifstream file;
		file.open(crontab_location);
		if (!file.is_open()) {
			Logger::Error("Cron file not loaded!!!");
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
		helpers->trim(secret_key);
		helpers->trim(secret_iv);
	}

	~Crontab()
	{
	}

	void reload()
	{

	}

	void remove()
	{

	}

	void add()
	{

	}

	void createJob()
	{
		while (true) {

			helpers->MilliSleep(1000);
		}
	}

	void start()
	{
		while(true) {
			//helpers->executeForWindows();

			helpers->MilliSleep(1000);
		}
	}

};


#endif //CRONTAB_H
