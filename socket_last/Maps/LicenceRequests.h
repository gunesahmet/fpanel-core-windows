#pragma once
#ifndef FPANEL_CORE_LICENCE_REQUESTS_H
#define FPANEL_CORE_LICENCE_REQUESTS_H

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <tsl/ordered_map.h>

class LicenceRequests {
    tsl::ordered_map<std::string, int> requests;
    std::mutex licence_request_mutex;
public:
    LicenceRequests() {
        requests = tsl::ordered_map<std::string, int>();
    }

    void add(std::string request)
    {
        std::lock_guard<std::mutex> lockGuard(licence_request_mutex);
        int req_id = 1;
        if(requests.rbegin() != requests.rend()) {
            if(requests.size() > 0) {
                int last_req_id = requests.rbegin()->second;
                req_id = last_req_id + 1;
            }
        }
        requests.insert({ request, req_id });
    }

    std::string getLast()
    {
        std::lock_guard<std::mutex> lockGuard(licence_request_mutex);
        if(requests.rbegin() != requests.rend()) {
            if(requests.size() > 0) {
                std::string request = requests.rbegin()->first;
                int req_id = requests.rbegin()->second;
                return request;
            }
        }
        return {};
    }

    void clear()
    {
        std::lock_guard<std::mutex> lockGuard(licence_request_mutex);
        requests.clear();
        requests.shrink_to_fit();
    }
};


#endif //FPANEL_CORE_LICENCE_REQUESTS_H
