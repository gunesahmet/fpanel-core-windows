#pragma once
#ifndef FPANEL_CORE_CPU_MAP_H
#define FPANEL_CORE_CPU_MAP_H
#include <iostream>
#include <mutex>
#include <tuple>
#include "../tsl/ordered_map.h"

class CpuMap {
	tsl::ordered_map<int, int> cpu_usages;
	std::mutex cpu_usages_mutex;
public:
	CpuMap() {
		cpu_usages = tsl::ordered_map<int, int>();
	}

	void addCpuUsage(int job)
	{
		std::lock_guard<std::mutex> lockGuard(cpu_usages_mutex);
		int job_id = 1;
		if (cpu_usages.rbegin() != cpu_usages.rend()) {
			if (cpu_usages.size() > 0) {
				int last_job_id = cpu_usages.rbegin()->first;
				job_id = last_job_id + 1;
			}
		}
		if (cpu_usages.size() > 20) {
			if (cpu_usages.rbegin() != cpu_usages.rend()) {
				if (cpu_usages.size() > 0) {
					int usage_id = cpu_usages.begin()->first;
					int usage = cpu_usages.begin()->second;
					cpu_usages.erase(usage_id);
				}
			}
		}
		cpu_usages.insert({ job_id , job });
	}

	int getCpuUsage()
	{
		std::lock_guard<std::mutex> lockGuard(cpu_usages_mutex);
		if (cpu_usages.rbegin() != cpu_usages.rend()) {
			if (cpu_usages.size() > 0) {
				return cpu_usages.end()->second;
			}
		}
		return 0;
	}
};
#endif //FPANEL_CORE_CPU_MAP_H
