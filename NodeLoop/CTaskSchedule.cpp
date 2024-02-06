#include "CTaskSchedule.h"


namespace LinuxLogFactory {
	

	void to_json(nlohmann::json& j, const STaskSchedule& schedule) {
        j = nlohmann::json{
            {
                "order_id", schedule.order_id
            },
            {
                "task", schedule.task
            },
            {
                "deadline", std::chrono::duration_cast<std::chrono::milliseconds>(schedule.deadline.time_since_epoch()).count()
            },
            {
                "execution_time", schedule.execution_time
            }
        };
	}

	void from_json(const nlohmann::json& j, STaskSchedule& schedule) {
        j.at("order_id").get_to(schedule.order_id);
        j.at("task").get_to(schedule.task);

        long long deadline = j.at("deadline").get<long long>();
        schedule.deadline = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(deadline));

        j.at("execution_time").get_to(schedule.execution_time);
	}




    void to_json(nlohmann::json& j, const STaskSchedules& schedules) {
        j = nlohmann::json{
            {
                "schedules", schedules.schedules
            }
        };
    }

    void from_json(const nlohmann::json& j, STaskSchedules& schedules) {
        for (auto& elem : j["schedules"])
            schedules.schedules.push_back(elem.template get<STaskSchedule>());
        //j["schedules"]
        //j.at("schedules").get_to(schedules.schedules);
    }



}