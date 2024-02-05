#pragma once

#include <vector>
#include <string>
#include <chrono>
#include "nlohmann/json.hpp"

namespace LinuxLogFactory {
	
	struct SOrder {
	public:
		std::vector<std::string> tasks;
		std::chrono::system_clock::time_point deadline;
		std::string order_id;
		int reorder_limit;
	};


	class COrder
	{

	};

	void to_json(nlohmann::json& j, const SOrder& order);

	void from_json(const nlohmann::json& j, SOrder& order);

}



