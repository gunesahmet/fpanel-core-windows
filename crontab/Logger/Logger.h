#pragma once
#ifndef CRONTAB_LOGGER_H
#define CRONTAB_LOGGER_H

#include <iostream>
#include "../plog/Log.h"
#include "../plog/Appenders/ConsoleAppender.h"

class Logger {
    enum {
        SecondLog = 1,
        MaxLogFileSize = 104857600 // 100MB
    };
public:
    static void Info(const std::string &message)
    {
        LOGI << message;
    }

	static void Error(const std::string &message)
    {
        LOGE << message;
    }

	static void Debug(const std::string &message)
    {
		bool debug = false;
		try {
			struct stat buffer;
			debug = (stat("c:\\fpanel\\core\\bin\\debug", &buffer) == 0);;
		}
		catch (...) {
		}

        if (debug) {
            std::cout << message << std::endl;
            LOGD << message;
        }
    }
};


#endif //CRONTAB_LOGGER_H
