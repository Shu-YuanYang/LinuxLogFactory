#pragma once

#include <string>
#include <chrono>
#include "nlohmann/json.hpp"
#include <vector>


namespace LinuxLogFactory {
	/*
	order_id,
	task,
	start_time,
	deadline,
	execution_time

	*/

	struct STaskSchedule {
	public:
		std::string order_id;
		std::string task;
		//std::chrono::system_clock::time_point start_time;
		std::chrono::system_clock::time_point deadline;
		long long execution_time;
	};

	struct STaskSchedules {
	public:
		std::vector<STaskSchedule> schedules;
	};



	class CTaskSchedule
	{
	};

	void to_json(nlohmann::json& j, const STaskSchedule& schedule);

	void from_json(const nlohmann::json& j, STaskSchedule& schedule);

	void to_json(nlohmann::json& j, const STaskSchedules& schedules);

	void from_json(const nlohmann::json& j, STaskSchedules& schedules);
}



