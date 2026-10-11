#pragma once
#ifndef FPANEL_CORE_LICENCE_H
#define FPANEL_CORE_LICENCE_H

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include "../tsl/ordered_map.h"

class Licence {
    tsl::ordered_map<std::string, int> licence;
    std::mutex licence_mutex;
public:
    Licence() {
        licence = tsl::ordered_map<std::string, int>();
    }

    void add(std::string created_date)
    {
        std::lock_guard<std::mutex> lockGuard(licence_mutex);
        licence.insert({ created_date , licence.size() + 1 });
    }

    void clear()
    {
        std::lock_guard<std::mutex> lockGuard(licence_mutex);
        licence.clear();
        licence.shrink_to_fit();
    }

    int getTotalTry()
    {
        std::lock_guard<std::mutex> lockGuard(licence_mutex);
        return licence.size();
    }
};


#endif //FPANEL_CORE_LICENCE_H
