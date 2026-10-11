#ifndef CORE_WEBSTATISTICS_H
#define CORE_WEBSTATISTICS_H
#include <iostream>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <chrono>

#include <Logger/Logger.h>
#include <Config/Config.h>
#include <Helpers/Helpers.h>
#include <Ini/inireader.h>

namespace Queue {
    class WebStatistics {
    private:
        Helpers     _helpers;
        Config      *_config;
        Logger      _logger;
    public:
        WebStatistics& SetLogger(Logger logger)
        {
            _logger = logger;
            return *this;
        }

        WebStatistics& SetConfig(Config *config)
        {
            _config = config;
            return *this;
        }

        void Start()
        {
            std::string current_fpanel_directory = "/usr/local/fpanel/";
            int next_time = 60;
            while (1) {
                time_t t = time(0);   // get time now
                struct tm * now = localtime( & t );
                std::string hour = std::to_string(now->tm_hour);
                int min = now->tm_min;
                std::string minute = std::to_string(now->tm_min);
                if(min < 10)
                    minute = "0" + minute;

                INIReader reader(current_fpanel_directory + "/httpd/htdocs/config/config.ini");

                if (reader.ParseError() < 0) {
                    _logger.Error("Can't load 'config.ini'\n");
                    _helpers.SecSleep(next_time);
                    continue;
                }
                int is_random = reader.GetInteger("update", "random",0);
                std::string update_time = "Undefined";

                if(is_random != 0) {
                    update_time = reader.Get("update", "time","Undefined");
                }

                if((update_time.compare("Undefined") == 0)) {
                    update_time = "02:00";
                }

                std::vector<std::string> utime = _helpers.Split(update_time, ':');
                std::string search_hour = utime[0];
                std::string search_min = utime[1];
                _helpers.ReplaceWithString(search_hour, "\"", "");
                _helpers.ReplaceWithString(search_min, "\"", "");

                _helpers.Println("Update Search Hour: " + search_hour);
                _helpers.Println("Update Search Min: " + search_min);
                _helpers.Println("Update Current Hour: " + hour);
                _helpers.Println("Update Current Min: " + minute);


                if((search_hour.compare(hour) == 0) && (search_min.compare(minute) == 0)) {
                    _helpers.Println("Starting update...");
                    std::string statisticsmng = current_fpanel_directory + "/core/bin/updatemng.exe --init";
                    _helpers.CmdOutput(statisticsmng.c_str());
                    _helpers.SecSleep(next_time);
                }
            }
        }
    };
}
#endif //CORE_WEBSTATISTICS_H
