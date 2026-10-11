#pragma once
#ifndef FPANEL_CORE_LOGGER_H
#define FPANEL_CORE_LOGGER_H

#include <iostream>
#include <plog/Log.h>
#include <plog/Appenders/ConsoleAppender.h>

class Logger {
    enum {
        SecondLog = 1,
        MaxLogFileSize = 104857600 // 100MB
    };
public:
    bool debug = false;

    void Info(const std::string &message)
    {
        if (debug) {
            std::cout << message << std::endl;
        }
        LOGI << message;
    }

    void Error(const std::string &message)
    {
        if (debug) {
            std::cout << message << std::endl;
        }
        LOGE << message;
    }

    void Debug(const std::string &message)
    {
        if (debug) {
            std::cout << message << std::endl;
            LOGD << message;
        }
    }
};


#endif //FPANEL_CORE_LOGGER_H
