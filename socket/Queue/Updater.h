#ifndef CORE_UPDATER_H
#define CORE_UPDATER_H
#include <iostream>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <chrono>

#include <Logger/Logger.h>
#include <Config/Config.h>
#include <Helpers/Helpers.h>
#include <Ini/inireader.h>

namespace Queue {
    class Updater {
    private:
        Helpers     _helpers;
        Config      *_config;
        Logger      _logger;
    public:
        Updater& SetLogger(Logger logger)
        {
            _logger = logger;
            return *this;
        }

        Updater& SetConfig(Config *config)
        {
            _config = config;
            return *this;
        }

        void Start()
        {
            int min_time = 21600;
            int max_time = 32400;
            while(true) {
                int rand_time;
                rand_time = rand()%(max_time-min_time + 1) + min_time;
                INIReader reader("/usr/local/fpanel/httpd/htdocs/config/config.ini");
                if (reader.ParseError() < 0) {
                    _logger.Error("Can't load 'config.ini'");
                }
                int next_time = reader.GetInteger("statistics", "range", -1);
                if(next_time == -1) {
                    if(next_time > 3600)
                        rand_time = next_time;
                }

                _helpers.CmdOutput("/usr/local/fpanel/core/bin/statisticsmng --init");
                _helpers.SecSleep(rand_time);
            }
        }
    };
}
#endif //CORE_UPDATER_H
