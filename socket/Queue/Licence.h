#ifndef CORE_LICENCE_H
#define CORE_LICENCE_H
#include <iostream>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <chrono>

#include <Logger/Logger.h>
#include <Config/Config.h>
#include <Helpers/Helpers.h>
#include <Ini/inireader.h>

namespace Queue {
    class Licence {
    private:
        Helpers     _helpers;
        Logger      _logger;
    public:
        Licence& SetLogger(Logger logger)
        {
            _logger = logger;
            return *this;
        }

        Licence& SetConfig(Config *config)
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
                _helpers.CmdOutput("/usr/local/fpanel/core/bin/statisticsmng --init");
                _helpers.SecSleep(rand_time);
            }
        }
    };
}
#endif //CORE_LICENCE_H
