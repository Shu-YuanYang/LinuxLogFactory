#include "COrder.h"


namespace LinuxLogFactory {
    
    void to_json(nlohmann::json& j, const SOrder& order)
    {
        
        j = nlohmann::json {
            {
                "tasks", order.tasks
            },
            {
                "deadline", std::chrono::duration_cast<std::chrono::milliseconds>(order.deadline.time_since_epoch()).count()
            },
            { 
                "order_id", order.order_id
            },
            {
                "reorder_limit", order.reorder_limit
            }
        };
    }


    void from_json(const nlohmann::json& j, SOrder& order) {
        
        j.at("tasks").get_to(order.tasks);

        long long deadline = j.at("deadline").get<long long>();
        order.deadline = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(deadline));

        j.at("order_id").get_to(order.order_id);
        j.at("reorder_limit").get_to(order.reorder_limit);
    }
    

}