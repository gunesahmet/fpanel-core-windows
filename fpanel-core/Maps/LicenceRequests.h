#pragma once
#ifndef FPANEL_CORE_LICENCE_REQUESTS_H
#define FPANEL_CORE_LICENCE_REQUESTS_H

#include <iostream>
#include <thread>
#include <vector>
#include <shared_mutex>
#include <tsl/ordered_map.h>

class LicenceRequests {
public:
	struct licence_result {
		std::string created_date;
		std::string expire_date;
		std::string licence_key;
		std::string licence_hash;
		std::string current_date;
		std::string	status;
		std::string domains;
		std::string clients;
	};

	licence_result default_licence_result;
	tsl::ordered_map<int, licence_result> requests;
	std::shared_mutex licence_request_mutex;

	LicenceRequests() {
        requests = tsl::ordered_map<int, licence_result>();
		default_licence_result = {
			"none",
			"",
			"",
			"",
			"none",
			"0",
			"1",
			"1"
		};
    }

	licence_result MakeResult(std::string created_date,
		std::string expire_date,
		std::string licence_key,
		std::string licence_hash,
		std::string current_date,
		std::string	status,
		std::string domains,
		std::string clients)
	{
		licence_result result = {
			created_date,
			expire_date,
			licence_key,
			licence_hash,
			current_date,
			status,
			domains,
			clients
		};
		return result;
	}

    void add(licence_result request)
    {
		std::unique_lock<std::shared_mutex> slk(licence_request_mutex);
        int req_id = 1;
        if(requests.rbegin() != requests.rend()) {
            if(requests.size() > 0) {
                int last_req_id = requests.rbegin()->first;
                req_id = last_req_id + 1;
            }
        }
        requests.insert({ req_id, request });
		if (requests.size() > 10) {
			req_id = requests.begin()->first;
			requests.erase(req_id);
			requests.shrink_to_fit();
		}
    }

	licence_result getLast()
    {
		std::shared_lock<std::shared_mutex> slk(licence_request_mutex);
        if(requests.rbegin() != requests.rend()) {
            if(requests.size() > 0) {
				licence_result request = requests.rbegin()->second;
                return request;
            }
        }
        return default_licence_result;
    }

    void clear()
    {
		std::unique_lock<std::shared_mutex> slk(licence_request_mutex);
        requests.clear();
        requests.shrink_to_fit();
    }
};


#endif //FPANEL_CORE_LICENCE_REQUESTS_H
