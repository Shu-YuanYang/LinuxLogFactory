// NodeLoop.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <unistd.h> 
//#include "nlohmann/json.hpp"
//#include <string>
#include "COrder.h"
#include "CTaskSchedule.h"
#include "FileLock.h"
#include <algorithm>





int main(int argc, char* argv[])
{
    //N1schedules.json
    /*
    //std::cout << "Hello World Main Thread!\n";
    std::string arg_str(argv[1]);
    std::replace(arg_str.begin(), arg_str.end(), '\'', '"');
    nlohmann::json Doc{ nlohmann::json::parse(arg_str)};
    
    LinuxLogFactory::STaskSchedules sample_schedules{
        std::list<LinuxLogFactory::STaskSchedule> {
            LinuxLogFactory::STaskSchedule {
                "20240205162600001",
                "task1",
                std::chrono::system_clock::now(),
                2503
            },
            LinuxLogFactory::STaskSchedule {
                "20240205162600001",
                "task2",
                std::chrono::system_clock::now(),
                1311
            },

        }
    };

    nlohmann::json j = sample_schedules;
    std::cout << j << std::endl;
    LinuxLogFactory::STaskSchedules confirm_schedules(j.template get<LinuxLogFactory::STaskSchedules>());

    long long time_elapsed1 = std::chrono::duration_cast<std::chrono::milliseconds>(confirm_schedules.schedules.front().deadline.time_since_epoch()).count();
    long long time_elapsed2 = std::chrono::duration_cast<std::chrono::milliseconds>(confirm_schedules.schedules.back().deadline.time_since_epoch()).count();
    std::cout << time_elapsed1 << std::endl;
    std::cout << time_elapsed2 << std::endl;

    std::cout << Doc << std::endl;
    //std::thread scheduler_th(scheduler_run, 5);
    //scheduler_th.join();
    */


    std::string N1schedules = std::read_file_with_lock("N1schedules.json");
    //std::cout << N1schedules << std::endl;
    nlohmann::json N1SchedulesDoc = nlohmann::json::parse(N1schedules);
    LinuxLogFactory::STaskSchedules schedules(N1SchedulesDoc.template get<LinuxLogFactory::STaskSchedules>());
    

    if (schedules.schedules.begin() == schedules.schedules.end()) {
        long long completion_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        std::cout << "{\"completed_task\": null, \"completed_time\": " << completion_time << ", \"next\": false}" << std::endl;
    }

    LinuxLogFactory::STaskSchedule& first_task = schedules.schedules.front();
    // execute first task
    usleep(first_task.execution_time * 1000);
    // execute complete

    std::cout << "Hello?" << std::endl;
    N1schedules = std::read_file_with_lock("N1schedules.json");
    N1SchedulesDoc = nlohmann::json::parse(N1schedules);
    LinuxLogFactory::STaskSchedules new_schedules = N1SchedulesDoc.template get<LinuxLogFactory::STaskSchedules>();

    std::vector<LinuxLogFactory::STaskSchedule>::iterator iter = new_schedules.schedules.begin();
    while(iter != new_schedules.schedules.end() && iter->order_id != first_task.order_id) ++iter;
    new_schedules.schedules.erase(iter);

    nlohmann::json j = new_schedules;
    std::cout << j << std::endl;

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
